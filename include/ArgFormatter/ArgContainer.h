#pragma once

#include <string_view>
#include <array>
#include <iterator>
#include <variant>
#include <vector>

namespace formatter {

#ifdef _DEBUG
	#ifndef AF_ASSERT
		#define AF_ASSERT(condition, message)                                                                                                                       \
			if( !(condition) ) {                                                                                                                                    \
					fprintf(stderr, "Assertion Failed: (%s) |File: %s | Line: %i\nMessage:%s\n", #condition, __FILE__, __LINE__, message);                          \
					abort();                                                                                                                                        \
			}
	#endif
#else
	#ifndef AF_ASSERT
		#define AF_ASSERT(condition, message) void(0)
	#endif
#endif

	// convenience typedefs
	namespace internal_helper::af_typedefs {
		template<typename T> using type        = std::remove_cvref_t<T>;
		template<typename T> using FwdRef      = std::add_lvalue_reference_t<type<T>>;
		template<typename T> using FwdConstRef = std::add_lvalue_reference_t<std::add_const_t<type<T>>>;
		template<typename T> using FwdMove     = std::add_rvalue_reference_t<type<T>>;
		template<typename T> using Iterator    = std::back_insert_iterator<type<T>>;
		template<typename T> using FwdMoveIter = std::add_rvalue_reference_t<Iterator<T>>;
		template<typename T> using FwdRefIter  = std::add_lvalue_reference_t<Iterator<T>>;
	}    // namespace internal_helper::af_typedefs

	template<typename Value> struct CustomFormatter
	{
		constexpr CustomFormatter()                                  = default;
		constexpr CustomFormatter(const CustomFormatter&)            = default;
		constexpr CustomFormatter& operator=(const CustomFormatter&) = default;
		constexpr CustomFormatter(CustomFormatter&&)                 = default;
		constexpr CustomFormatter& operator=(CustomFormatter&&)      = default;
		constexpr ~CustomFormatter()                                 = default;

		constexpr void Parse(std::string_view) { }
		template<typename ValueType, typename ContainerCtx> constexpr auto Format(ValueType&&, ContainerCtx&&) { }
	};

	namespace internal_helper {

		template<typename T> struct IteratorAccessHelper: public std::back_insert_iterator<T>
		{
			using std::back_insert_iterator<T>::container_type;
			constexpr explicit IteratorAccessHelper(std::back_insert_iterator<T>&(Iter))
				: std::back_insert_iterator<T>(std::forward<af_typedefs::FwdRefIter<T>>(Iter)) { }
			constexpr explicit IteratorAccessHelper(std::back_insert_iterator<T> && (Iter))
				: std::back_insert_iterator<T>(std::forward<af_typedefs::FwdMoveIter<T>>(Iter)) { }
			constexpr IteratorAccessHelper()                                       = delete;
			constexpr IteratorAccessHelper(const IteratorAccessHelper&)            = delete;
			constexpr IteratorAccessHelper& operator=(const IteratorAccessHelper&) = delete;
			constexpr IteratorAccessHelper(IteratorAccessHelper&&)                 = default;
			constexpr IteratorAccessHelper& operator=(IteratorAccessHelper&&)      = default;
			constexpr ~IteratorAccessHelper()                                      = default;

			constexpr auto& Container() {
				return *(this->container);
			}
		};

		struct CustomValue
		{
			using FormatCallBackFunc = void (*)(std::string_view parseView, const void* data, const void* contPtr);
			template<typename Container, typename T>
			explicit constexpr CustomValue(Container&& cont, T&& value)
				: data(std::addressof(value)), container(std::addressof(cont)),
				  CustomFormatCallBack([](std::string_view parseView, const void* ptr, const void* contPtr) {
					  using QualifiedType = std::add_const_t<internal_helper::af_typedefs::type<T>>;
					  using QualifiedRef  = std::add_lvalue_reference_t<QualifiedType>;
					  using ContainerType = internal_helper::af_typedefs::type<Container>;
					  using ContainerRef  = std::add_lvalue_reference_t<ContainerType>;
					  /*  Above are just  some convenient typedefs from the paramater value types for use below in casting the pointers to  the correct types */
					  auto formatter { CustomFormatter<internal_helper::af_typedefs::type<T>> {} };
					  formatter.Parse(parseView);
					  formatter.Format(QualifiedRef(*static_cast<QualifiedType*>(ptr)), ContainerRef(*static_cast<const ContainerType*>(contPtr)));
				  }) { }
			constexpr CustomValue()                              = delete;
			constexpr CustomValue(const CustomValue&)            = delete;
			constexpr CustomValue& operator=(const CustomValue&) = delete;
			constexpr CustomValue(CustomValue&& o) noexcept: data(o.data), container(o.container), CustomFormatCallBack(std::move(o.CustomFormatCallBack)) { }
			constexpr CustomValue& operator=(CustomValue&& o) noexcept {
				data                 = o.data;
				container            = o.container;
				CustomFormatCallBack = std::move(o.CustomFormatCallBack);
				return *this;
			}
			~CustomValue() = default;

			constexpr void FormatCallBack(std::string_view parseView) {
				CustomFormatCallBack(parseView, data, container);
			}

			const void* data;
			const void* container;
			FormatCallBackFunc CustomFormatCallBack;
		};
	}    // namespace internal_helper
	namespace internal_helper::af_typedefs {
		// clang-format off
		using VType = std::variant<std::monostate, std::string, const char*, std::string_view, int, unsigned int, long long,
			unsigned long long, bool, char, float, double, long double, const void*, void*, std::tm, internal_helper::CustomValue>;
		// clang-format on
	}    // namespace internal_helper::af_typedefs

	namespace internal_helper::af_concepts {

		template<typename T, typename U> struct is_supported;
		template<typename T, typename... Ts> struct is_supported<T, std::variant<Ts...>>: std::bool_constant<(std::is_same_v<std::remove_reference_t<T>, Ts> || ...)>
		{
		};
		template<typename T> inline constexpr bool is_supported_v = is_supported<T, internal_helper::af_typedefs::VType>::value;

		template<typename T> struct is_supported_ptr_type;
		template<typename T>
		struct is_supported_ptr_type: std::bool_constant<std::is_same_v<T, std::string_view> || std::is_same_v<T, const char*> || std::is_same_v<T, void*> ||
		                                                 std::is_same_v<T, const void*> || std::is_same_v<T, std::tm*>>
		{
		};
		template<typename T> inline constexpr bool is_supported_ptr_type_v = is_supported_ptr_type<internal_helper::af_typedefs::type<T>>::value;

		template<typename T> struct has_formatter: std::bool_constant<std::is_default_constructible_v<CustomFormatter<T>>>
		{
		};
		template<typename T> inline constexpr bool has_formatter_v = has_formatter<T>::value;

		template<typename T> struct is_formattable;
		template<typename T>
		struct is_formattable
			: std::bool_constant<is_supported<std::remove_reference_t<T>, internal_helper::af_typedefs::VType>::value || has_formatter<std::remove_cvref_t<T>>::value>
		{
		};
		template<typename T> inline constexpr bool is_formattable_v = is_formattable<T>::value;
	}    // namespace internal_helper::af_concepts

}    // namespace formatter

namespace formatter::msg_details {

	enum class SpecType
	{
		MonoType         = 0,
		StringType       = 1,
		CharPointerType  = 2,
		StringViewType   = 3,
		IntType          = 4,
		U_IntType        = 5,
		LongLongType     = 6,
		U_LongLongType   = 7,
		BoolType         = 8,
		CharType         = 9,
		FloatType        = 10,
		DoubleType       = 11,
		LongDoubleType   = 12,
		ConstVoidPtrType = 13,
		VoidPtrType      = 14,
		CTimeType        = 15,
		CustomType       = 16,
	};

	constexpr size_t MAX_ARG_COUNT = 25;
	constexpr size_t MAX_ARG_INDEX = 24;

	class ArgContainer
	{
	  public:
		constexpr ArgContainer()                               = default;
		constexpr ArgContainer(const ArgContainer&)            = delete;
		constexpr ArgContainer& operator=(const ArgContainer&) = delete;
		constexpr ArgContainer(ArgContainer&&)                 = default;
		constexpr ArgContainer& operator=(ArgContainer&&)      = default;
		~ArgContainer()                                        = default;

		template<typename Iter, typename... Args> constexpr auto CaptureArgs(Iter&& iter, Args&&... args) -> decltype(iter);
		template<typename Iter, typename... Args> constexpr auto StoreArgs(Iter&& iter, Args&&... args) -> decltype(iter);
		template<typename T> constexpr void StoreNativeArg(T&& arg);
		template<typename Iter, typename T> constexpr auto StoreCustomArg(Iter&& iter, T&& arg) -> decltype(iter);

		constexpr std::array<internal_helper::af_typedefs::VType, MAX_ARG_COUNT>& ArgStorage();
		constexpr std::array<SpecType, MAX_ARG_COUNT>& SpecTypesCaptured();
		constexpr std::string_view string_state(size_t index);
		constexpr std::string_view c_string_state(size_t index);
		constexpr std::string_view string_view_state(size_t index);
		constexpr int& int_state(size_t index);
		constexpr unsigned int& uint_state(size_t index);
		constexpr long long& long_long_state(size_t index);
		constexpr unsigned long long& u_long_long_state(size_t index);
		constexpr bool& bool_state(size_t index);
		constexpr char& char_state(size_t index);
		constexpr float& float_state(size_t index);
		constexpr double& double_state(size_t index);
		constexpr long double& long_double_state(size_t index);
		constexpr const void* const_void_ptr_state(size_t index);
		constexpr void* void_ptr_state(size_t index);
		constexpr std::tm& c_time_state(size_t index);
		constexpr internal_helper::CustomValue& custom_state(size_t index);

	  private:
		std::array<internal_helper::af_typedefs::VType, MAX_ARG_COUNT> argContainer {};
		std::array<SpecType, MAX_ARG_COUNT> specContainer {};
		size_t counter {};
	};
	// putting the definition here since clang was warning on extra qualifiers
	template<typename T> static constexpr SpecType GetArgType(T&& val) {
		using enum SpecType;
		if constexpr( std::is_same_v<internal_helper::af_typedefs::type<T>, std::monostate> ) {
				return std::forward<SpecType>(MonoType);
		} else if constexpr( std::is_same_v<internal_helper::af_typedefs::type<T>, std::string> ) {
				return std::forward<SpecType>(StringType);
		} else if constexpr( std::is_same_v<internal_helper::af_typedefs::type<T>, const char*> ) {
				return std::forward<SpecType>(CharPointerType);
		} else if constexpr( std::is_same_v<internal_helper::af_typedefs::type<T>, std::string_view> ) {
				return std::forward<SpecType>(StringViewType);
		} else if constexpr( std::is_same_v<internal_helper::af_typedefs::type<T>, int> ) {
				return std::forward<SpecType>(IntType);
		} else if constexpr( std::is_same_v<internal_helper::af_typedefs::type<T>, unsigned int> ) {
				return std::forward<SpecType>(U_IntType);
		} else if constexpr( std::is_same_v<internal_helper::af_typedefs::type<T>, long long> ) {
				return std::forward<SpecType>(LongLongType);
		} else if constexpr( std::is_same_v<internal_helper::af_typedefs::type<T>, unsigned long long> ) {
				return std::forward<SpecType>(U_LongLongType);
		} else if constexpr( std::is_same_v<internal_helper::af_typedefs::type<T>, bool> ) {
				return std::forward<SpecType>(BoolType);
		} else if constexpr( std::is_same_v<internal_helper::af_typedefs::type<T>, char> ) {
				return std::forward<SpecType>(CharType);
		} else if constexpr( std::is_same_v<internal_helper::af_typedefs::type<T>, float> ) {
				return std::forward<SpecType>(FloatType);
		} else if constexpr( std::is_same_v<internal_helper::af_typedefs::type<T>, double> ) {
				return std::forward<SpecType>(DoubleType);
		} else if constexpr( std::is_same_v<internal_helper::af_typedefs::type<T>, long double> ) {
				return std::forward<SpecType>(LongDoubleType);
		} else if constexpr( std::is_same_v<internal_helper::af_typedefs::type<T>, const void*> ) {
				return std::forward<SpecType>(ConstVoidPtrType);
		} else if constexpr( std::is_same_v<internal_helper::af_typedefs::type<T>, void*> ) {
				return std::forward<SpecType>(VoidPtrType);
		} else if constexpr( std::is_same_v<internal_helper::af_typedefs::type<T>, std::tm> ) {
				return std::forward<SpecType>(CTimeType);
		} else if constexpr( std::is_reference_v<T> ) {
				// As odd as it is, this case deals with 'char[]& 'or 'char[]' so as to treat those cases as c-style strings. If the base type is constructible, then
				// we can test for the case of it being a c-string relative, if it's not constructible, then only references can be made to the value, so just return
				// CustomType and let the CustomFormatter handle it
				if constexpr( std::is_constructible_v<internal_helper::af_typedefs::type<T>> ) {
						if constexpr( auto _ { std::add_const_t<std::remove_cvref_t<std::remove_extent_t<T>>> {} }; std::is_same_v<decltype(_), const char*> ) {
								return std::forward<SpecType>(CharPointerType);
						} else {
								return std::forward<SpecType>(MonoType);
							}
				} else {
						// clang-format off
					static_assert(internal_helper::af_concepts::is_formattable_v<internal_helper::af_typedefs::type<T>>,
						"A Template Specialization Must Exist For A Custom Type Argument.\n\t For ArgFormatter, This Can Be Done By "
						"Specializing The CustomFormatter Template For Your Type And Implementing The Parse() And Format() Functions.");
						// clang-format on
						return std::forward<SpecType>(CustomType);
					}
		} else {
				// This is the case where if none of the above has been matched, then it's either an xvalue
				// or pvalue, so just return CustomType here and let the CustomFormatter handle it

				// clang-format off
				static_assert(internal_helper::af_concepts::is_formattable_v<internal_helper::af_typedefs::type<T>>,
				"A Template Specialization Must Exist For A Custom Type Argument.\n\t For ArgFormatter, This Can Be Done By "
				"Specializing The CustomFormatter Template For Your Type And Implementing The Parse() And Format() Functions.");
				// clang-format on
				return std::forward<SpecType>(CustomType);
			}
	}
}    // namespace formatter::msg_details
#include "ArgContainerImpl.h"
