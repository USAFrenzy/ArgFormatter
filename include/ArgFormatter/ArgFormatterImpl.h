#pragma once

#include "ArgFormatter.h"

static constexpr std::array<const char*, 12> short_months = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};
static constexpr std::array<const char*, 7> short_weekdays = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
};
static constexpr std::array<const char*, 7> long_weekdays = {
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday",
};
static constexpr std::array<const char*, 12> long_months = {
	"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December",
};

static constexpr bool IsDigit(const char& ch) {
	return ((ch >= '0') && (ch <= '9'));
}

static constexpr bool IsAlpha(const char& ch) {
	return ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'));
}

using u_char_string = std::basic_string<unsigned char>;

// Note: there's no distinction made here for the overlapping case of 'Ey' and 'Oy' yet
static constexpr utf_utils::u_wstring LocalizedFormat(const unsigned char& ch) {
	utf_utils::u_wstring tmp;
	switch( ch ) {
			case 'c': [[fallthrough]];
			case 'x': [[fallthrough]];
			case 'C': [[fallthrough]];
			case 'X': [[fallthrough]];
			case 'Y': tmp.append(1, 'E').append(1, ch); break;
			default: tmp.append(1, 'O').append(1, ch); break;
		}
	return std::move(tmp);
}

// clang-format off
// NOTE: The use of std::put_time() incurs some massive overhead -> there may be a faster way to do this with the locale's facets  instead but haven't looked
//              into this quite yet, so currently unsure if that's a valid approach. Other than that though, the recreation of the format string to use for std::put_time() 
//              also has some overhead; only a small fraction of the overhead of std::put_time() itself, but it's the next line of code that hogs cycles in this function. 
//              For reference, in 100,000,000 iterations, std::put_time() took ~60% of CPU cycles while the reconstruction of the format string took ~13% of CPU 
//              cycles. So this can definitely be optimized more I think. Still results (in a char based container) in timings that are ~2.5x faster than MSVC though 
//              -> though I'm not encoding/decoding which may be why they have some more overhead.
// clang-format on

inline void formatter::arg_formatter::ArgFormatter::LocalizeBool(const std::locale& loc) {
	auto& storage { argStorage.isCustomFormatter ? customStorage : argStorage };
	std::string_view sv { storage.bool_state(specValues.argPosition) ? std::use_facet<std::numpunct<char>>(loc).truename()
		                                                             : std::use_facet<std::numpunct<char>>(loc).falsename() };
	valueSize = +sv.size();
	std::copy(sv.data(), sv.data() + valueSize, buffer.begin());
}

inline void formatter::arg_formatter::ArgFormatter::FormatIntegralGrouping(const std::locale& loc, size_t end) {
	auto groupings { std::use_facet<std::numpunct<char>>(loc).grouping() };
	if( end <= *groupings.begin() ) return;
	auto separator { std::use_facet<std::numpunct<char>>(loc).thousands_sep() };
	auto groupBegin { groupings.begin() };
	int groupGap { *groupBegin };
	std::string_view sv { buffer.data(), buffer.data() + end };
	size_t groups { 0 };

	switch( groupings.size() ) {
			case 3:
				{
					// grouping is unique
					int secondGroup { *(++groupBegin) };
					int repeatingGroup { *(++groupBegin) };
					groups = (valueSize - groupGap - secondGroup) / static_cast<size_t>(repeatingGroup) +
					         (valueSize - groupGap - secondGroup) % static_cast<size_t>(repeatingGroup);
					auto spaceRequired { valueSize + groups + 1 };    // add one for intermediate grouping
					valueSize = spaceRequired;

					std::copy(sv.end() - groupGap, sv.end(), buffer.data() + spaceRequired - groupGap);
					sv.remove_suffix(groupGap);
					spaceRequired -= groupGap;
					buffer[ --spaceRequired ] = separator;

					std::copy(sv.end() - secondGroup, sv.end(), buffer.data() + spaceRequired - secondGroup);
					spaceRequired -= secondGroup;
					sv.remove_suffix(secondGroup);
					buffer[ --spaceRequired ] = separator;

					for( ; groups; --groups ) {
							if( sv.size() > repeatingGroup ) {
									std::copy(sv.end() - repeatingGroup, sv.end(), buffer.data() + spaceRequired - repeatingGroup);
									spaceRequired -= repeatingGroup;
							} else {
									std::copy(sv.begin(), sv.end() - 1, buffer.begin());
									return;
								}
							if( groups > 0 ) {
									buffer[ --spaceRequired ] = separator;
							}
							if( sv.size() > repeatingGroup ) {
									sv.remove_suffix(repeatingGroup);
							}
						}
					break;
				}
			case 2:
				{
					// grouping is one group and then uniform
					int repeatingGroup { *(++groupBegin) };
					groups = (valueSize - groupGap) / static_cast<size_t>(repeatingGroup) + (valueSize - groupGap) % static_cast<size_t>(repeatingGroup);
					auto spaceRequired { valueSize + groups };
					valueSize = spaceRequired;

					std::copy(sv.end() - groupGap, sv.end(), buffer.data() + spaceRequired - groupGap);
					sv.remove_suffix(groupGap);
					spaceRequired -= groupGap;
					buffer[ --spaceRequired ] = separator;

					for( ; groups; --groups ) {
							if( sv.size() > repeatingGroup ) {
									std::copy(sv.end() - repeatingGroup, sv.end(), buffer.data() + spaceRequired - repeatingGroup);
									spaceRequired -= repeatingGroup;
							} else {
									std::copy(sv.begin(), sv.end() - 1, buffer.begin());
									return;
								}
							if( groups > 0 ) {
									buffer[ --spaceRequired ] = separator;
							}
							if( sv.size() > repeatingGroup ) {
									sv.remove_suffix(repeatingGroup);
							}
						}
					break;
				}
			case 1:
				{
					// grouping is uniform
					groups = valueSize / static_cast<size_t>(groupGap) + valueSize % static_cast<size_t>(groupGap);
					size_t spaceRequired { valueSize + groups - 1 };
					valueSize = spaceRequired;
					for( ; groups; --groups ) {
							if( sv.size() > groupGap ) {
									std::copy(sv.end() - groupGap, sv.end(), buffer.data() + spaceRequired - groupGap);
									spaceRequired -= groupGap;    // taking into consideration both the grouping and separator per group
							} else {
									std::copy(sv.begin(), sv.end() - 1, buffer.begin());
									return;
								}
							if( groups > 0 ) {
									buffer[ --spaceRequired ] = separator;
							}
							if( sv.size() > groupGap ) {
									sv.remove_suffix(groupGap);
							}
						}
					break;
				}
			default: break;
		}
}

inline void formatter::arg_formatter::ArgFormatter::LocalizeArgument(const std::locale& loc, const int& precision, const SpecType& type) {
	using enum formatter::msg_details::SpecType;
	// NOTE: The following types should have been caught in the verification process:  monostate, string, c-string, string view, const void*, void *
	switch( type ) {
			case IntType: [[fallthrough]];
			case U_IntType: [[fallthrough]];
			case LongLongType: LocalizeIntegral(loc, precision, type); break;
			case FloatType: [[fallthrough]];
			case DoubleType: [[fallthrough]];
			case LongDoubleType: [[fallthrough]];
			case U_LongLongType: LocalizeFloatingPoint(loc, precision, type); break;
			case BoolType: LocalizeBool(loc); break;
		}
	// should re-work this a little so using the unsigned buffer isn't totally necessary if not needed, however, WriteBufferToContainer() & WriteToContainer()
	// use the unsigned buffer at the moment when localization is set , so the contents of the original buffer need to be copied over to the unsigned buffer
	auto pos { -1 };
	auto& locBuff { timeSpec.localizationBuff };
	locBuff.resize(valueSize);
	for( ;; ) {
			if( ++pos >= valueSize ) return;
			locBuff[ pos ] = static_cast<unsigned char>(buffer[ pos ]);
		}
}

inline void formatter::arg_formatter::ArgFormatter::LocalizeIntegral(const std::locale& loc, const int& precision, const SpecType& type) {
	FormatArgument(precision, type);
	FormatIntegralGrouping(loc, valueSize);
}

inline void formatter::arg_formatter::ArgFormatter::LocalizeFloatingPoint(const std::locale& loc, const int& precision, const SpecType& type) {
	FormatArgument(precision, type);
	size_t pos { 0 };
	bool hasMantissa { false };
	for( ;; ) {
			if( pos >= valueSize ) break;
			if( buffer[ pos ] == '.' ) {
					hasMantissa = true;
					FormatIntegralGrouping(loc, pos);
					buffer[ pos ] = std::use_facet<std::numpunct<char>>(loc).decimal_point();
					++pos;
					break;
			}
			++pos;
		}
	if( !hasMantissa ) {
			FormatIntegralGrouping(loc, valueSize);
	}
}

inline void formatter::arg_formatter::ArgFormatter::LocalizeCTime(const std::locale& loc, const std::tm& timeStruct, const int& precision) {
	using namespace utf_utils;

	auto end { timeSpec.timeSpecCounter };
	// If the locale matches any of the below, they're taken care of by standard formatting via FormatCTime()
	if( auto name { loc.name() }; name == "" || name == "C" || name == "en_US" || name == "en_US.UTF8" ) {
			specValues.localize = false;    // set to false so that when writing to the container, it doesn't call the localization buffer
			return FormatCTime(timeStruct, precision, 0, end);
	}
	// Due to major shifts over to little endian back in the early 2000's, this is making the assumption that the system is LE and NOT BE.
	AF_ASSERT(utf_utils::IsLittleEndian(), "Big Endian Format Is Currently Unsupported. If Support Is Necessary, Please Open A New Issue At "
	                                       "'https://github.com/USAFrenzy/ArgFormatter/issues'");
	static std::basic_ostringstream<u_wchar> localeStream;
	auto pos { -1 };
	auto format { timeSpec.timeSpecFormat };
	auto& cont { timeSpec.timeSpecContainer };
	u_wstring localeFmt;
	localeFmt.reserve(cont.size() * 2);
	for( ;; ) {
			if( ++pos >= end ) break;
			if( cont[ pos ] != ' ' ) {
					localeFmt += '%';
					format[ pos ] == LocaleFormat::standard ? localeFmt.append(1, cont[ pos ]) : localeFmt.append(LocalizedFormat(cont[ pos ]));
			} else {
					localeFmt.append(1, ' ');
				}
		}
	localeStream.str(u_wstring {});
	localeStream.clear();
	localeStream.imbue(loc);
	localeStream << std::put_time<u_wchar>(&timeStruct, localeFmt.data());
	if constexpr( sizeof(u_wchar) == 2 ) {
			U16ToU8(std::move(localeStream.str()), timeSpec.localizationBuff, valueSize);
	} else {
			U32ToU8(std::move(localeStream.str()), timeSpec.localizationBuff, valueSize);
		}
}

inline void formatter::arg_formatter::ArgFormatter::FormatSubseconds(const int& precision) {
	std::array<char, 24> buff {};
	auto begin { buff.data() };
	auto subSeconds { std::chrono::floor<std::chrono::nanoseconds>(std::chrono::system_clock::now()) };
	auto length { std::to_chars(begin, begin + 24, subSeconds.time_since_epoch().count()).ptr - begin };
	//  Set 'pos' to be the offset that only deals with sub-seconds; the `pos -= (pos/10);` line is only needed due to the increment in the first check below
	auto pos { (length % 10) + (length / 10) };
	if( pos >= 10 ) pos -= (pos / 10);
	auto end { pos + precision + 1 };    // increment precision due to the increment in pos below
	if( !specValues.localize ) {
			buffer[ valueSize ] = '.';
			++valueSize;
			for( ;; ) {
					if( ++pos >= end ) break;
					buffer[ valueSize ] = buff[ pos ];
					++valueSize;
				}
	} else {
			auto& timeBuff { timeSpec.localizationBuff };
			timeBuff[ valueSize++ ] = '.';
			for( ;; ) {
					if( ++pos >= end ) break;
					timeBuff[ valueSize ] = buff[ pos ];
					++valueSize;
				}
		}
}

inline void formatter::arg_formatter::ArgFormatter::FormatUtcOffset() {
	auto& utcOffset { formatter::globals::UtcOffset() };
	auto hours { std::chrono::duration_cast<std::chrono::hours>(utcOffset).count() };
	if( hours < 0 ) hours *= -1;
	auto min { static_cast<int>(hours * 0.166f) };
	if( !specValues.localize ) {
			buffer[ valueSize ] = (utcOffset.count() >= 0) ? '+' : '-';
			++valueSize;
	}
	Format24HM(hours, min);
}

inline void formatter::arg_formatter::ArgFormatter::FormatTZName() {
	std::string_view name { formatter::globals::TZInfo().abbrev };
	auto size { name.size() };
	int pos {};
	for( ;; ) {
			buffer[ valueSize ] = name[ pos ];
			++valueSize;
			if( ++pos >= size ) return;
		}
}

//  offset used to get the decimal value represented in this use case (same as '0' but faster by a few nanoseconds for direct operations involving this)
static constexpr int NumericalAsciiOffset { 48 };
template<typename IntergralType>
requires std::is_integral_v<std::remove_cvref_t<IntergralType>>
static constexpr size_t se_from_chars(const char* begin, IntergralType&& value) {
	if( !IsDigit(*begin) ) return 0;
	value = *begin - NumericalAsciiOffset;
	size_t digits { 1 };
	for( ;; ) {
			if( !IsDigit(*(++begin)) ) return digits;
			(value *= 10) += (*begin - NumericalAsciiOffset);
			++digits;
		}
}

constexpr auto fillBuffDefaultCapacity { 256 };
inline constexpr formatter::arg_formatter::ArgFormatter::ArgFormatter()
	: argCounter(0), m_indexMode(IndexMode::automatic), bracketResults(BracketSearchResults {}), specValues(SpecFormatting {}), argStorage(ArgContainer {}),
	  customStorage(ArgContainer {}), buffer(std::array<char, AF_ARG_BUFFER_SIZE> {}), valueSize(size_t {}), fillBuffer(std::vector<char> {}),
	  errHandle(formatter::af_errors::error_handler {}), timeSpec(TimeSpecs {}), lastRootCounter(0) {
	// Initialize now to lower the initial cost when formatting (brings initial cost from ~33us down to ~11us). The Call To
	// UtcOffset() will initialize TimeZoneInstance() via TimeZone() via TZInfo() -> thereby initializing  all function statics.
	// NOTE: Would still love a constexpr friendly version of this but I'm not finding anything online that says that might be remotely possible
	//               using the standard and I'd rather not have the cost of initialization fall on a logging call and rather fall on the construction call instead.
	if( !std::is_constant_evaluated() ) formatter::globals::UtcOffset();
	fillBuffer.reserve(fillBuffDefaultCapacity);
}

inline constexpr void formatter::arg_formatter::BracketSearchResults::Reset() {
	if( !std::is_constant_evaluated() ) {
			std::memset(this, 0, sizeof(BracketSearchResults));
	} else {
			beginPos = endPos = 0;
		}
}

inline constexpr void formatter::arg_formatter::TimeSpecs::Reset() {
	// the arrays don't need anything special considering values
	// get overridden by index assignment with the counter
	timeSpecCounter = 0;
	localizationBuff.clear();
}

inline constexpr void formatter::arg_formatter::SpecFormatting::ResetSpecs() {
	if( !std::is_constant_evaluated() ) {
			std::memset(this, 0, sizeof(SpecFormatting));
	} else {
			argPosition = nestedWidthArgPos = nestedPrecArgPos = 0;
			localize = hasAlt = hasClosingBrace = false;
			alignmentPadding = precision = 0;
			fillCharacter = typeSpec = '\0';
			align                    = Alignment::Empty;
			signType                 = Sign::Empty;
			preAltForm               = "";
		}
}

inline static constexpr std::vector<char> ConvBuffToSigned(std::vector<unsigned char>& buff, size_t endPos) {
	std::vector<char> signedTemp(endPos);
	auto pos { -1 };
	for( ;; ) {
			if( ++pos >= endPos ) break;
			signedTemp[ pos ] = static_cast<signed char>(buff[ pos ]);
		}
	return signedTemp;
}

template<typename T, typename U>
requires utf_utils::utf_constraints::IsSupportedUSource<T> && utf_utils::utf_constraints::IsSupportedUContainer<U>
constexpr void formatter::arg_formatter::ArgFormatter::WriteToContainer(T&& buff, size_t endPos, U&& container) {
	namespace se_con = utf_utils::utf_constraints;
	using CharType   = typename formatter::internal_helper::af_typedefs::type<U>::value_type;
	if constexpr( std::is_same_v<CharType, char> ) {
			// Assume utf-8 encoding and just handle as byte strings (as it should have been stored as such internally)
			if constexpr( std::is_same_v<typename formatter::internal_helper::af_typedefs::type<T>::value_type, unsigned char> && std::is_signed_v<char> ) {
					// unsigned  char -> char
					auto tmp { std::move(ConvBuffToSigned(buff, endPos)) };
					if constexpr( se_con::is_string_v<U> ) {
							if( endPos == 1 ) {
									container += tmp[ 0 ];
							} else {
									container.append(tmp.data(), endPos);
								}
					} else if constexpr( se_con::is_vector_v<U> ) {
							switch( endPos ) {
									case 0: return;
									case 1: container.emplace_back(tmp[ 0 ]); return;
									case 2:
										container.emplace_back(tmp[ 0 ]);
										container.emplace_back(tmp[ 1 ]);
										return;
									case 3:
										container.emplace_back(tmp[ 0 ]);
										container.emplace_back(tmp[ 1 ]);
										container.emplace_back(tmp[ 2 ]);
										return;
									default: container.insert(container.end(), tmp.begin(), tmp.begin() + endPos); return;
								}
					} else {
							std::copy_n(tmp.begin(), endPos, std::back_inserter(std::forward<U>(container)));
							return;
						}
			} else {
					// char -> char
					if constexpr( se_con::is_string_v<U> ) {
							if( endPos == 1 ) {
									container += buff[ 0 ];
							} else {
									container.append(buff.data(), endPos);
								}
					} else if constexpr( se_con::is_vector_v<U> ) {
							switch( endPos ) {
									case 0: return;
									case 1: container.emplace_back(buff[ 0 ]); return;
									case 2:
										container.emplace_back(buff[ 0 ]);
										container.emplace_back(buff[ 1 ]);
										return;
									case 3:
										container.emplace_back(buff[ 0 ]);
										container.emplace_back(buff[ 1 ]);
										container.emplace_back(buff[ 2 ]);
										return;
									default: container.insert(container.end(), buff.begin(), buff.begin() + endPos); return;
								}
					} else {
							std::copy_n(buff.begin(), endPos, std::back_inserter(std::forward<U>(container)));
							return;
						}
				}
	} else if constexpr( se_con::is_u16_type_string_v<U> ) {
			AF_ASSERT(utf_utils::IsLittleEndian(), "Big Endian Format Is Currently Unsupported. If Support Is Necessary, Please Open A New Issue At "
			                                       "'https://github.com/USAFrenzy/ArgFormatter/issues'");
			// Assume utf-16 encoding and convert from utf-8
			if constexpr( se_con::is_string_v<U> || se_con::is_vector_v<U> ) {
					utf_utils::U8ToU16(buff, std::forward<U>(container));
			} else {
					std::u16string tmp;
					tmp.reserve(endPos);
					utf_utils::U8ToU16(buff, tmp);
					std::copy_n(tmp.begin(), endPos, std::back_inserter(std::forward<U>(container)));
				}
	} else if constexpr( se_con::is_u32_type_string_v<U> ) {
			AF_ASSERT(utf_utils::IsLittleEndian(), "Big Endian Format Is Currently Unsupported. If Support Is Necessary, Please Open A New Issue At "
			                                       "'https://github.com/USAFrenzy/ArgFormatter/issues'");
			// Assume utf-32 encoding and convert from utf-8
			if constexpr( se_con::is_string_v<U> || se_con::is_vector_v<U> ) {
					utf_utils::U8ToU32(buff, std::forward<U>(container));
			} else {
					std::u32string tmp;
					tmp.reserve(endPos);
					utf_utils::U8ToU32(buff, tmp);
					std::copy_n(tmp.begin(), endPos, std::back_inserter(std::forward<U>(container)));
				}
	}
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteBufferToContainer(T&& container) {
	if( !specValues.localize ) {
			WriteToContainer(buffer, valueSize, std::forward<T>(container));
	} else {
			auto& localeBuff { timeSpec.localizationBuff };
			if constexpr( std::is_signed_v<char> ) {
					WriteToContainer(std::move(ConvBuffToSigned(localeBuff, valueSize)), valueSize, std::forward<T>(container));
			} else {
					WriteToContainer(localeBuff, valueSize, std::forward<T>(container));
				}
		}
}

template<typename Iter, typename... Args> constexpr auto formatter::arg_formatter::ArgFormatter::CaptureArgs(Iter&& iter, Args&&... args) -> decltype(iter) {
	if( argStorage.isCustomFormatter ) {
			return std::move(customStorage.CaptureArgs(std::forward<Iter>(iter), std::forward<Args>(args)...));
	} else {
			return std::move(argStorage.CaptureArgs(std::forward<Iter>(iter), std::forward<Args>(args)...));
		}
}

template<typename T, typename... Args>
constexpr void formatter::arg_formatter::ArgFormatter::format_to(std::back_insert_iterator<T>&& Iter, std::string_view sv, Args&&... args) {
	lastRootCounter = argCounter;
	ParseFormatString(std::move(CaptureArgs(std::move(Iter), std::forward<Args>(args)...)), sv);
	argStorage.isCustomFormatter = false;
	argCounter                   = lastRootCounter;
}

template<typename T, typename... Args>
constexpr void formatter::arg_formatter::ArgFormatter::format_to(std::back_insert_iterator<T>&& Iter, const std::locale& loc, std::string_view sv, Args&&... args) {
	lastRootCounter = argCounter;
	ParseFormatString(std::move(CaptureArgs(std::move(Iter), std::forward<Args>(args)...)), loc, sv);
	argStorage.isCustomFormatter = false;
	argCounter                   = lastRootCounter;
}

template<typename... Args> std::string formatter::arg_formatter::ArgFormatter::format(std::string_view sv, Args&&... args) {
	std::string tmp;
	tmp.reserve(ReserveCapacity(std::forward<Args>(args)...));
	format_to(std::move(std::back_inserter(tmp)), sv, std::forward<Args>(args)...);
	return tmp;
}

template<typename... Args> std::string formatter::arg_formatter::ArgFormatter::format(const std::locale& loc, std::string_view sv, Args&&... args) {
	std::string tmp;
	tmp.reserve(ReserveCapacity(std::forward<Args>(args)...));
	format_to(std::move(std::back_inserter(tmp)), loc, sv, std::forward<Args>(args)...);
	return tmp;
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteAlignedLeft(T&& container, const int& totalWidth) {
	if( totalWidth > fillBuffDefaultCapacity ) fillBuffer.reserve(totalWidth);
	FillBuffWithChar(totalWidth);
	std::memcpy(fillBuffer.data(), buffer.data(), valueSize);
	WriteToContainer(fillBuffer, totalWidth, std::forward<T>(container));
}

template<typename T>
constexpr void formatter::arg_formatter::ArgFormatter::WriteAlignedLeft(T&& container, std::string_view val, const int& precision, const int& totalWidth) {
	if( totalWidth > fillBuffDefaultCapacity ) fillBuffer.reserve(totalWidth);
	FillBuffWithChar(totalWidth);
	std::memcpy(fillBuffer.data(), val.data(), precision);
	WriteToContainer(fillBuffer, totalWidth, std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteAlignedRight(T&& container, const int& totalWidth, const size_t& fillAmount) {
	if( totalWidth > fillBuffDefaultCapacity ) fillBuffer.reserve(totalWidth);
	FillBuffWithChar(totalWidth);
	std::memcpy(fillBuffer.data() + fillAmount, buffer.data(), valueSize);
	WriteToContainer(fillBuffer, totalWidth, std::forward<T>(container));
}

template<typename T>
constexpr void formatter::arg_formatter::ArgFormatter::WriteAlignedRight(T&& container, std::string_view val, const int& precision, const int& totalWidth,
                                                                         const size_t& fillAmount) {
	if( totalWidth > fillBuffDefaultCapacity ) fillBuffer.reserve(totalWidth);
	FillBuffWithChar(totalWidth);
	std::memcpy(fillBuffer.data() + fillAmount, val.data(), precision);
	WriteToContainer(fillBuffer, totalWidth, std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteAlignedCenter(T&& container, const int& totalWidth, const size_t& fillAmount) {
	if( totalWidth > fillBuffDefaultCapacity ) fillBuffer.reserve(totalWidth);
	FillBuffWithChar(totalWidth);
	std::memcpy(fillBuffer.data() + fillAmount, buffer.data(), valueSize);
	WriteToContainer(fillBuffer, totalWidth, std::forward<T>(container));
}

template<typename T>
constexpr void formatter::arg_formatter::ArgFormatter::WriteAlignedCenter(T&& container, std::string_view val, const int& precision, const int& totalWidth,
                                                                          const size_t& fillAmount) {
	if( totalWidth > fillBuffDefaultCapacity ) fillBuffer.reserve(totalWidth);
	FillBuffWithChar(totalWidth);
	std::memcpy(fillBuffer.data() + fillAmount, val.data(), precision);
	WriteToContainer(fillBuffer, totalWidth, std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteNonAligned(T&& container) {
	WriteToContainer(buffer, valueSize, std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteNonAligned(T&& container, std::string_view val, const int& precision) {
	WriteToContainer(val, precision, std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteSimplePadding(T&& container, const size_t& fillAmount) {
	WriteToContainer(buffer, fillAmount, std::forward<T>(container));
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FillBuffWithChar(const int& totalWidth) {
	!std::is_constant_evaluated() ? static_cast<void>(std::memset(fillBuffer.data(), specValues.fillCharacter, totalWidth))
								  : fillBuffer.resize(totalWidth, specValues.fillCharacter);
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::FormatAlignment(T&& container, const int& totalWidth) {
	if( auto fill { (totalWidth > valueSize) ? totalWidth - valueSize : 0 }; fill != 0 ) {
			switch( specValues.align ) {
					case Alignment::AlignLeft: return WriteAlignedLeft(std::forward<T>(container), totalWidth);
					case Alignment::AlignRight: return WriteAlignedRight(std::forward<T>(container), totalWidth, fill);
					case Alignment::AlignCenter: return WriteAlignedCenter(std::forward<T>(container), totalWidth, fill / 2);
					default: return WriteSimplePadding(std::forward<T>(container), fill);
				}
	} else {
			WriteNonAligned(std::forward<T>(container));
		}
}

template<typename T>
constexpr void formatter::arg_formatter::ArgFormatter::FormatAlignment(T&& container, std::string_view val, const int& totalWidth, int precision) {
	auto size { static_cast<int>(val.size()) };
	precision = precision != 0 ? precision > size ? size : precision : size;
	if( auto fill { totalWidth > size ? totalWidth - size : 0 }; fill != 0 ) {
			switch( specValues.align ) {
					case Alignment::AlignLeft: return WriteAlignedLeft(std::forward<T>(container), val, precision, totalWidth);
					case Alignment::AlignRight: return WriteAlignedRight(std::forward<T>(container), val, precision, totalWidth, fill);
					case Alignment::AlignCenter: return WriteAlignedCenter(std::forward<T>(container), val, precision, totalWidth, fill / 2);
					default: return WriteSimplePadding(std::forward<T>(container), fill);
				}
	} else {
			WriteNonAligned(std::forward<T>(container), val, precision);
		}
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::Format(T&& container, const msg_details::SpecType& argType) {
	auto& storage { argStorage.isCustomFormatter ? customStorage : argStorage };
	auto precision { specValues.nestedPrecArgPos != 0 ? argStorage.int_state(specValues.nestedPrecArgPos) : specValues.precision != 0 ? specValues.precision : 0 };
	auto totalWidth { specValues.nestedWidthArgPos != 0  ? argStorage.int_state(specValues.nestedWidthArgPos)
		              : specValues.alignmentPadding != 0 ? specValues.alignmentPadding
		                                                 : 0 };
	if( totalWidth == 0 ) {    // use this check to guard from any unnecessary additional checks
			if( IsSimpleSubstitution(argType, precision) ) {
					// Handles The Case Of No Specifiers And No Alignment
					return WriteSimpleValue(std::forward<T>(container), argType);
			}
			// Handles The Case Of Specifiers And No Alignment
			switch( argType ) {
					default:
						!specValues.localize ? FormatArgument(precision, argType) : LocalizeArgument(default_locale, precision, argType);
						WriteBufferToContainer(std::forward<T>(container));
						return;
					case SpecType::StringViewType: [[fallthrough]];
					case SpecType::CharPointerType: [[fallthrough]];
					case SpecType::StringType: WriteFormattedString(std::forward<T>(container), argType, precision); return;
				}
	}
	// Handles The Case Of Specifiers WITH Alignment
	switch( argType ) {
			default:
				!specValues.localize ? FormatArgument(precision, argType) : LocalizeArgument(default_locale, precision, argType);
				FormatAlignment(std::forward<T>(container), totalWidth);
				return;
			case SpecType::StringViewType:
				FormatAlignment(std::forward<T>(container), storage.string_view_state(specValues.argPosition), totalWidth, precision);
				return;
			case SpecType::CharPointerType:
				FormatAlignment(std::forward<T>(container), storage.c_string_state(specValues.argPosition), totalWidth, precision);
				return;
			case SpecType::StringType: FormatAlignment(std::forward<T>(container), storage.string_state(specValues.argPosition), totalWidth, precision); return;
		}
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::Format(T&& container, const std::locale& loc, const msg_details::SpecType& argType) {
	auto& storage { argStorage.isCustomFormatter ? customStorage : argStorage };
	auto precision { specValues.nestedPrecArgPos != 0 ? storage.int_state(specValues.nestedPrecArgPos) : specValues.precision != 0 ? specValues.precision : 0 };
	auto totalWidth { specValues.nestedWidthArgPos != 0  ? storage.int_state(specValues.nestedWidthArgPos)
		              : specValues.alignmentPadding != 0 ? specValues.alignmentPadding
		                                                 : 0 };
	if( totalWidth == 0 ) {    // use this check to guard from any unnecessary additional checks
			if( IsSimpleSubstitution(argType, precision) ) {
					// Handles The Case Of No Specifiers And No Alignment
					return WriteSimpleValue(std::forward<T>(container), argType);
			}
			// Handles The Case Of Specifiers And No Alignment
			switch( argType ) {
					default:
						!specValues.localize ? FormatArgument(precision, argType) : LocalizeArgument(loc, precision, argType);
						WriteBufferToContainer(std::forward<T>(container));
						return;
					case SpecType::StringViewType: [[fallthrough]];
					case SpecType::CharPointerType: [[fallthrough]];
					case SpecType::StringType: WriteFormattedString(std::forward<T>(container), argType, precision); return;
				}
	}
	// Handles The Case Of Specifiers WITH Alignment
	switch( argType ) {
			default:
				!specValues.localize ? FormatArgument(precision, argType) : LocalizeArgument(loc, precision, argType);
				FormatAlignment(std::forward<T>(container), totalWidth);
				return;
			case SpecType::StringViewType:
				FormatAlignment(std::forward<T>(container), storage.string_view_state(specValues.argPosition), totalWidth, precision);
				return;
			case SpecType::CharPointerType:
				FormatAlignment(std::forward<T>(container), storage.c_string_state(specValues.argPosition), totalWidth, precision);
				return;
			case SpecType::StringType: FormatAlignment(std::forward<T>(container), storage.string_state(specValues.argPosition), totalWidth, precision); return;
		}
}

inline constexpr void formatter::arg_formatter::ArgFormatter::VerifyTimeSpec(std::string_view sv, size_t& pos) {
	// this is for c-time formatting
	auto size { sv.size() };
	auto& counterRef { timeSpec.timeSpecCounter };
	auto counter { counterRef };

	for( ;; ) {
			switch( sv[ pos ] ) {
					case 'E':
						switch( ++pos < size ? sv[ pos ] : '}' ) {
								case 'c': [[fallthrough]];
								case 'x': [[fallthrough]];
								case 'y': [[fallthrough]];
								case 'C': [[fallthrough]];
								case 'X': [[fallthrough]];
								case 'Y':
									timeSpec.timeSpecFormat[ counter ]    = LocaleFormat::localized;
									timeSpec.timeSpecContainer[ counter ] = sv[ pos ];
									++counter;
									break;
								default: errHandle.ReportError(af_errors::ErrorType::invalid_ctime_spec);
							}
						break;
					case 'O':
						switch( ++pos < size ? sv[ pos ] : '}' ) {
								case 'd': [[fallthrough]];
								case 'e': [[fallthrough]];
								case 'm': [[fallthrough]];
								case 'u': [[fallthrough]];
								case 'w': [[fallthrough]];
								case 'y': [[fallthrough]];
								case 'H': [[fallthrough]];
								case 'I': [[fallthrough]];
								case 'M': [[fallthrough]];
								case 'S': [[fallthrough]];
								case 'U': [[fallthrough]];
								case 'V': [[fallthrough]];
								case 'W':
									timeSpec.timeSpecFormat[ counter ]    = LocaleFormat::localized;
									timeSpec.timeSpecContainer[ counter ] = sv[ pos ];
									++counter;
									break;
								default: errHandle.ReportError(af_errors::ErrorType::invalid_ctime_spec);
							}
						break;
					case 'T':
						{
							if( sv[ ++pos ] == '.' ) {
									timeSpec.timeSpecFormat[ counter ]    = LocaleFormat::standard;
									timeSpec.timeSpecContainer[ counter ] = 'T';
									++counter;
									if( ++pos >= size ) {
											--pos;
											break;
									}
									pos += se_from_chars(sv.data() + pos, specValues.precision);
									break;
							} else {
									timeSpec.timeSpecFormat[ counter ]    = LocaleFormat::standard;
									timeSpec.timeSpecContainer[ counter ] = 'T';
									++counter;
									--pos;
									break;
								}
						}
					case 'a': [[fallthrough]];
					case 'b': [[fallthrough]];
					case 'c': [[fallthrough]];
					case 'd': [[fallthrough]];
					case 'e': [[fallthrough]];
					case 'g': [[fallthrough]];
					case 'h': [[fallthrough]];
					case 'j': [[fallthrough]];
					case 'k': [[fallthrough]];
					case 'm': [[fallthrough]];
					case 'n': [[fallthrough]];
					case 'p': [[fallthrough]];
					case 'r': [[fallthrough]];
					case 't': [[fallthrough]];
					case 'u': [[fallthrough]];
					case 'w': [[fallthrough]];
					case 'x': [[fallthrough]];
					case 'y': [[fallthrough]];
					case 'z': [[fallthrough]];
					case 'A': [[fallthrough]];
					case 'B': [[fallthrough]];
					case 'C': [[fallthrough]];
					case 'D': [[fallthrough]];
					case 'F': [[fallthrough]];
					case 'G': [[fallthrough]];
					case 'H': [[fallthrough]];
					case 'I': [[fallthrough]];
					case 'M': [[fallthrough]];
					case 'R': [[fallthrough]];
					case 'S': [[fallthrough]];
					case 'U': [[fallthrough]];
					case 'V': [[fallthrough]];
					case 'W': [[fallthrough]];
					case 'X': [[fallthrough]];
					case 'Y': [[fallthrough]];
					case 'Z': [[fallthrough]];
					case '%': [[fallthrough]];
					default:
						timeSpec.timeSpecFormat[ counter ]    = LocaleFormat::standard;
						timeSpec.timeSpecContainer[ counter ] = sv[ pos ];
						++counter;
						break;
				}
			if( ++pos >= size ) {
					counterRef = counter;
					return;
			}
			switch( sv[ pos ] ) {
					case '%': ++pos >= size ? errHandle.ReportError(af_errors::ErrorType::missing_ctime_spec) : void(0); continue;
					case '}':
						{
							counterRef = counter;
							return;
						}
					default: continue;
				}
		}
}

inline constexpr void formatter::arg_formatter::ArgFormatter::VerifyFillAlignTimeField(std::string_view sv, size_t& currentPos) {
	const auto& ch { sv[ currentPos ] };
	switch( ++currentPos >= sv.size() ? '}' : sv[ currentPos ] ) {
			case '<': OnAlignLeft(ch, currentPos); return;
			case '>': OnAlignRight(ch, currentPos); return;
			case '^': OnAlignCenter(ch, currentPos); return;
			default:
				--currentPos;
				specValues.fillCharacter = ' ';
				break;
		}
}

inline constexpr void formatter::arg_formatter::ArgFormatter::VerifyTimePrecisionField(std::string_view sv, size_t& currentPosition) {
	using enum msg_details::SpecType;
	if( const auto& ch { sv[ ++currentPosition ] }; IsDigit(ch) ) {
			auto data { sv.data() };
			currentPosition += se_from_chars(data + currentPosition, specValues.precision);
			++argCounter;
			return;
	} else {
			switch( ch ) {
					case '{': VerifyPositionalField(sv, ++currentPosition, specValues.nestedPrecArgPos); return;
					case '}': VerifyPositionalField(sv, currentPosition, specValues.nestedPrecArgPos); return;
					default: errHandle.ReportError(af_errors::ErrorType::missing_bracket); return;
				}
		}
}

inline constexpr void formatter::arg_formatter::ArgFormatter::ParseTimeField(std::string_view sv, size_t& start) {
	timeSpec.Reset();
	auto svSize { sv.size() };
	if( sv[ start ] != '%' ) {
			VerifyFillAlignTimeField(sv, start);
			if( start >= svSize ) return;
	}
	if( (sv[ start ] == '{') || (sv[ start ] >= '1' && sv[ start ] <= '9') ) {
			VerifyWidthField(sv, start);
			if( start >= svSize ) return;
	}
	if( sv[ start ] == '.' ) {
			VerifyTimePrecisionField(sv, start);
			if( start >= svSize ) return;
	}
	if( sv[ start ] == 'L' ) {
			specValues.localize = true;
			if( ++start >= svSize ) return;
	}
	// assume the next ch is '%' and advance past it
	VerifyTimeSpec(sv, ++start);
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::FormatTimeField(T&& container) {
	auto& storage { argStorage.isCustomFormatter ? customStorage : argStorage };
	auto precision { specValues.nestedPrecArgPos != 0 ? storage.int_state(specValues.nestedPrecArgPos) : specValues.precision != 0 ? specValues.precision : 0 };
	auto totalWidth { specValues.nestedWidthArgPos != 0  ? storage.int_state(specValues.nestedWidthArgPos)
		              : specValues.alignmentPadding != 0 ? specValues.alignmentPadding
		                                                 : 0 };
	const auto& counter { timeSpec.timeSpecCounter };
	if( totalWidth == 0 && precision == 0 && counter < 2 ) {
			return WriteSimpleCTime(std::forward<T>(container));
	} else if( totalWidth == 0 ) {
			!specValues.localize ? FormatCTime(storage.c_time_state(specValues.argPosition), precision, 0, counter)
								 : LocalizeCTime(default_locale, storage.c_time_state(specValues.argPosition), precision);
			return WriteBufferToContainer(std::forward<T>(container));
	} else {
			!specValues.localize ? FormatCTime(storage.c_time_state(specValues.argPosition), precision, 0, counter)
								 : LocalizeCTime(default_locale, storage.c_time_state(specValues.argPosition), precision);
			FormatAlignment(std::forward<T>(container), totalWidth);
		}
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::FormatTimeField(T&& container, const std::locale& loc) {
	auto& storage { argStorage.isCustomFormatter ? customStorage : argStorage };
	auto precision { specValues.nestedPrecArgPos != 0 ? storage.int_state(specValues.nestedPrecArgPos) : specValues.precision != 0 ? specValues.precision : 0 };
	auto totalWidth { specValues.nestedWidthArgPos != 0  ? storage.int_state(specValues.nestedWidthArgPos)
		              : specValues.alignmentPadding != 0 ? specValues.alignmentPadding
		                                                 : 0 };
	;
	const auto& counter { timeSpec.timeSpecCounter };
	if( totalWidth == 0 && precision == 0 && counter < 2 ) {
			return WriteSimpleCTime(std::forward<T>(container));
	} else if( totalWidth == 0 ) {
			!specValues.localize ? FormatCTime(storage.c_time_state(specValues.argPosition), precision, counter)
								 : LocalizeCTime(loc, storage.c_time_state(specValues.argPosition), precision);
			return WriteBufferToContainer(std::forward<T>(container));
	} else {
			!specValues.localize ? FormatCTime(storage.c_time_state(specValues.argPosition), precision, counter)
								 : LocalizeCTime(loc, storage.c_time_state(specValues.argPosition), precision);
			FormatAlignment(std::forward<T>(container), totalWidth);
		}
}

inline constexpr void formatter::arg_formatter::ArgFormatter::Parse(std::string_view sv, size_t& start, const msg_details::SpecType& argType) {
	auto svSize { sv.size() };
	VerifyFillAlignField(sv, start, argType);
	if( start >= svSize ) return;
	switch( sv[ start ] ) {
			case '+':
				specValues.signType = Sign::Plus;
				if( ++start >= svSize ) return;
				break;
			case '-':
				specValues.signType = Sign::Minus;
				if( ++start >= svSize ) return;
				break;
			case ' ':
				specValues.signType = Sign::Space;
				if( ++start >= svSize ) return;
				break;
			default: break;
		}
	if( sv[ start ] == '#' ) {
			VerifyAltField(argType);
			if( ++start >= svSize ) return;
	}
	if( sv[ start ] == '0' ) {
			if( specValues.fillCharacter == '\0' ) {
					specValues.fillCharacter = '0';
			}
			if( ++start >= svSize ) return;
	}
	if( (sv[ start ] == '{') || (sv[ start ] >= '1' && sv[ start ] <= '9') ) {
			VerifyWidthField(sv, start);
			if( start >= svSize ) return;
	}
	if( sv[ start ] == '.' ) {
			VerifyPrecisionField(sv, start, argType);
			if( start >= svSize ) return;
	}
	if( sv[ start ] == 'L' ) {
			VerifyLocaleField(start, argType);
			if( start >= svSize ) return;
	}
	if( sv[ start ] != '}' ) {
			HandlePotentialTypeField(sv[ start ], argType);
	}
}

inline static constexpr std::string_view closeBracket { "}" };
template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::ParseFormatString(std::back_insert_iterator<T>&& Iter, std::string_view sv) {
	if( !std::is_constant_evaluated() ) {
			std::memset(buffer.data(), 0, AF_ARG_BUFFER_SIZE);
	} else {
			std::fill(buffer.begin(), buffer.begin() + valueSize, '\0');
		}
	valueSize   = 0;
	argCounter  = 0;
	m_indexMode = IndexMode::automatic;
	auto& container { internal_helper::IteratorAccessHelper(std::move(Iter)).Container() };
	for( ;; ) {
			specValues.ResetSpecs();
			const auto& storage { argStorage.isCustomFormatter ? customStorage : argStorage };
			// Check small sv size conditions and handle the niche cases, otherwise break and continue onward
			switch( sv.size() ) {
					case 0: return;
					case 1: WriteToContainer(sv, 1, container); return;
					case 2:
						// Handle If format string only contains '{}' and no other text
						if( sv[ 0 ] == '{' && sv[ 1 ] == '}' ) {
								switch( const auto& argType { storage.SpecTypesCaptured()[ 0 ] } ) {
										case SpecType::CustomType:
											{
												argStorage.isCustomFormatter = true;
												argStorage.custom_state(specValues.argPosition).FormatCallBack(sv);
												argStorage.isCustomFormatter = false;
												return;
											}
										default: WriteSimpleValue(container, argType); return;
									}
						}
						// Otherwise, write out any remaining characters as-is and bypass the check that would have found
						// this case in FindBrackets(). In most cases, ending early here saw ~2-3% gain in performance
						WriteToContainer(sv, 2, container);
						return;
					default: break;
				}
			// If the above wasn't executed, then find the first pair of curly brackets and if none were found, write out the parse string as-is
			if( !FindBrackets(sv) ) {
					WriteToContainer(sv, sv.size(), container);
					return;
			}
			// If the position of the first curly bracket found isn't the beginning of the parse string, then write the text as-is up until the bracket position
			auto& begin { bracketResults.beginPos };
			auto& end { bracketResults.endPos };
			if( begin > 0 ) {
					WriteToContainer(sv, begin, container);
					sv.remove_prefix(begin);
					end -= begin;
					begin = 0;
			}
			size_t pos { 0 };
			auto bracketSize { end - begin };
			std::string_view argBracket(sv.data() + 1, sv.data() + bracketSize + 1);
			/* Since we advance the begin position by 1 in the argBracket construction, if the first char is '{' then it was an escaped bracket */
			if( argBracket[ pos ] == '{' ) {
					WriteToContainer(closeBracket, 1, container);
					++pos;
			}
			// NOTE: Since a well-formed substitution bracket should end with '}' and we can assume it's well formed due to the check in FindBrackets(),
			//                argBracket[ bracketSize - 2 ] is used here to check if the position before the close bracket is another closing bracket instead.
			if( bracketSize > 3 && argBracket[ bracketSize - 2 ] == '}' ) specValues.hasClosingBrace = true;
			/************************************* Handle Positional Args *************************************/
			if( !VerifyPositionalField(argBracket, pos, specValues.argPosition) ) {
					// Nothing Else to Parse- just a simple substitution after position field so write it and continue parsing format string
					switch( const auto& argType { storage.SpecTypesCaptured()[ specValues.argPosition ] } ) {
							case SpecType::CustomType:
								{
									argStorage.isCustomFormatter = true;
									argStorage.custom_state(specValues.argPosition).FormatCallBack(argBracket);
									argStorage.isCustomFormatter = false;
									break;
								}
							case SpecType::CTimeType: WriteSimpleCTime(container); break;
							default: WriteSimpleValue(container, argType); break;
						}
					if( specValues.hasClosingBrace ) {
							WriteToContainer(closeBracket, 1, container);
					}
					sv.remove_prefix(bracketSize + 1);
					continue;
			}
			/****************************** Handle What's Left Of The Bracket ******************************/
			switch( const auto& argType { storage.SpecTypesCaptured()[ specValues.argPosition ] } ) {
					case SpecType::CustomType:
						{
							argStorage.isCustomFormatter = true;
							argStorage.custom_state(specValues.argPosition).FormatCallBack(argBracket);
							argStorage.isCustomFormatter = false;
							break;
						}
					case SpecType::CTimeType:
						ParseTimeField(argBracket, pos);
						FormatTimeField(container);
						break;
					default:
						Parse(argBracket, pos, argType);
						Format(container, argType);
						break;
				}
			if( specValues.hasClosingBrace ) {
					WriteToContainer(closeBracket, 1, container);
			}
			sv.remove_prefix(bracketSize + 1);
		}
}

template<typename T>
constexpr void formatter::arg_formatter::ArgFormatter::ParseFormatString(std::back_insert_iterator<T>&& Iter, const std::locale& loc, std::string_view sv) {
	if( !std::is_constant_evaluated() ) {
			std::memset(buffer.data(), 0, AF_ARG_BUFFER_SIZE);
	} else {
			std::fill(buffer.begin(), buffer.begin() + valueSize, '\0');
		}
	valueSize   = 0;
	argCounter  = 0;
	m_indexMode = IndexMode::automatic;
	auto& container { internal_helper::IteratorAccessHelper(std::move(Iter)).Container() };
	for( ;; ) {
			specValues.ResetSpecs();
			const auto& storage { argStorage.isCustomFormatter ? customStorage : argStorage };
			// Check small sv size conditions and handle the niche cases, otherwise break and continue onward
			switch( sv.size() ) {
					case 0: return;
					case 1: WriteToContainer(sv, 1, container); return;
					case 2:
						// Handle If format string only contains '{}' and no other text
						if( sv[ 0 ] == '{' && sv[ 1 ] == '}' ) {
								switch( const auto& argType { storage.SpecTypesCaptured()[ 0 ] } ) {
										case SpecType::CustomType:
											{
												argStorage.isCustomFormatter = true;
												argStorage.custom_state(specValues.argPosition).FormatCallBack(sv);
												argStorage.isCustomFormatter = false;
												return;
											}
										default: WriteSimpleValue(container, argType); return;
									}
						}
						// Otherwise, write out any remaining characters as-is and bypass the check that would have found
						// this case in FindBrackets(). In most cases, ending early here saw ~2-3% gain in performance
						WriteToContainer(sv, 2, container);
						return;
					default: break;
				}
			// If the above wasn't executed, then find the first pair of curly brackets and if none were found, write out the parse string as-is
			if( !FindBrackets(sv) ) {
					WriteToContainer(sv, sv.size(), container);
					return;
			}
			// If the position of the first curly bracket found isn't the beginning of the parse string, then write the text as-is up until the bracket position
			auto& begin { bracketResults.beginPos };
			auto& end { bracketResults.endPos };
			if( begin > 0 ) {
					WriteToContainer(sv, begin, container);
					sv.remove_prefix(begin);
					end -= begin;
					begin = 0;
			}
			size_t pos { 0 };
			auto bracketSize { end - begin };
			std::string_view argBracket(sv.data() + 1, sv.data() + bracketSize + 1);
			/* Since we advance the begin position by 1 in the argBracket construction, if the first char is '{' then it was an escaped bracket */
			if( argBracket[ pos ] == '{' ) {
					WriteToContainer(closeBracket, 1, container);
					++pos;
			}
			// NOTE: Since a well-formed substitution bracket should end with '}' and we can assume it's well formed due to the check in FindBrackets(),
			//                argBracket[ bracketSize - 2 ] is used here to check if the position before the close bracket is another closing bracket instead.
			if( bracketSize > 3 && argBracket[ bracketSize - 2 ] == '}' ) specValues.hasClosingBrace = true;
			/************************************* Handle Positional Args *************************************/
			if( !VerifyPositionalField(argBracket, pos, specValues.argPosition) ) {
					// Nothing Else to Parse- just a simple substitution after position field so write it and continue parsing format string
					switch( const auto& argType { storage.SpecTypesCaptured()[ specValues.argPosition ] } ) {
							case SpecType::CustomType:
								{
									argStorage.isCustomFormatter = true;
									argStorage.custom_state(specValues.argPosition).FormatCallBack(argBracket);
									argStorage.isCustomFormatter = false;
									break;
								}
							case SpecType::CTimeType: WriteSimpleCTime(container); break;
							default: WriteSimpleValue(container, argType); break;
						}
					if( specValues.hasClosingBrace ) {
							WriteToContainer(closeBracket, 1, container);
					}
					sv.remove_prefix(bracketSize + 1);
					continue;
			}
			/****************************** Handle What's Left Of The Bracket ******************************/
			switch( const auto& argType { storage.SpecTypesCaptured()[ specValues.argPosition ] } ) {
					case SpecType::CustomType:
						{
							argStorage.isCustomFormatter = true;
							argStorage.custom_state(specValues.argPosition).FormatCallBack(argBracket);
							argStorage.isCustomFormatter = false;
							break;
						}
					case SpecType::CTimeType:
						ParseTimeField(argBracket, pos);
						FormatTimeField(container, loc);
						break;
					default:
						Parse(argBracket, pos, argType);
						Format(container, loc, argType);
						break;
				}
			if( specValues.hasClosingBrace ) {
					WriteToContainer(closeBracket, 1, container);
			}
			sv.remove_prefix(bracketSize + 1);
		}
}

inline constexpr bool formatter::arg_formatter::ArgFormatter::FindBrackets(std::string_view sv) {
	const auto svSize { sv.size() };
	if( svSize < 3 ) return false;
	auto& begin { bracketResults.beginPos };
	auto& end { bracketResults.endPos };
	begin = end = 0;
	for( ;; ) {
			if( sv[ begin ] == '{' ) break;
			if( ++begin >= svSize ) return false;
		}
	end = begin;
	for( ;; ) {
			if( ++end >= svSize ) errHandle.ReportError(af_errors::ErrorType::missing_bracket);
			switch( sv[ end ] ) {
					case '}': return true; break;
					case '{':
						for( ;; ) {
								if( ++end >= svSize ) errHandle.ReportError(af_errors::ErrorType::missing_bracket);
								if( sv[ end ] != '}' ) continue;
								for( ;; ) {
										if( ++end >= svSize ) errHandle.ReportError(af_errors::ErrorType::missing_bracket);
										if( sv[ end ] != '}' ) continue;
										return true;
									}
							}
					default: continue;
				}
		}
}

inline constexpr bool formatter::arg_formatter::ArgFormatter::VerifyPositionalField(std::string_view sv, size_t& start, unsigned char& positionValue) {
	if( m_indexMode == IndexMode::automatic ) {
			// we're in automatic mode
			auto& valueType { argStorage.isCustomFormatter ? customStorage.SpecTypesCaptured() : argStorage.SpecTypesCaptured() };
			if( const auto& ch { sv[ start ] }; IsDigit(ch) ) {
					m_indexMode = IndexMode::manual;
					return VerifyPositionalField(sv, start, positionValue);
			} else if( ch == '}' ) {
					positionValue = argCounter;
					if( valueType[ ++argCounter ] == formatter::msg_details::SpecType::MonoType ) {
							--argCounter;
					}
					return false;
			} else if( ch == ':' ) {
					positionValue = argCounter;
					if( valueType[ ++argCounter ] == formatter::msg_details::SpecType::MonoType ) {
							--argCounter;
					}
					++start;
					return true;
			} else if( ch == ' ' ) {
					for( ;; ) {
							if( start >= sv.size() ) break;
							if( sv[ start++ ] != ' ' ) break;
						}
					switch( sv[ start ] ) {
							case ':': [[fallthrough]];
							case '}':
								positionValue = argCounter;
								if( valueType[ ++argCounter ] == formatter::msg_details::SpecType::MonoType ) {
										--argCounter;
								}
								++start;
								return true;
								break;
							default: errHandle.ReportError(af_errors::ErrorType::position_field_spec); break;
						}
			}
	} else {
			// we're in manual mode
			auto data { sv.data() };
			start += se_from_chars(data + start, positionValue);
			if( start != 0 ) {
					AF_ASSERT(positionValue <= MAX_ARG_INDEX, "Error In Position Argument Field: Max Position (24) Exceeded.");
					switch( sv[ start ] ) {
							case ':':
								++argCounter;
								++start;
								return true;
							case '}': ++start; return false;
							default: errHandle.ReportError(af_errors::ErrorType::position_field_mode); break;
						}
			} else {
					switch( sv[ start ] ) {
							case '}': errHandle.ReportError(af_errors::ErrorType::position_field_mode); break;
							case ' ':
								{
									auto size { sv.size() };
									for( ;; ) {
											if( start >= size ) break;
											if( sv[ ++start ] != ' ' ) break;
										}
									return VerifyPositionalField(sv, start, positionValue);
									break;
								}
							case ':': errHandle.ReportError(af_errors::ErrorType::position_field_no_position); break;
							default: errHandle.ReportError(af_errors::ErrorType::position_field_runon); break;
						}
				}
		}
	errHandle.ReportError(af_errors::ErrorType::none);
}

inline constexpr void formatter::arg_formatter::ArgFormatter::OnAlignLeft(const char& ch, size_t& pos) {
	specValues.align = Alignment::AlignLeft;
	++pos;
	if( ch == ':' ) {
			specValues.fillCharacter = ' ';
	} else if( ch != '{' && ch != '}' ) {
			specValues.fillCharacter = ch;
	} else {
			errHandle.ReportError(af_errors::ErrorType::invalid_fill_character);
		}
}
inline constexpr void formatter::arg_formatter::ArgFormatter::OnAlignRight(const char& ch, size_t& pos) {
	specValues.align = Alignment::AlignRight;
	++pos;
	if( ch == ':' ) {
			specValues.fillCharacter = ' ';
	} else if( ch != '{' && ch != '}' ) {
			specValues.fillCharacter = ch;
	} else {
			errHandle.ReportError(af_errors::ErrorType::invalid_fill_character);
		}
}
inline constexpr void formatter::arg_formatter::ArgFormatter::OnAlignCenter(const char& ch, size_t& pos) {
	specValues.align = Alignment::AlignCenter;
	++pos;
	if( ch == ':' ) {
			specValues.fillCharacter = ' ';
	} else if( ch != '{' && ch != '}' ) {
			specValues.fillCharacter = ch;
	} else {
			errHandle.ReportError(af_errors::ErrorType::invalid_fill_character);
		}
}
inline constexpr void formatter::arg_formatter::ArgFormatter::OnAlignDefault(const SpecType& argType, size_t& pos) {
	using enum msg_details::SpecType;
	--pos;
	switch( argType ) {
			case MonoType: return;
			case IntType: [[fallthrough]];
			case U_IntType: [[fallthrough]];
			case DoubleType: [[fallthrough]];
			case FloatType: [[fallthrough]];
			case LongDoubleType: [[fallthrough]];
			case LongLongType: [[fallthrough]];
			case U_LongLongType: specValues.align = Alignment::AlignRight; break;
			default: specValues.align = Alignment::AlignLeft; break;
		}
	specValues.fillCharacter = ' ';
}

inline constexpr void formatter::arg_formatter::ArgFormatter::VerifyFillAlignField(std::string_view sv, size_t& currentPos, const msg_details::SpecType& argType) {
	const auto& ch { sv[ currentPos ] };
	// handle padding with and without argument to pad
	/*
	    format("{:*5}")        results in -> *****
	    format("{:*5}", 'a')  results in -> a****
	*/
	if( argType == SpecType::MonoType ) {
			specValues.fillCharacter = ch;
			// for monotype cases, don't have to worry about alignment, so just check if the position needs to be adjusted or not
			switch( ++currentPos >= sv.size() ? sv.back() : sv[ currentPos ] ) {
					default: return;
					case '<': [[fallthrough]];
					case '>': [[fallthrough]];
					case '^': ++currentPos; return;
				}
	}

	switch( ++currentPos >= sv.size() ? '}' : sv[ currentPos ] ) {
			case '<': OnAlignLeft(ch, currentPos); return;
			case '>': OnAlignRight(ch, currentPos); return;
			case '^': OnAlignCenter(ch, currentPos); return;
			default: OnAlignDefault(argType, currentPos); return;
		}
}

inline constexpr void formatter::arg_formatter::ArgFormatter::VerifyAltField(const msg_details::SpecType& argType) {
	using enum msg_details::SpecType;
	switch( argType ) {
			case IntType: [[fallthrough]];
			case U_IntType: [[fallthrough]];
			case DoubleType: [[fallthrough]];
			case FloatType: [[fallthrough]];
			case LongDoubleType: [[fallthrough]];
			case LongLongType: [[fallthrough]];
			case U_LongLongType: [[fallthrough]];
			case CharType: [[fallthrough]];
			case BoolType: specValues.hasAlt = true; return;
			default: errHandle.ReportError(af_errors::ErrorType::invalid_alt_type); break;
		}
}

inline constexpr void formatter::arg_formatter::ArgFormatter::VerifyWidthField(std::string_view sv, size_t& currentPosition) {
	if( const auto& ch { sv[ currentPosition ] }; IsDigit(ch) ) {
			auto svData { sv.data() };
			currentPosition += se_from_chars(svData + currentPosition, specValues.alignmentPadding);
			return;
	} else {
			switch( ch ) {
					case '{': VerifyPositionalField(sv, ++currentPosition, specValues.nestedWidthArgPos); return;
					case '}': VerifyPositionalField(sv, currentPosition, specValues.nestedWidthArgPos); return;
					default: errHandle.ReportError(af_errors::ErrorType::missing_bracket); return;
				}
		}
}

inline constexpr void formatter::arg_formatter::ArgFormatter::VerifyPrecisionField(std::string_view sv, size_t& currentPosition, const msg_details::SpecType& argType) {
	using enum msg_details::SpecType;
	switch( argType ) {
			case StringType: [[fallthrough]];
			case StringViewType: [[fallthrough]];
			case CharPointerType: [[fallthrough]];
			case FloatType: [[fallthrough]];
			case DoubleType: [[fallthrough]];
			case LongDoubleType: break;
			default: errHandle.ReportError(af_errors::ErrorType::invalid_precision_type); break;
		}
	if( const auto& ch { sv[ ++currentPosition ] }; IsDigit(ch) ) {
			auto data { sv.data() };
			currentPosition += se_from_chars(data + currentPosition, specValues.precision);
			++argCounter;
			return;
	} else {
			switch( ch ) {
					case '{': VerifyPositionalField(sv, ++currentPosition, specValues.nestedPrecArgPos); return;
					case '}': VerifyPositionalField(sv, currentPosition, specValues.nestedPrecArgPos); return;
					default: errHandle.ReportError(af_errors::ErrorType::missing_bracket); return;
				}
		}
}

inline constexpr void formatter::arg_formatter::ArgFormatter::VerifyLocaleField(size_t& currentPosition, const msg_details::SpecType& argType) {
	using SpecType = msg_details::SpecType;
	++currentPosition;
	switch( argType ) {
			case SpecType::CharType: [[fallthrough]];
			case SpecType::ConstVoidPtrType: [[fallthrough]];
			case SpecType::MonoType: [[fallthrough]];
			case SpecType::StringType: [[fallthrough]];
			case SpecType::StringViewType: [[fallthrough]];
			case SpecType::VoidPtrType: errHandle.ReportError(af_errors::ErrorType::invalid_locale_type); break;
			default: specValues.localize = true; return;
		}
}

inline constexpr bool formatter::arg_formatter::ArgFormatter::IsSimpleSubstitution(const msg_details::SpecType& argType, const int& prec) {
	using enum SpecType;
	switch( argType ) {
			case StringType: [[fallthrough]];
			case CharPointerType: [[fallthrough]];
			case StringViewType: return prec == 0;
			case IntType: [[fallthrough]];
			case U_IntType: [[fallthrough]];
			case LongLongType: [[fallthrough]];
			case U_LongLongType: return !specValues.hasAlt && specValues.signType == Sign::Empty && !specValues.localize && specValues.typeSpec == '\0';
			case BoolType: return !specValues.hasAlt && (specValues.typeSpec == '\0' || specValues.typeSpec == 's');
			case CharType: return !specValues.hasAlt && (specValues.typeSpec == '\0' || specValues.typeSpec == 'c');
			case FloatType: [[fallthrough]];
			case DoubleType: [[fallthrough]];
			case LongDoubleType:
				return !specValues.localize && prec == 0 && specValues.signType == Sign::Empty && !specValues.hasAlt && specValues.typeSpec == '\0';
				// for pointer types, if the width field is 0, there's no fill/alignment to take into account and therefore it's a simple sub
			case ConstVoidPtrType: [[fallthrough]];
			case VoidPtrType: return true;
			default: return false; break;
		}
}

inline constexpr void formatter::arg_formatter::ArgFormatter::OnValidTypeSpec(const SpecType& type, const char& ch) {
	using namespace std::literals::string_view_literals;
	using enum msg_details::SpecType;
	specValues.typeSpec = ch;
	if( !specValues.hasAlt ) return;
	switch( type ) {
			default: return;
			case IntType: [[fallthrough]];
			case U_IntType: [[fallthrough]];
			case LongLongType: [[fallthrough]];
			case U_LongLongType: [[fallthrough]];
			case BoolType: [[fallthrough]];
			case CharType:
				switch( ch ) {
						case 'b': specValues.preAltForm = "0b"sv; return;
						case 'B': specValues.preAltForm = "0B"sv; return;
						case 'o': specValues.preAltForm = "0"sv; return;
						case 'x': specValues.preAltForm = "0x"sv; return;
						case 'X': specValues.preAltForm = "0X"sv; return;
						default: break;
					}
		}
}

inline constexpr void formatter::arg_formatter::ArgFormatter::OnInvalidTypeSpec(const SpecType& type) {
	using enum msg_details::SpecType;
	switch( type ) {
			case IntType: [[fallthrough]];
			case U_IntType: [[fallthrough]];
			case LongLongType: [[fallthrough]];
			case U_LongLongType: errHandle.ReportError(af_errors::ErrorType::invalid_int_spec); break;
			case FloatType: [[fallthrough]];
			case DoubleType: [[fallthrough]];
			case LongDoubleType: errHandle.ReportError(af_errors::ErrorType::invalid_float_spec); break;
			case StringType: [[fallthrough]];
			case CharPointerType: [[fallthrough]];
			case StringViewType: errHandle.ReportError(af_errors::ErrorType::invalid_string_spec); break;
			case BoolType: errHandle.ReportError(af_errors::ErrorType::invalid_bool_spec); break;
			case CharType: errHandle.ReportError(af_errors::ErrorType::invalid_char_spec); break;
			case ConstVoidPtrType: [[fallthrough]];
			case VoidPtrType: errHandle.ReportError(af_errors::ErrorType::invalid_pointer_spec); break;
			case MonoType: [[fallthrough]];
			case CustomType: [[fallthrough]];
			case CTimeType: return;    // possibly issue warning on a af_typedefs::type spec being provided on no argument?
			default: break;
		}
}

inline constexpr void formatter::arg_formatter::ArgFormatter::HandlePotentialTypeField(const char& ch, const msg_details::SpecType& argType) {
	switch( ch ) {
			case 'a': [[fallthrough]];
			case 'A': [[fallthrough]];
			case 'b': [[fallthrough]];
			case 'B': [[fallthrough]];
			case 'c': [[fallthrough]];
			case 'd': [[fallthrough]];
			case 'e': [[fallthrough]];
			case 'E': [[fallthrough]];
			case 'f': [[fallthrough]];
			case 'F': [[fallthrough]];
			case 'g': [[fallthrough]];
			case 'G': [[fallthrough]];
			case 'o': [[fallthrough]];
			case 'p': [[fallthrough]];
			case 's': [[fallthrough]];
			case 'x': [[fallthrough]];
			case 'X': OnValidTypeSpec(argType, ch); return;
			default: OnInvalidTypeSpec(argType); return;
		}
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteSimpleString(T&& container) {
	const auto& storage { argStorage.isCustomFormatter ? customStorage : argStorage };
	std::string_view sv { storage.string_state(specValues.argPosition) };
	WriteToContainer(sv, sv.size(), std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteSimpleCString(T&& container) {
	const auto& storage { argStorage.isCustomFormatter ? customStorage : argStorage };
	std::string_view sv { storage.c_string_state(specValues.argPosition) };
	WriteToContainer(sv, sv.size(), std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteSimpleStringView(T&& container) {
	const auto& storage { argStorage.isCustomFormatter ? customStorage : argStorage };
	std::string_view sv { storage.string_view_state(specValues.argPosition) };
	WriteToContainer(sv, sv.size(), std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteSimpleInt(T&& container) {
	auto data { buffer.data() };
	auto size { buffer.size() };
	std::memset(data, 0, size);
	const auto& storage { argStorage.isCustomFormatter ? customStorage : argStorage };
	WriteToContainer(buffer, std::to_chars(data, data + size, storage.int_state(specValues.argPosition)).ptr - data, std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteSimpleUInt(T&& container) {
	auto data { buffer.data() };
	auto size { buffer.size() };
	std::memset(data, 0, size);
	const auto& storage { argStorage.isCustomFormatter ? customStorage : argStorage };
	WriteToContainer(buffer, std::to_chars(data, data + size, storage.uint_state(specValues.argPosition)).ptr - data, std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteSimpleLongLong(T&& container) {
	auto data { buffer.data() };
	auto size { buffer.size() };
	std::memset(data, 0, size);
	const auto& storage { argStorage.isCustomFormatter ? customStorage : argStorage };
	WriteToContainer(buffer, std::to_chars(data, data + size, storage.long_long_state(specValues.argPosition)).ptr - data, std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteSimpleULongLong(T&& container) {
	auto data { buffer.data() };
	auto size { buffer.size() };
	std::memset(data, 0, size);
	const auto& storage { argStorage.isCustomFormatter ? customStorage : argStorage };
	WriteToContainer(buffer, std::to_chars(data, data + size, storage.u_long_long_state(specValues.argPosition)).ptr - data, std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteSimpleBool(T&& container) {
	const auto& storage { argStorage.isCustomFormatter ? customStorage : argStorage };
	std::string_view sv { storage.bool_state(specValues.argPosition) ? "true" : "false" };
	WriteToContainer(sv, sv.size(), std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteSimpleFloat(T&& container) {
	auto data { buffer.data() };
	auto size { buffer.size() };
	std::memset(data, 0, size);
	const auto& storage { argStorage.isCustomFormatter ? customStorage : argStorage };
	WriteToContainer(buffer, std::to_chars(data, data + size, storage.float_state(specValues.argPosition)).ptr - data, std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteSimpleDouble(T&& container) {
	auto data { buffer.data() };
	auto size { buffer.size() };
	std::memset(data, 0, size);
	const auto& storage { argStorage.isCustomFormatter ? customStorage : argStorage };
	WriteToContainer(buffer, std::to_chars(data, data + size, storage.double_state(specValues.argPosition)).ptr - data, std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteSimpleLongDouble(T&& container) {
	auto data { buffer.data() };
	auto size { buffer.size() };
	std::memset(data, 0, size);
	const auto& storage { argStorage.isCustomFormatter ? customStorage : argStorage };
	WriteToContainer(buffer, std::to_chars(data, data + size, storage.long_double_state(specValues.argPosition)).ptr - data, std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteSimpleConstVoidPtr(T&& container) {
	auto data { buffer.data() };
	auto size { buffer.size() };
	std::memset(data, 0, size);
	buffer[ 0 ] = '0';
	buffer[ 1 ] = 'x';
	const auto& storage { argStorage.isCustomFormatter ? customStorage : argStorage };
	WriteToContainer(buffer,
	                 std::to_chars(data + 2, data + buffer.size() - 2, reinterpret_cast<size_t>(storage.const_void_ptr_state(specValues.argPosition)), 16).ptr - data,
	                 std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteSimpleVoidPtr(T&& container) {
	auto data { buffer.data() };
	auto size { buffer.size() };
	std::memset(data, 0, size);
	buffer[ 0 ] = '0';
	buffer[ 1 ] = 'x';
	const auto& storage { argStorage.isCustomFormatter ? customStorage : argStorage };
	WriteToContainer(buffer, std::to_chars(data + 2, data + buffer.size() - 2, reinterpret_cast<size_t>(storage.void_ptr_state(specValues.argPosition)), 16).ptr - data,
	                 std::forward<T>(container));
}

template<typename T>
requires std::is_integral_v<std::remove_cvref_t<T>>
constexpr void formatter::arg_formatter::ArgFormatter::TwoDigitToBuff(T&& val) {
	buffer[ valueSize ] = val > 9 ? static_cast<char>((val / 10) + NumericalAsciiOffset) : '0';
	++valueSize;
	buffer[ valueSize ] = static_cast<char>((val % 10) + NumericalAsciiOffset);
	++valueSize;
}

inline constexpr void formatter::arg_formatter::ArgFormatter::Format24HourTime(const int& hour, const int& min, const int& sec, int precision) {
	Format24HM(hour, min);
	buffer[ valueSize ] = ':';
	++valueSize;
	TwoDigitToBuff(sec);
	if( precision != 0 ) FormatSubseconds(precision);
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::Write24HourTime(T&& container, const int& hour, const int& min, const int& sec) {
	Format24HourTime(hour, min, sec);
	WriteBufferToContainer(std::forward<T>(container));
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatShortMonth(const int& mon) {
	auto month { short_months[ mon ] };
	int pos { 0 };
	for( ;; ) {
			buffer[ valueSize ] = month[ pos ];
			++valueSize;
			if( ++pos >= 3 ) return;
		}
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteShortMonth(T&& container, const int& mon) {
	FormatShortMonth(mon);
	WriteBufferToContainer(std::forward<T>(container));
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatShortWeekday(const int& wkday) {
	auto wkDay { short_weekdays[ wkday ] };
	int pos { 0 };
	for( ;; ) {
			buffer[ valueSize ] = wkDay[ pos ];
			++valueSize;
			if( ++pos >= 3 ) return;
		}
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteShortWeekday(T&& container, const int& wkday) {
	FormatShortWeekday(wkday);
	WriteBufferToContainer(std::forward<T>(container));
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatTimeDate(const std::tm& time) {
	FormatShortWeekday(time.tm_wday);
	buffer[ valueSize ] = ' ';
	++valueSize;
	FormatShortMonth(time.tm_mon);
	buffer[ valueSize ] = ' ';
	++valueSize;
	TwoDigitToBuff(time.tm_mday);
	buffer[ valueSize ] = ' ';
	++valueSize;
	Format24HourTime(time.tm_hour, time.tm_min, time.tm_sec);
	buffer[ valueSize ] = ' ';
	++valueSize;
	FormatLongYear(time.tm_year);
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteTimeDate(T&& container, const std::tm& time) {
	FormatTimeDate(time);
	WriteBufferToContainer(std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteShortYear(T&& container, const int& year) {
	FormatShortYear(year);
	WriteBufferToContainer(std::forward<T>(container));
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatShortYear(const int& yr) {
	auto year { yr % 100 };
	buffer[ valueSize ] = static_cast<char>((year / 10) + NumericalAsciiOffset);
	++valueSize;
	buffer[ valueSize ] = static_cast<char>((year % 10) + NumericalAsciiOffset);
	++valueSize;
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WritePaddedDay(T&& container, const int& day) {
	TwoDigitToBuff(day);
	WriteBufferToContainer(std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteSpacePaddedDay(T&& container, const int& day) {
	FormatSpacePaddedDay(day);
	WriteBufferToContainer(std::forward<T>(container));
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatSpacePaddedDay(const int& day) {
	buffer[ valueSize ] = day > 9 ? static_cast<char>((day / 10) + NumericalAsciiOffset) : ' ';
	++valueSize;
	buffer[ valueSize ] = static_cast<char>((day % 10) + NumericalAsciiOffset);
	++valueSize;
}

template<typename T>
constexpr void formatter::arg_formatter::ArgFormatter::WriteShortIsoWeekYear(T&& container, const int& year, const int& yrday, const int& wkday) {
	FormatShortIsoWeekYear(year, yrday, wkday);
	WriteBufferToContainer(std::forward<T>(container));
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatShortIsoWeekYear(const int& yr, const int& yrday, const int& wkday) {
	auto year { yr + 1900 };
	auto w { (10 + yrday - wkday) / 7 };
	if( w < 1 ) return FormatShortYear(year - 1901);    // decrement year
	auto prevYear { year - 1 };
	int weeks { 52 + ((year / 4 - year / 100 - year / 400 == 4) || ((prevYear / 4 - prevYear / 100 - prevYear / 400) == 3) ? 1 : 0) };
	if( w > weeks ) return FormatShortYear(year - 1899);    // increment year
	return FormatShortYear(year - 1900);                    // Use current year
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteDayOfYear(T&& container, const int& day) {
	FormatDayOfYear(day);
	WriteBufferToContainer(std::forward<T>(container));
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatDayOfYear(const int& d) {
	auto day { d + 1 };    // increment due to the inclusion of 0 -> day  0 is day 1 of year
	buffer[ valueSize ] = day > 99 ? static_cast<char>((day / 100) + NumericalAsciiOffset) : '0';
	++valueSize;
	buffer[ valueSize ] = day > 9 ? static_cast<char>(day > 99 ? ((day / 10) % 10) + NumericalAsciiOffset : ((day / 10) + NumericalAsciiOffset)) : '0';
	++valueSize;
	buffer[ valueSize ] = static_cast<char>((day % 10) + NumericalAsciiOffset);
	++valueSize;
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WritePaddedMonth(T&& container, const int& month) {
	TwoDigitToBuff(month + 1);
	WriteBufferToContainer(std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteLiteral(T&& container, unsigned char lit) {
	if constexpr( std::is_same_v<formatter::internal_helper::af_typedefs::type<T>, std::string> ) {
			container += lit;
	} else if constexpr( std::is_same_v<formatter::internal_helper::af_typedefs::type<T>, std::vector<typename formatter::internal_helper::af_typedefs::type<T>::value_type>> )
		{
			container.insert(container.end(), &lit, &lit + 1);
	} else {
			std::copy(&lit, &lit + 1, std::back_inserter(std::forward<T>(container)));
		}
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatLiteral(const unsigned char& lit) {
	buffer[ valueSize ] = lit;
	++valueSize;
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteAMPM(T&& container, const int& hour) {
	FormatAMPM(hour);
	WriteBufferToContainer(std::forward<T>(container));
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatAMPM(const int& hour) {
	buffer[ valueSize ] = hour >= 12 ? 'P' : 'A';
	++valueSize;
	buffer[ valueSize ] = 'M';
	++valueSize;
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::Write12HourTime(T&& container, const int& hour, const int& min, const int& sec) {
	Format24HourTime(hour > 12 ? hour - 12 : hour, min, sec);
	WriteBufferToContainer(std::forward<T>(container));
}

inline constexpr void formatter::arg_formatter::ArgFormatter::Format12HourTime(const int& hour, const int& min, const int& sec, int precision) {
	Format24HourTime(hour > 12 ? hour - 12 : hour, min, sec, precision);
	FormatAMPM(hour);
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteWeekdayDec(T&& container, const int& wkday) {
	FormatWeekdayDec(wkday);
	WriteBufferToContainer(std::forward<T>(container));
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatWeekdayDec(const int& wkday) {
	buffer[ valueSize ] = static_cast<char>(wkday + NumericalAsciiOffset);
	++valueSize;
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteMMDDYY(T&& container, const int& month, const int& day, const int& year) {
	FormatMMDDYY(month, day, year);
	WriteBufferToContainer(std::forward<T>(container));
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatMMDDYY(const int& month, const int& day, const int& year) {
	FormatShortMonth(month + 1);
	buffer[ valueSize ] = '/';
	++valueSize;
	TwoDigitToBuff(day);
	buffer[ valueSize ] = '/';
	++valueSize;
	FormatShortYear(year % 100);
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteIsoWeekDec(T&& container, const int& wkday) {
	FormatIsoWeekDec(wkday);
	WriteBufferToContainer(std::forward<T>(container));
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatIsoWeekDec(const int& wkday) {
	buffer[ valueSize ] = static_cast<char>((wkday != 0 ? wkday : 7) + NumericalAsciiOffset);
	++valueSize;
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteUtcOffset(T&& container) {
	FormatUtcOffset();
	WriteBufferToContainer(std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteLongWeekday(T&& container, const int& wkday) {
	FormatLongWeekday(wkday);
	WriteBufferToContainer(std::forward<T>(container));
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatLongWeekday(const int& wkday) {
	std::string_view weekday { long_weekdays[ wkday ] };
	int pos { 0 };
	auto size { weekday.size() };
	for( ;; ) {
			buffer[ valueSize ] = weekday[ pos ];
			++valueSize;
			if( ++pos >= size ) return;
		}
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteLongMonth(T&& container, const int& mon) {
	FormatLongMonth(mon);
	WriteBufferToContainer(std::forward<T>(container));
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatLongMonth(const int& mon) {
	std::string_view month { long_months[ mon ] };
	int pos { 0 };
	auto size { month.size() };
	for( ;; ) {
			buffer[ valueSize ] = month[ pos ];
			++valueSize;
			if( ++pos >= size ) return;
		}
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteYYYYMMDD(T&& container, const int& year, const int& mon, const int& day) {
	FormatYYYYMMDD(year, mon, day);
	WriteBufferToContainer(std::forward<T>(container));
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatYYYYMMDD(const int& year, const int& mon, const int& day) {
	FormatLongYear(year + 1900);
	buffer[ valueSize ] = '-';
	++valueSize;
	FormatShortMonth(mon + 1);
	buffer[ valueSize ] = '-';
	++valueSize;
	TwoDigitToBuff(day);
}
template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteLongYear(T&& container, int year) {
	FormatLongYear(year);
	WriteBufferToContainer(std::forward<T>(container));
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatLongYear(const int& yr) {
	auto year { yr + 1900 };
	buffer[ valueSize ] = static_cast<char>(year / 1000 + NumericalAsciiOffset);
	++valueSize;
	buffer[ valueSize ] = static_cast<char>((year / 100) % 10 + NumericalAsciiOffset);
	++valueSize;
	buffer[ valueSize ] = static_cast<char>((year % 100) / 10 + NumericalAsciiOffset);
	++valueSize;
	buffer[ valueSize ] = static_cast<char>((year % 100) % 10 + NumericalAsciiOffset);
	++valueSize;
}

template<typename T>
constexpr void formatter::arg_formatter::ArgFormatter::WriteLongIsoWeekYear(T&& container, const int& year, const int& yrday, const int& wkday) {
	FormatLongIsoWeekYear(year, yrday, wkday);
	WriteBufferToContainer(std::forward<T>(container));
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatLongIsoWeekYear(const int& yr, const int& yrday, const int& wkday) {
	auto year { yr + 1900 };
	auto w { (10 + yrday - wkday) / 7 };
	if( w < 1 ) return FormatLongYear(year - 1901);    // decrement year
	auto prevYear { year - 1 };
	int weeks { 52 + ((year / 4 - year / 100 - year / 400 == 4) || ((prevYear / 4 - prevYear / 100 - prevYear / 400) == 3) ? 1 : 0) };
	if( w > weeks ) return FormatLongYear(year - 1899);    // increment year
	return FormatLongYear(year - 1900);                    // Use current year
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteTruncatedYear(T&& container, const int& year) {
	FormatTruncatedYear(year);
	WriteBufferToContainer(std::forward<T>(container));
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatTruncatedYear(const int& yr) {
	auto year { (yr + 1900) / 100 };
	buffer[ valueSize ] = static_cast<char>((year / 10) + NumericalAsciiOffset);
	++valueSize;
	buffer[ valueSize ] = static_cast<char>((year % 10) + NumericalAsciiOffset);
	++valueSize;
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::Write24Hour(T&& container, const int& hour) {
	TwoDigitToBuff(hour);
	WriteBufferToContainer(std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::Write12Hour(T&& container, const int& hour) {
	TwoDigitToBuff(hour > 12 ? hour - 12 : hour);
	WriteBufferToContainer(std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteMinute(T&& container, const int& min) {
	TwoDigitToBuff(min);
	WriteBufferToContainer(std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::Write24HM(T&& container, const int& hour, const int& min) {
	Format24HM(hour, min);
	WriteBufferToContainer(std::forward<T>(container));
}

inline constexpr void formatter::arg_formatter::ArgFormatter::Format24HM(const int& hour, const int& min) {
	TwoDigitToBuff(hour);
	buffer[ valueSize ] = ':';
	++valueSize;
	TwoDigitToBuff(min);
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteSecond(T&& container, const int& sec) {
	TwoDigitToBuff(sec);
	WriteBufferToContainer(std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteTime(T&& container, const int& hour, const int& min, const int& sec) {
	Format24HourTime(hour, min, sec);
	(hour, min, sec);
	WriteBufferToContainer(std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteTZName(T&& container) {
	FormatTZName();
	WriteBufferToContainer(std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteWeek(T&& container, const int& yrday, const int& wkday) {
	TwoDigitToBuff((10 + yrday - wkday) / 7);
	WriteBufferToContainer(std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteIsoWeek(T&& container, const int& yrday, const int& wkday) {
	TwoDigitToBuff((yrday + 7 - (wkday == 0 ? 6 : wkday - 1)) / 7);
	WriteBufferToContainer(std::forward<T>(container));
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteIsoWeekNumber(T&& container, const int& year, const int& yrday, const int& wkday) {
	FormatIsoWeekNumber(year, yrday, wkday);
	WriteBufferToContainer(std::forward<T>(container));
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatIsoWeekNumber(const int& yr, const int& yrday, const int& wkday) {
	auto w = (10 + yrday - wkday) / 7;
	auto year { yr };
	if( w < 1 ) {
			--year;
			auto prevYear = year - 1;
			int weeks { 52 + ((year / 4 - year / 100 - year / 400 == 4) || ((prevYear / 4 - prevYear / 100 - prevYear / 400) == 3) ? 1 : 0) };
			TwoDigitToBuff(weeks);
			return;
	}
	auto prevYear = year - 1;
	int weeks { 52 + ((year / 4 - year / 100 - year / 400 == 4) || ((prevYear / 4 - prevYear / 100 - prevYear / 400) == 3) ? 1 : 0) };
	if( w > weeks ) {
			buffer[ valueSize++ ] = 1;
			return;
	}
	TwoDigitToBuff(w);
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteSimpleCTime(T&& container) {
	const auto& tm { argStorage.isCustomFormatter ? customStorage.c_time_state(specValues.argPosition) : argStorage.c_time_state(specValues.argPosition) };
	switch( timeSpec.timeSpecContainer[ 0 ] ) {
			case 'a': WriteShortWeekday(std::forward<T>(container), tm.tm_wday); return;
			case 'h': [[fallthrough]];
			case 'b': WriteShortMonth(std::forward<T>(container), tm.tm_mon); return;
			case 'c': WriteTimeDate(std::forward<T>(container), tm); return;
			case 'd': WritePaddedDay(std::forward<T>(container), tm.tm_mday); return;
			case 'e': WriteSpacePaddedDay(std::forward<T>(container), tm.tm_mday); return;
			case 'g': WriteShortIsoWeekYear(std::forward<T>(container), tm.tm_year, tm.tm_yday, tm.tm_wday); return;
			case 'j': WriteDayOfYear(std::forward<T>(container), tm.tm_yday); return;
			case 'k': WriteWkday_DDMMMYY_Time(std::forward<T>(container), tm); return;
			case 'm': WritePaddedMonth(std::forward<T>(container), tm.tm_mon); return;
			case 'p': WriteAMPM(std::forward<T>(container), tm.tm_hour); return;
			case 'r': Write12HourTime(std::forward<T>(container), tm.tm_hour, tm.tm_min, tm.tm_sec); return;
			case 'u': WriteIsoWeekDec(std::forward<T>(container), tm.tm_wday); return;
			case 'w': WriteWeekdayDec(std::forward<T>(container), tm.tm_wday); return;
			case 'D': [[fallthrough]];
			case 'x': WriteMMDDYY(std::forward<T>(container), tm.tm_mon, tm.tm_mday, tm.tm_year); return;
			case 'y': WriteShortYear(std::forward<T>(container), tm.tm_year); return;
			case 'z': WriteUtcOffset(std::forward<T>(container)); return;
			case 'A': WriteLongWeekday(std::forward<T>(container), tm.tm_wday); return;
			case 'B': WriteLongMonth(std::forward<T>(container), tm.tm_mon); return;
			case 'C': WriteTruncatedYear(std::forward<T>(container), tm.tm_year); return;
			case 'F': WriteYYYYMMDD(std::forward<T>(container), tm.tm_year, tm.tm_mon, tm.tm_mday); return;
			case 'G': WriteLongIsoWeekYear(std::forward<T>(container), tm.tm_year, tm.tm_yday, tm.tm_wday); return;
			case 'H': Write24Hour(std::forward<T>(container), tm.tm_hour); return;
			case 'I': Write12Hour(std::forward<T>(container), tm.tm_hour); return;
			case 'M': WriteMinute(std::forward<T>(container), tm.tm_min); return;
			case 'R': Write24HM(std::forward<T>(container), tm.tm_hour, tm.tm_min); return;
			case 'S': WriteSecond(std::forward<T>(container), tm.tm_sec); return;
			case 'T': Write24HourTime(std::forward<T>(container), tm.tm_hour, tm.tm_min, tm.tm_sec); return;
			case 'U': WriteWeek(std::forward<T>(container), tm.tm_yday, tm.tm_wday); return;
			case 'V': WriteIsoWeekNumber(std::forward<T>(container), tm.tm_year, tm.tm_yday, tm.tm_wday); return;
			case 'W': WriteIsoWeek(std::forward<T>(container), tm.tm_yday, tm.tm_wday); return;
			case 'X': WriteTime(std::forward<T>(container), tm.tm_hour, tm.tm_min, tm.tm_sec); return;
			case 'Y': WriteLongYear(std::forward<T>(container), tm.tm_year); return;
			case 'Z': WriteTZName(std::forward<T>(container)); return;
			case 'n': WriteLiteral(std::forward<T>(container), '\n'); return;
			case 't': WriteLiteral(std::forward<T>(container), '\t'); return;
			case '%': WriteLiteral(std::forward<T>(container), '%'); return;
			default: WriteLiteral(std::forward<T>(container), timeSpec.timeSpecContainer[ 0 ]); return;
		}
}

template<typename T> inline constexpr void formatter::arg_formatter::ArgFormatter::WriteWkday_DDMMMYY_Time(T&& container, const std::tm& time) {
	FormatWkday_DDMMMYY_Time(time);
	WriteBufferToContainer(std::forward<T>(container));
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatWkday_DDMMMYY_Time(const std::tm& time, int precision) {
	FormatShortWeekday(time.tm_wday);
	buffer[ valueSize ] = ' ';
	++valueSize;
	FormatSpacePaddedDay(time.tm_mday);
	FormatShortMonth(time.tm_mon);
	FormatShortYear(time.tm_year);
	buffer[ valueSize ] = ' ';
	++valueSize;
	Format24HourTime(time.tm_hour, time.tm_min, time.tm_sec, precision);
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatCTime(const std::tm& time, const int& precision, int startPos, int endPos) {
	--startPos;
	for( ;; ) {
			if( ++startPos >= endPos ) return;
			switch( timeSpec.timeSpecContainer[ startPos ] ) {
					case 'a': FormatShortWeekday(time.tm_wday); continue;
					case 'h': [[fallthrough]];
					case 'b': FormatShortMonth(time.tm_mon); continue;
					case 'c': FormatTimeDate(time); continue;
					case 'd': TwoDigitToBuff(time.tm_mday); continue;
					case 'e': FormatSpacePaddedDay(time.tm_mday); continue;
					case 'g': FormatShortIsoWeekYear(time.tm_year, time.tm_yday, time.tm_wday); continue;
					case 'j': FormatDayOfYear(time.tm_yday); continue;
					case 'k': FormatWkday_DDMMMYY_Time(time, precision); continue;
					case 'm': TwoDigitToBuff(time.tm_mon + 1); continue;
					case 'p': FormatAMPM(time.tm_hour); continue;
					case 'r': Format12HourTime(time.tm_hour, time.tm_min, time.tm_sec, precision); continue;
					case 'w': FormatWeekdayDec(time.tm_wday); continue;
					case 'u': FormatIsoWeekDec(time.tm_wday); continue;
					case 'D': [[fallthrough]];
					case 'x': FormatMMDDYY(time.tm_mon, time.tm_mday, time.tm_year); continue;
					case 'y': FormatShortYear(time.tm_year); continue;
					case 'z': FormatUtcOffset(); continue;
					case 'A': FormatLongWeekday(time.tm_wday); continue;
					case 'B': FormatLongMonth(time.tm_mon); continue;
					case 'C': FormatTruncatedYear(time.tm_year); continue;
					case 'F': FormatYYYYMMDD(time.tm_year, time.tm_mon, time.tm_mday); continue;
					case 'G': FormatLongIsoWeekYear(time.tm_year, time.tm_yday, time.tm_wday); continue;
					case 'H': TwoDigitToBuff(time.tm_hour); continue;
					case 'I': TwoDigitToBuff(time.tm_hour > 12 ? time.tm_hour - 12 : time.tm_hour); continue;
					case 'M': TwoDigitToBuff(time.tm_min); continue;
					case 'R': Format24HM(time.tm_hour, time.tm_min); continue;
					case 'S': TwoDigitToBuff(time.tm_sec); continue;
					case 'T': Format24HourTime(time.tm_hour, time.tm_min, time.tm_sec, precision); continue;
					case 'U': TwoDigitToBuff((10 + time.tm_yday - time.tm_wday) / 7); continue;
					case 'V': FormatIsoWeekNumber(time.tm_year, time.tm_yday, time.tm_wday); continue;
					case 'W': TwoDigitToBuff((time.tm_yday + 7 - (time.tm_wday == 0 ? 6 : time.tm_wday - 1)) / 7); continue;
					case 'X': Format24HourTime(time.tm_hour, time.tm_min, time.tm_sec, precision); continue;
					case 'Y': FormatLongYear(time.tm_year); continue;
					case 'Z': FormatTZName(); continue;
					case 'n': FormatLiteral('\n'); continue;
					case 't': FormatLiteral('\t'); continue;
					case '%': FormatLiteral('%'); continue;
					default: FormatLiteral(timeSpec.timeSpecContainer[ startPos ]); continue;
				}
		}
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteSimpleValue(T&& container, const msg_details::SpecType& argType) {
	using enum msg_details::SpecType;

	switch( argType ) {
			case StringType: WriteSimpleString(std::forward<T>(container)); return;
			case CharPointerType: WriteSimpleCString(std::forward<T>(container)); return;
			case StringViewType: WriteSimpleStringView(std::forward<T>(container)); return;
			case IntType: WriteSimpleInt(std::forward<T>(container)); return;
			case U_IntType: WriteSimpleUInt(std::forward<T>(container)); return;
			case LongLongType: WriteSimpleLongLong(std::forward<T>(container)); return;
			case U_LongLongType: WriteSimpleULongLong(std::forward<T>(container)); return;
			case BoolType: WriteSimpleBool(std::forward<T>(container)); return;
			case CharType:
				container.insert(container.end(),
				                 (argStorage.isCustomFormatter ? customStorage.char_state(specValues.argPosition) : argStorage.char_state(specValues.argPosition)));
				return;
			case FloatType: WriteSimpleFloat(std::forward<T>(container)); return;
			case DoubleType: WriteSimpleDouble(std::forward<T>(container)); return;
			case LongDoubleType: WriteSimpleLongDouble(std::forward<T>(container)); return;
			case ConstVoidPtrType: WriteSimpleConstVoidPtr(std::forward<T>(container)); return;
			case VoidPtrType: WriteSimpleVoidPtr(std::forward<T>(container)); return;
			default: return;
		}
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatCharType(const char& value) {
	specValues.typeSpec != '\0' && specValues.typeSpec != 'c' ? FormatIntegerType(static_cast<int>(value)) : WriteChar(value);
}

inline constexpr void formatter::arg_formatter::ArgFormatter::WriteBool(const bool& value) {
	using namespace std::string_view_literals;
	auto sv { value ? "true"sv : "false"sv };
	std::copy(sv.data(), sv.data() + sv.size(), buffer.begin());
	valueSize += sv.size();
}
inline constexpr void formatter::arg_formatter::ArgFormatter::FormatBoolType(const bool& value) {
	specValues.typeSpec != '\0' && specValues.typeSpec != 's' ? FormatIntegerType(static_cast<unsigned char>(value)) : WriteBool(value);
}

template<typename T>
requires std::is_pointer_v<std::remove_cvref_t<T>>
constexpr void formatter::arg_formatter::ArgFormatter::FormatPointerType(T&& value, const msg_details::SpecType& type) {
	using enum msg_details::SpecType;
	constexpr std::string_view sv { "0x" };
	switch( type ) {
			case ConstVoidPtrType:
				{
					auto data { buffer.data() };
					std::memcpy(data, sv.data(), 2);
					valueSize = std::to_chars(data + 2, data + buffer.size(), reinterpret_cast<size_t>(std::forward<T>(value)), 16).ptr - data;
					return;
				}
			case VoidPtrType:
				{
					auto data { buffer.data() };
					std::memcpy(data, sv.data(), 2);
					valueSize = std::to_chars(data + 2, data + buffer.size(), reinterpret_cast<size_t>(std::forward<T>(value)), 16).ptr - data;
					return;
				}
			default: return;
		}
}

inline constexpr void formatter::arg_formatter::ArgFormatter::FormatArgument(const int& precision, const msg_details::SpecType& type) {
	using enum msg_details::SpecType;
	auto& storage { argStorage.isCustomFormatter ? customStorage : argStorage };
	switch( type ) {
			case IntType: FormatIntegerType(storage.int_state(specValues.argPosition)); return;
			case U_IntType: FormatIntegerType(storage.uint_state(specValues.argPosition)); return;
			case LongLongType: FormatIntegerType(storage.long_long_state(specValues.argPosition)); return;
			case U_LongLongType: FormatIntegerType(storage.u_long_long_state(specValues.argPosition)); return;
			case BoolType: FormatBoolType(storage.bool_state(specValues.argPosition)); return;
			case CharType: FormatCharType(storage.char_state(specValues.argPosition)); return;
			case FloatType: FormatFloatType(storage.float_state(specValues.argPosition), precision); return;
			case DoubleType: FormatFloatType(storage.double_state(specValues.argPosition), precision); return;
			case LongDoubleType: FormatFloatType(storage.long_double_state(specValues.argPosition), precision); return;
			case ConstVoidPtrType: FormatPointerType(storage.const_void_ptr_state(specValues.argPosition), type); return;
			case VoidPtrType: FormatPointerType(storage.void_ptr_state(specValues.argPosition), type); return;
			default: return;
		}
}

inline constexpr void formatter::arg_formatter::ArgFormatter::BufferToUpper(char* begin, const char* end) {
	// Bypassing a check on every char and only matching these cases is slightly faster and uses considerably less CPU cycles.
	// Since this is being used explicitly to convert hex values and the case 'p' (for floating point cases), these are the only
	// values that are of any concern to check here.
	for( ;; ) {
			if( begin == end ) return;
			switch( *begin ) {
					case 'a': [[fallthrough]];
					case 'b': [[fallthrough]];
					case 'c': [[fallthrough]];
					case 'd': [[fallthrough]];
					case 'e': [[fallthrough]];
					case 'f': [[fallthrough]];
					case 'p': [[fallthrough]];
					case 'x': *begin++ -= 32; continue;
					default: ++begin; continue;
				}
		}
}

inline constexpr void formatter::arg_formatter::ArgFormatter::SetFloatingFormat(std::chars_format& format, int& precision, bool& isUpper) {
	switch( specValues.typeSpec ) {
			case '\0':
				// default behaviors
				if( specValues.hasAlt && precision == 0 ) {
						format = std::chars_format::scientific;
				} else if( !specValues.localize && specValues.alignmentPadding == 0 && precision == 0 && specValues.signType == Sign::Empty ) {
						format = std::chars_format::fixed;
				} else {
						format = std::chars_format::general;
					}
				break;
			case 'a':
				precision = precision > 0 ? precision : 0;
				format    = std::chars_format::hex;
				break;
			case 'A':
				isUpper   = true;
				precision = precision > 0 ? precision : 0;
				format    = std::chars_format::hex;
				break;
			case 'e':
				format    = std::chars_format::scientific;
				precision = precision > 0 ? precision : 6;
				break;
			case 'E':
				isUpper   = true;
				format    = std::chars_format::scientific;
				precision = precision > 0 ? precision : 6;
				break;
			case 'f':
				format    = std::chars_format::fixed;
				precision = precision > 0 ? precision : 6;
				break;
			case 'F':
				isUpper   = true;
				format    = std::chars_format::fixed;
				precision = precision > 0 ? precision : 6;
				break;
			case 'g':
				format    = std::chars_format::general;
				precision = precision > 0 ? precision : 6;
				break;
			case 'G':
				isUpper   = true;
				format    = std::chars_format::general;
				precision = precision > 0 ? precision : 6;
				break;
			default: break;
		}
}

template<typename T>
requires std::is_arithmetic_v<std::remove_cvref_t<T>>
constexpr void formatter::arg_formatter::ArgFormatter::WriteSign(T&& value, int& pos) {
	switch( specValues.signType == Sign::Space ? value < 0 ? Sign::Minus : Sign::Space : specValues.signType ) {
			case Sign::Space: buffer[ pos++ ] = ' '; return;
			case Sign::Plus: buffer[ pos++ ] = '+'; return;
			case Sign::Empty: [[fallthrough]];
			case Sign::Minus: return;
		}
}

inline constexpr void formatter::arg_formatter::ArgFormatter::WriteChar(const char& value) {
	buffer[ valueSize ] = value;
	++valueSize;
}

inline constexpr void formatter::arg_formatter::ArgFormatter::SetIntegralFormat(int& base, bool& isUpper) {
	// spec 'c' is handled in FormatArgument() By direct write to buffer
	switch( specValues.typeSpec ) {
			case '\0': base = 10; return;
			case 'b': base = 2; return;
			case 'B':
				base    = 2;
				isUpper = true;
				return;
			case 'o': base = 8; return;
			case 'x': base = 16; return;
			case 'X':
				base    = 16;
				isUpper = true;
				return;
			default: base = 10; return;
		}
}

template<typename T>
requires std::is_floating_point_v<std::remove_cvref_t<T>>
constexpr void formatter::arg_formatter::ArgFormatter::FormatFloatType(T&& value, int precision) {
	int pos { 0 };
	bool isUpper { false };
	auto data { buffer.data() };
	std::chars_format format {};
	!std::is_constant_evaluated() ? static_cast<void>(std::memset(data, 0, AF_ARG_BUFFER_SIZE)) : std::fill(buffer.begin(), buffer.end(), 0);
	if( specValues.signType != Sign::Empty ) WriteSign(std::forward<T>(value), pos);
	SetFloatingFormat(format, precision, isUpper);
	auto end { precision != 0 ? std::to_chars(data + pos, data + AF_ARG_BUFFER_SIZE, value, format, precision).ptr
		                      : std::to_chars(data + pos, data + AF_ARG_BUFFER_SIZE, value, format).ptr };
	valueSize = end - data;
	if( isUpper ) BufferToUpper(data, end);
}

template<typename T>
requires std::is_integral_v<std::remove_cvref_t<T>>
constexpr void formatter::arg_formatter::ArgFormatter::FormatIntegerType(T&& value) {
	int pos { 0 }, base { 10 };
	bool isUpper { false };
	auto data { buffer.data() };
	!std::is_constant_evaluated() ? static_cast<void>(std::memset(data, 0, AF_ARG_BUFFER_SIZE)) : std::fill(buffer.begin(), buffer.end(), 0);
	if( specValues.signType != Sign::Empty ) WriteSign(std::forward<T>(value), pos);
	if( specValues.preAltForm.size() != 0 ) {
			std::memcpy(data + pos, specValues.preAltForm.data(), specValues.preAltForm.size());
			pos += static_cast<int>(specValues.preAltForm.size());    // safe to downcast as it will only ever be positive and max val of 2
	}
	SetIntegralFormat(base, isUpper);
	auto end { std::to_chars(data + pos, data + AF_ARG_BUFFER_SIZE, value, base).ptr };
	valueSize = end - data;
	if( isUpper ) BufferToUpper(data, end);
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::FormatStringType(T&& container, std::string_view val, const int& precision) {
	int size { static_cast<int>(val.size()) };
	using CharType = typename formatter::internal_helper::af_typedefs::type<T>::value_type;
	if constexpr( std::is_same_v<CharType, char> ) {
			WriteToContainer(val, precision, std::forward<T>(container));
	} else if constexpr( std::is_same_v<CharType, char16_t> || (std::is_same_v<CharType, wchar_t> && sizeof(wchar_t) == 2) ) {
			std::u16string tmp;
			tmp.reserve(precision);
			utf_utils::U8ToU16(val, tmp);
			WriteToContainer(std::move(tmp), precision, std::forward<T>(container));
	} else if constexpr( std::is_same_v<CharType, char32_t> || (std::is_same_v<CharType, wchar_t> && sizeof(wchar_t) == 4) ) {
			std::u32string tmp;
			tmp.reserve(precision);
			utf_utils::U8ToU32(val, tmp);
			WriteToContainer(std::move(tmp), precision, std::forward<T>(container));
	} else {
			auto iter { std::back_inserter(std::forward<T>(container)) };
			std::copy(val.data(), val.data() + (precision != 0 ? precision > size ? size : precision : size), iter);
		}
}

template<typename T> constexpr void formatter::arg_formatter::ArgFormatter::WriteFormattedString(T&& container, const SpecType& type, const int& precision) {
	using enum msg_details::SpecType;
	auto& storage { argStorage.isCustomFormatter ? customStorage : argStorage };
	switch( type ) {
			case StringViewType: return FormatStringType(std::forward<T>(container), storage.string_view_state(specValues.argPosition), precision);
			case StringType: return FormatStringType(std::forward<T>(container), storage.string_state(specValues.argPosition), precision);
			case CharPointerType: return FormatStringType(std::forward<T>(container), storage.c_string_state(specValues.argPosition), precision);
			default: return;
		}
}

inline constexpr bool formatter::arg_formatter::ArgFormatter::IsCustomFmtProcActive() {
	return argStorage.isCustomFormatter;
}

inline constexpr void formatter::arg_formatter::ArgFormatter::EnableCustomFmtProc(bool enable) {
	argStorage.isCustomFormatter = enable;
}
