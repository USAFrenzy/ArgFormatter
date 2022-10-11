#pragma once

#include "dependancies/UTF-Utils/utf-utils.h"

namespace formatter::msg_details {
#include "ArgContainer.h"

	namespace af_types = formatter::internal_helper::af_typedefs;

	constexpr std::array<af_types::VType, MAX_ARG_COUNT>& ArgContainer::ArgStorage() {
		return argContainer;
	}

	constexpr std::array<SpecType, MAX_ARG_COUNT>& ArgContainer::SpecTypesCaptured() {
		return specContainer;
	}

	template<typename Iter, typename T> constexpr auto ArgContainer::StoreCustomArg(Iter&& iter, T&& value) -> decltype(iter) {
		argContainer[ counter ] =
		CustomValue(IteratorAccessHelper(std::remove_reference_t<decltype(iter)>(iter)).Container(), std::forward<FwdConstRef<T>>(FwdConstRef<T>(value)));
		return std::move(std::add_rvalue_reference_t<decltype(iter)>(iter));
	}

	template<typename T> constexpr void ArgContainer::StoreNativeArg(T&& value) {
		if constexpr( is_supported_ptr_type_v<T> || std::is_same_v<af_types::type<T>, std::string> ) {
				if constexpr( std::is_same_v<af_types::type<T>, std::tm*> ) {
						argContainer[ counter ] = std::forward<af_types::FwdRef<T>>(af_types::FwdRef<T>(*value));
				} else {
						argContainer[ counter ] = std::forward<af_types::type<T>>(af_types::type<T>(value));
					}
		} else {
				argContainer[ counter ] = std::forward<af_types::FwdRef<T>>(af_types::FwdRef<T>(value));
			}
	}

	template<typename Iter, typename... Args> constexpr auto ArgContainer::StoreArgs(Iter&& iter, Args&&... args) -> decltype(iter) {
		(
		[ this ](auto&& arg, auto&& iter) {
			using ArgType = decltype(arg);
			using namespace utf_utils;
			using enum bom_type;

			// handle string types as a special case
			if constexpr( utf_constraints::is_string_v<ArgType> || utf_constraints::is_string_view_v<ArgType> ) {
					switch( DetectBom(std::forward<ArgType>(arg)) ) {
							case utf8_bom: [[fallthrough]];
							case utf8_no_bom:
								{
									if constexpr( std::is_same_v<typename af_types::type<ArgType>::value_type, unsigned char> && std::is_signed_v<char> ) {
											std::string tmp;
											tmp.reserve(arg.size());
											for( auto& ch: arg ) {
													tmp += static_cast<char>(ch);
												}
											if constexpr( utf_constraints::is_string_v<ArgType> ) {
													argContainer[ counter ]  = std::move(tmp);
													specContainer[ counter ] = SpecType::StringType;
											} else {
													argContainer[ counter ]  = std::string_view(std::move(tmp));
													specContainer[ counter ] = SpecType::StringViewType;
												}
									} else {
											using remove_ref        = af_types::type<decltype(arg)>;
											argContainer[ counter ] = remove_ref(arg);
											if constexpr( utf_constraints::is_string_v<ArgType> ) {
													specContainer[ counter ] = SpecType::StringType;
											} else {
													specContainer[ counter ] = SpecType::StringViewType;
												}
										}
									break;
								}
							case utf16LE_bom: [[fallthrough]];
							case utf16BE_bom: [[fallthrough]];
							case utf16_no_bom:
								{
									std::string tmp;
									tmp.reserve(ReserveLengthForU8(std::forward<ArgType>(arg)));
									U16ToU8(std::forward<ArgType>(arg), tmp);
									if constexpr( utf_constraints::is_string_v<ArgType> ) {
											argContainer[ counter ]  = std::move(tmp);
											specContainer[ counter ] = SpecType::StringType;
									} else {
											argContainer[ counter ]  = std::string_view(std::move(tmp));
											specContainer[ counter ] = SpecType::StringViewType;
										}
									break;
								}
							case utf32LE_bom: [[fallthrough]];
							case utf32BE_bom: [[fallthrough]];
							case utf32_no_bom:
								{
									std::string tmp;
									tmp.reserve(ReserveLengthForU8(std::forward<ArgType>(arg)));
									U32ToU8(std::forward<ArgType>(arg), tmp);
									if constexpr( utf_constraints::is_string_v<ArgType> ) {
											argContainer[ counter ]  = std::move(tmp);
											specContainer[ counter ] = SpecType::StringType;
									} else {
											argContainer[ counter ]  = std::string_view(std::move(tmp));
											specContainer[ counter ] = SpecType::StringViewType;
										}
									break;
								}
							default: AF_ASSERT(false, "Unknown Encoding Detected"); break;
						}
					++counter;
					AF_ASSERT(counter < MAX_ARG_COUNT, "Too Many Arguments Supplied To Formatting Function");
			} else {
					specContainer[ counter ] = GetArgType(std::forward<af_types::FwdRef<ArgType>>(af_types::FwdRef<ArgType>(arg)));
					if constexpr( is_supported_v<af_types::type<ArgType>> ) {
							StoreNativeArg(std::forward<af_types::FwdRef<ArgType>>(af_types::FwdRef<ArgType>(arg)));
					} else {
							iter = std::move(StoreCustomArg(std::move(iter), std::forward<af_types::FwdRef<ArgType>>(af_types::FwdRef<ArgType>(arg))));
						}
					++counter;
					AF_ASSERT(counter < MAX_ARG_COUNT, "Too Many Arguments Supplied To Formatting Function");
				}
		}(std::forward<af_types::FwdRef<Args>>(af_types::FwdRef<Args>(args)), std::move(iter)),
		...);
		return std::move(iter);
	}
	template<typename Iter, typename... Args> constexpr auto ArgContainer::CaptureArgs(Iter&& iter, Args&&... args) -> decltype(iter) {
		counter = 0;
		std::memset(specContainer.data(), 0, MAX_ARG_COUNT);
		return std::move(StoreArgs(std::move(iter), std::forward<Args>(args)...));
	}

	constexpr std::string_view ArgContainer::string_state(size_t index) {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<std::string>(argContainer[ index ]), "Error Retrieving std::string: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<1>(&argContainer[ index ]);
	}

	constexpr std::string_view ArgContainer::c_string_state(size_t index) {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<const char*>(argContainer[ index ]), "Error Retrieving const char*: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<2>(&argContainer[ index ]);
	}

	constexpr std::string_view ArgContainer::string_view_state(size_t index) {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		// clang-format off
		AF_ASSERT(std::holds_alternative<std::string_view>(argContainer[index]),
			"Error Retrieving std::string_view: Variant At Index Provided Doesn't Contain This Type.");
		// clang-format on
		return *std::get_if<3>(&argContainer[ index ]);
	}

	constexpr int& ArgContainer::int_state(size_t index) {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<int>(argContainer[ index ]), "Error Retrieving int: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<4>(&argContainer[ index ]);
	}

	constexpr unsigned int& ArgContainer::uint_state(size_t index) {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<unsigned int>(argContainer[ index ]), "Error Retrieving unsigned int: Variant At Index Provided Doesn't Contain This "
		                                                                       "Type.");
		return *std::get_if<5>(&argContainer[ index ]);
	}

	constexpr long long& ArgContainer::long_long_state(size_t index) {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<long long>(argContainer[ index ]), "Error Retrieving long long: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<6>(&argContainer[ index ]);
	}

	unsigned constexpr long long& ArgContainer::u_long_long_state(size_t index) {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		// clang-format off
		AF_ASSERT(std::holds_alternative<unsigned long long>(argContainer[index]),
			"Error Retrieving unsigned long long: Variant At Index Provided Doesn't Contain This Type.");
		// clang-format on
		return *std::get_if<7>(&argContainer[ index ]);
	}

	constexpr bool& ArgContainer::bool_state(size_t index) {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<bool>(argContainer[ index ]), "Error Retrieving bool: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<8>(&argContainer[ index ]);
	}

	constexpr char& ArgContainer::char_state(size_t index) {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<char>(argContainer[ index ]), "Error Retrieving char: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<9>(&argContainer[ index ]);
	}

	constexpr float& ArgContainer::float_state(size_t index) {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<float>(argContainer[ index ]), "Error Retrieving float: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<10>(&argContainer[ index ]);
	}

	constexpr double& ArgContainer::double_state(size_t index) {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<double>(argContainer[ index ]), "Error Retrieving double: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<11>(&argContainer[ index ]);
	}

	constexpr long double& ArgContainer::long_double_state(size_t index) {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<long double>(argContainer[ index ]), "Error Retrieving long double: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<12>(&argContainer[ index ]);
	}
	constexpr const void* ArgContainer::const_void_ptr_state(size_t index) {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<const void*>(argContainer[ index ]), "Error Retrieving const void*: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<13>(&argContainer[ index ]);
	}

	constexpr void* ArgContainer::void_ptr_state(size_t index) {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<void*>(argContainer[ index ]), "Error Retrieving Void*: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<14>(&argContainer[ index ]);
	}

	constexpr std::tm& formatter::msg_details::ArgContainer::c_time_state(size_t index) {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<std::tm>(argContainer[ index ]), "Error Retrieving std::tm: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<15>(&argContainer[ index ]);
	}

	constexpr formatter::internal_helper::CustomValue& formatter::msg_details::ArgContainer::custom_state(size_t index) {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		// clang-format off
		AF_ASSERT(std::holds_alternative<formatter::internal_helper::CustomValue>(argContainer[index]),
			"Error Retrieving custom value type: Variant At Index Provided Doesn't Contain This Type.");
		// clang-format on
		return *std::get_if<16>(&argContainer[ index ]);
	}

}    // namespace formatter::msg_details