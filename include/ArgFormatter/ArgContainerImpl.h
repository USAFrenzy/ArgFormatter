#pragma once

#ifndef COMPILED_LIBRARY
	#include "dependencies/UTF-Utils/utf-utils.h"
#else
	#include <UTF-Utils/utf-utils.h>
#endif    // !BUILD_COMPILED_LIBRARY

namespace formatter::msg_details {
#include "ArgContainer.h"

	constexpr std::array<formatter::internal_helper::af_typedefs::VType, MAX_ARG_COUNT>& ArgContainer::ArgStorage() {
		return argContainer;
	}

	constexpr const std::array<SpecType, MAX_ARG_COUNT>& ArgContainer::SpecTypesCaptured() const {
		return specContainer;
	}

	template<typename Iter, typename T> constexpr auto ArgContainer::StoreCustomArg(Iter&& iter, T&& value) -> decltype(iter) {
		using namespace formatter::internal_helper;
		if constexpr( std::is_pointer_v<T> ) {
				argContainer[ counter ] = CustomValue(IteratorAccessHelper(std::forward<Iter>(iter)).Container(), std::forward<std::remove_pointer_t<T>>(*value));
		} else {
				argContainer[ counter ] = CustomValue(IteratorAccessHelper(std::forward<Iter>(iter)).Container(), std::forward<T>(value));
			}
		return std::move(std::add_rvalue_reference_t<decltype(iter)>(iter));
	}

	template<typename T> constexpr void ArgContainer::StoreNativeArg(T&& value) {
		using namespace formatter::internal_helper;
		if constexpr( af_concepts::is_supported_ptr_type_v<T> || std::is_same_v<af_typedefs::type<T>, std::string> ) {
				if constexpr( std::is_same_v<af_typedefs::type<T>, std::tm*> ) {
						argContainer[ counter ] = std::forward<T>(*value);
				} else {
						argContainer[ counter ] = std::forward<T>(value);
					}
		} else {
				argContainer[ counter ] = std::forward<T>(value);
			}
	}

	template<typename Iter, typename... Args> constexpr auto ArgContainer::StoreArgs(Iter&& iter, Args&&... args) -> decltype(iter) {
		(
		[ this ](auto&& arg, auto&& iter) {
			using namespace formatter::internal_helper;
			using ArgType = decltype(arg);
			using namespace utf_utils;
			using enum bom_type;

			// handle string types as a special case
			if constexpr( utf_constraints::is_string_v<ArgType> || utf_constraints::is_string_view_v<ArgType> ) {
					switch( DetectBom(std::forward<ArgType>(arg)) ) {
							case utf8_bom: [[fallthrough]];
							case utf8_no_bom:
								{
									if constexpr( std::is_same_v<typename formatter::internal_helper::af_typedefs::type<ArgType>::value_type, unsigned char> &&
									              std::is_signed_v<char> ) {
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
											using remove_ref        = formatter::internal_helper::af_typedefs::type<decltype(arg)>;
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
					specContainer[ counter ] = GetArgType(std::forward<ArgType>(arg));
					if constexpr( af_concepts::is_supported_v<af_typedefs::type<ArgType>> ) {
							StoreNativeArg(std::forward<ArgType>(arg));
					} else if constexpr( std::is_constructible_v<std::remove_cvref_t<std::remove_extent_t<ArgType>>> ) {
							// test for cases of 'char[]&] and 'char[]' and treat as a c-string (storing it natively) -> solving
							// https://github.com/USAFrenzy/ArgFormatter/issues/2
							if constexpr( auto _ { std::remove_cvref_t<std::remove_extent_t<ArgType>> {} }; std::is_same_v<decltype(_), char*> ) {
									StoreNativeArg(std::forward<const char*>(static_cast<const char*>(arg)));
							} else {    // isn't a c-string relative or native type so store it as a custom type
									iter = std::move(StoreCustomArg(std::move(iter), std::forward<ArgType>(arg)));
								}
					} else {    // non-constructible -> assume the type requires a reference and that the default/copy constructors are deleted
							iter = std::move(StoreCustomArg(std::move(iter), std::forward<ArgType>(arg)));
						}
					++counter;
					AF_ASSERT(counter < MAX_ARG_COUNT, "Too Many Arguments Supplied To Formatting Function");
				}
		}(std::forward<Args>(args), std::move(iter)),
		...);
		return std::move(iter);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//! NOTE: This is most likely where issues #1-#3 honestly stem from (%95 sure of this): Similar to how a copy is stored of the spec
	//! type container to restore from, we should store a copy of the arg container to restore from (quick & dirty but hacky fix)
	//! GOING FORWARD -> the above is a hacky solution, the better alternative would be to have the custom type have it's own
	//! argument/spec type containers so as not to interfere with the top level containers
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template<typename Iter, typename... Args> constexpr auto ArgContainer::CaptureArgs(Iter&& iter, Args&&... args) -> decltype(iter) {
		counter = 0;
		std::memset(specContainer.data(), 0, MAX_ARG_COUNT);
		return std::move(StoreArgs(std::move(iter), std::forward<Args>(args)...));
	}

	constexpr const std::string_view ArgContainer::string_state(size_t index) const {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<std::string>(argContainer[ index ]), "Error Retrieving std::string: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<1>(&argContainer[ index ]);
	}

	constexpr const std::string_view ArgContainer::c_string_state(size_t index) const {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<const char*>(argContainer[ index ]), "Error Retrieving const char*: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<2>(&argContainer[ index ]);
	}

	constexpr const std::string_view ArgContainer::string_view_state(size_t index) const {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		// clang-format off
		AF_ASSERT(std::holds_alternative<std::string_view>(argContainer[index]),
			"Error Retrieving std::string_view: Variant At Index Provided Doesn't Contain This Type.");
		// clang-format on
		return *std::get_if<3>(&argContainer[ index ]);
	}

	constexpr const int& ArgContainer::int_state(size_t index) const {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<int>(argContainer[ index ]), "Error Retrieving int: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<4>(&argContainer[ index ]);
	}

	constexpr const unsigned int& ArgContainer::uint_state(size_t index) const {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<unsigned int>(argContainer[ index ]), "Error Retrieving unsigned int: Variant At Index Provided Doesn't Contain This "
		                                                                       "Type.");
		return *std::get_if<5>(&argContainer[ index ]);
	}

	constexpr const long long& ArgContainer::long_long_state(size_t index) const {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<long long>(argContainer[ index ]), "Error Retrieving long long: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<6>(&argContainer[ index ]);
	}

	unsigned constexpr const long long& ArgContainer::u_long_long_state(size_t index) const {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		// clang-format off
		AF_ASSERT(std::holds_alternative<unsigned long long>(argContainer[index]),
			"Error Retrieving unsigned long long: Variant At Index Provided Doesn't Contain This Type.");
		// clang-format on
		return *std::get_if<7>(&argContainer[ index ]);
	}

	constexpr const bool& ArgContainer::bool_state(size_t index) const {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<bool>(argContainer[ index ]), "Error Retrieving bool: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<8>(&argContainer[ index ]);
	}

	constexpr const char& ArgContainer::char_state(size_t index) const {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<char>(argContainer[ index ]), "Error Retrieving char: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<9>(&argContainer[ index ]);
	}

	constexpr const float& ArgContainer::float_state(size_t index) const {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<float>(argContainer[ index ]), "Error Retrieving float: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<10>(&argContainer[ index ]);
	}

	constexpr const double& ArgContainer::double_state(size_t index) const {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<double>(argContainer[ index ]), "Error Retrieving double: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<11>(&argContainer[ index ]);
	}

	constexpr const long double& ArgContainer::long_double_state(size_t index) const {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<long double>(argContainer[ index ]), "Error Retrieving long double: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<12>(&argContainer[ index ]);
	}
	constexpr const void* ArgContainer::const_void_ptr_state(size_t index) const {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<const void*>(argContainer[ index ]), "Error Retrieving const void*: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<13>(&argContainer[ index ]);
	}

	constexpr void* ArgContainer::void_ptr_state(size_t index) const {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<void*>(argContainer[ index ]), "Error Retrieving Void*: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<14>(&argContainer[ index ]);
	}

	constexpr const std::tm& formatter::msg_details::ArgContainer::c_time_state(size_t index) const {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		AF_ASSERT(std::holds_alternative<std::tm>(argContainer[ index ]), "Error Retrieving std::tm: Variant At Index Provided Doesn't Contain This Type.");
		return *std::get_if<15>(&argContainer[ index ]);
	}

	constexpr const formatter::internal_helper::CustomValue& formatter::msg_details::ArgContainer::custom_state(size_t index) const {
		AF_ASSERT(index <= MAX_ARG_INDEX, "Error Retrieving Stored Value - Index Is Out Of Bounds");
		// clang-format off
		AF_ASSERT(std::holds_alternative<formatter::internal_helper::CustomValue>(argContainer[index]),
			"Error Retrieving custom value type: Variant At Index Provided Doesn't Contain This Type.");
		// clang-format on
		return *std::get_if<16>(&argContainer[ index ]);
	}

}    // namespace formatter::msg_details