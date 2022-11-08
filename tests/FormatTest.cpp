#pragma once

#include "catch.hpp"

#include "../include/ArgFormatter/ArgFormatter.h"
#include <format>
#include <iostream>
using namespace formatter::arg_formatter;

// common testing variables
static int a                = 424'242'424;
static unsigned int b       = 4'242'424'242;
static long long c          = 424'242'424'242'424;
static unsigned long long d = 424'242'424'242'424'424;
static float e              = 424'242'424.424f;
static double f             = 424'242'424.42424242;
static long double g        = 424'424'242'424.42424242;
static std::string h { "This is a string arg" };
static const char* i { "This is a const char* arg" };
static std::string_view j { "This is a string_view arg" };
static bool k = true;
static char l = 'm';
const void* m = static_cast<const void*>(&a);
void* n       = static_cast<void*>(&b);

TEST_CASE("Base Auto Index Formatting") {
	ArgFormatter formatter;

	static constexpr std::string_view parseString { "{}" };
	REQUIRE(std::format(parseString, a) == formatter.format(parseString, a));
	REQUIRE(std::format(parseString, b) == formatter.format(parseString, b));
	REQUIRE(std::format(parseString, c) == formatter.format(parseString, c));
	REQUIRE(std::format(parseString, d) == formatter.format(parseString, d));
	REQUIRE(std::format(parseString, e) == formatter.format(parseString, e));
	REQUIRE(std::format(parseString, f) == formatter.format(parseString, f));
	REQUIRE(std::format(parseString, g) == formatter.format(parseString, g));
	REQUIRE(std::format(parseString, h) == formatter.format(parseString, h));
	REQUIRE(std::format(parseString, i) == formatter.format(parseString, i));
	REQUIRE(std::format(parseString, j) == formatter.format(parseString, j));
	REQUIRE(std::format(parseString, k) == formatter.format(parseString, k));
	REQUIRE(std::format(parseString, l) == formatter.format(parseString, l));
	REQUIRE(std::format(parseString, m) == formatter.format(parseString, m));
	REQUIRE(std::format(parseString, n) == formatter.format(parseString, n));

	REQUIRE(std::format("{} {} {} {} {} {} {} {} {} {} {} {} {} {}", a, b, c, d, e, f, g, h, i, j, k, l, m, n) ==
	        formatter.format("{} {} {} {} {} {} {} {} {} {} {} {} {} {}", a, b, c, d, e, f, g, h, i, j, k, l, m, n));
}

TEST_CASE("Base Manual Index Formatting") {
	ArgFormatter formatter;

	REQUIRE(std::format("{0}", a, b, c, d, e, f, g, h, i, j, k, l, m, n) == formatter.format("{0}", a, b, c, d, e, f, g, h, i, j, k, l, m, n));
	REQUIRE(std::format("{1}", a, b, c, d, e, f, g, h, i, j, k, l, m, n) == formatter.format("{1}", a, b, c, d, e, f, g, h, i, j, k, l, m, n));
	REQUIRE(std::format("{2}", a, b, c, d, e, f, g, h, i, j, k, l, m, n) == formatter.format("{2}", a, b, c, d, e, f, g, h, i, j, k, l, m, n));
	REQUIRE(std::format("{3}", a, b, c, d, e, f, g, h, i, j, k, l, m, n) == formatter.format("{3}", a, b, c, d, e, f, g, h, i, j, k, l, m, n));
	REQUIRE(std::format("{4}", a, b, c, d, e, f, g, h, i, j, k, l, m, n) == formatter.format("{4}", a, b, c, d, e, f, g, h, i, j, k, l, m, n));
	REQUIRE(std::format("{5}", a, b, c, d, e, f, g, h, i, j, k, l, m, n) == formatter.format("{5}", a, b, c, d, e, f, g, h, i, j, k, l, m, n));
	REQUIRE(std::format("{6}", a, b, c, d, e, f, g, h, i, j, k, l, m, n) == formatter.format("{6}", a, b, c, d, e, f, g, h, i, j, k, l, m, n));
	REQUIRE(std::format("{7}", a, b, c, d, e, f, g, h, i, j, k, l, m, n) == formatter.format("{7}", a, b, c, d, e, f, g, h, i, j, k, l, m, n));
	REQUIRE(std::format("{8}", a, b, c, d, e, f, g, h, i, j, k, l, m, n) == formatter.format("{8}", a, b, c, d, e, f, g, h, i, j, k, l, m, n));
	REQUIRE(std::format("{9}", a, b, c, d, e, f, g, h, i, j, k, l, m, n) == formatter.format("{9}", a, b, c, d, e, f, g, h, i, j, k, l, m, n));
	REQUIRE(std::format("{10}", a, b, c, d, e, f, g, h, i, j, k, l, m, n) == formatter.format("{10}", a, b, c, d, e, f, g, h, i, j, k, l, m, n));
	REQUIRE(std::format("{11}", a, b, c, d, e, f, g, h, i, j, k, l, m, n) == formatter.format("{11}", a, b, c, d, e, f, g, h, i, j, k, l, m, n));
	REQUIRE(std::format("{12}", a, b, c, d, e, f, g, h, i, j, k, l, m, n) == formatter.format("{12}", a, b, c, d, e, f, g, h, i, j, k, l, m, n));
	REQUIRE(std::format("{13}", a, b, c, d, e, f, g, h, i, j, k, l, m, n) == formatter.format("{13}", a, b, c, d, e, f, g, h, i, j, k, l, m, n));

	REQUIRE(std::format("{0} {1} {2} {3} {4} {5} {6} {7} {8} {9} {10} {11} {12} {13}", a, b, c, d, e, f, g, h, i, j, k, l, m, n) ==
	        formatter.format("{0} {1} {2} {3} {4} {5} {6} {7} {8} {9} {10} {11} {12} {13}", a, b, c, d, e, f, g, h, i, j, k, l, m, n));

	REQUIRE(std::format("{9} {3} {2} {10} {4} {12} {6} {5} {13} {11} {8} {7} {0} {1}", a, b, c, d, e, f, g, h, i, j, k, l, m, n) ==
	        formatter.format("{9} {3} {2} {10} {4} {12} {6} {5} {13} {11} {8} {7} {0} {1}", a, b, c, d, e, f, g, h, i, j, k, l, m, n));
}

TEST_CASE("Type Specifier Formatting") {
	ArgFormatter formatter;

	REQUIRE(std::format("{:b}", a) == formatter.format("{:b}", a));
	REQUIRE(std::format("{:B}", a) == formatter.format("{:B}", a));
	REQUIRE(std::format("{:d}", a) == formatter.format("{:d}", a));
	REQUIRE(std::format("{:o}", a) == formatter.format("{:o}", a));
	REQUIRE(std::format("{:x}", a) == formatter.format("{:x}", a));
	REQUIRE(std::format("{:X}", a) == formatter.format("{:X}", a));

	REQUIRE(std::format("{:b}", b) == formatter.format("{:b}", b));
	REQUIRE(std::format("{:B}", b) == formatter.format("{:B}", b));
	REQUIRE(std::format("{:d}", b) == formatter.format("{:d}", b));
	REQUIRE(std::format("{:o}", b) == formatter.format("{:o}", b));
	REQUIRE(std::format("{:x}", b) == formatter.format("{:x}", b));
	REQUIRE(std::format("{:X}", b) == formatter.format("{:X}", b));

	REQUIRE(std::format("{:b}", c) == formatter.format("{:b}", c));
	REQUIRE(std::format("{:B}", c) == formatter.format("{:B}", c));
	REQUIRE(std::format("{:d}", c) == formatter.format("{:d}", c));
	REQUIRE(std::format("{:o}", c) == formatter.format("{:o}", c));
	REQUIRE(std::format("{:x}", c) == formatter.format("{:x}", c));
	REQUIRE(std::format("{:X}", c) == formatter.format("{:X}", c));

	REQUIRE(std::format("{:b}", d) == formatter.format("{:b}", d));
	REQUIRE(std::format("{:B}", d) == formatter.format("{:B}", d));
	REQUIRE(std::format("{:d}", d) == formatter.format("{:d}", d));
	REQUIRE(std::format("{:o}", d) == formatter.format("{:o}", d));
	REQUIRE(std::format("{:x}", d) == formatter.format("{:x}", d));
	REQUIRE(std::format("{:X}", d) == formatter.format("{:X}", d));

	REQUIRE(std::format("{:a}", e) == formatter.format("{:a}", e));
	REQUIRE(std::format("{:A}", e) == formatter.format("{:A}", e));

	REQUIRE(std::format("{:a}", f) == formatter.format("{:a}", f));
	REQUIRE(std::format("{:A}", f) == formatter.format("{:A}", f));

	REQUIRE(std::format("{:a}", g) == formatter.format("{:a}", g));
	REQUIRE(std::format("{:A}", g) == formatter.format("{:A}", g));

	REQUIRE(std::format("{:s}", h) == formatter.format("{:s}", h));
	REQUIRE(std::format("{:s}", i) == formatter.format("{:s}", i));
	REQUIRE(std::format("{:s}", j) == formatter.format("{:s}", j));

	REQUIRE(std::format("{:B}", k) == formatter.format("{:B}", k));
	REQUIRE(std::format("{:o}", k) == formatter.format("{:o}", k));
	REQUIRE(std::format("{:x}", k) == formatter.format("{:x}", k));
	REQUIRE(std::format("{:X}", k) == formatter.format("{:X}", k));

	REQUIRE(std::format("{:B}", l) == formatter.format("{:B}", l));
	REQUIRE(std::format("{:c}", l) == formatter.format("{:c}", l));
	REQUIRE(std::format("{:d}", l) == formatter.format("{:d}", l));
	REQUIRE(std::format("{:o}", l) == formatter.format("{:o}", l));
	REQUIRE(std::format("{:x}", l) == formatter.format("{:x}", l));
	REQUIRE(std::format("{:X}", l) == formatter.format("{:X}", l));

	REQUIRE(std::format("{:p}", m) == formatter.format("{:p}", m));
	REQUIRE(std::format("{:p}", n) == formatter.format("{:p}", n));
}

TEST_CASE("Alternate Int Formatting") {
	ArgFormatter formatter;

	REQUIRE(std::format("{:#}", a) == formatter.format("{:#}", a));
	REQUIRE(std::format("{:#b}", a) == formatter.format("{:#b}", a));
	REQUIRE(std::format("{:#B}", a) == formatter.format("{:#B}", a));
	REQUIRE(std::format("{:#d}", a) == formatter.format("{:#d}", a));
	REQUIRE(std::format("{:#o}", a) == formatter.format("{:#o}", a));
	REQUIRE(std::format("{:#x}", a) == formatter.format("{:#x}", a));
	REQUIRE(std::format("{:#X}", a) == formatter.format("{:#X}", a));
}

TEST_CASE("Alternate U_Int Formatting") {
	ArgFormatter formatter;

	REQUIRE(std::format("{:#}", b) == formatter.format("{:#}", b));
	REQUIRE(std::format("{:#b}", b) == formatter.format("{:#b}", b));
	REQUIRE(std::format("{:#B}", b) == formatter.format("{:#B}", b));
	REQUIRE(std::format("{:#d}", b) == formatter.format("{:#d}", b));
	REQUIRE(std::format("{:#o}", b) == formatter.format("{:#o}", b));
	REQUIRE(std::format("{:#x}", b) == formatter.format("{:#x}", b));
	REQUIRE(std::format("{:#X}", b) == formatter.format("{:#X}", b));
}

TEST_CASE("Alternate Long Long Formatting") {
	ArgFormatter formatter;

	REQUIRE(std::format("{:#}", c) == formatter.format("{:#}", c));
	REQUIRE(std::format("{:#b}", c) == formatter.format("{:#b}", c));
	REQUIRE(std::format("{:#B}", c) == formatter.format("{:#B}", c));
	REQUIRE(std::format("{:#d}", c) == formatter.format("{:#d}", c));
	REQUIRE(std::format("{:#o}", c) == formatter.format("{:#o}", c));
	REQUIRE(std::format("{:#x}", c) == formatter.format("{:#x}", c));
	REQUIRE(std::format("{:#X}", c) == formatter.format("{:#X}", c));
}

TEST_CASE("Alternate Unsigned Long Long Formatting") {
	ArgFormatter formatter;

	REQUIRE(std::format("{:#}", d) == formatter.format("{:#}", d));
	REQUIRE(std::format("{:#b}", d) == formatter.format("{:#b}", d));
	REQUIRE(std::format("{:#B}", d) == formatter.format("{:#B}", d));
	REQUIRE(std::format("{:#d}", d) == formatter.format("{:#d}", d));
	REQUIRE(std::format("{:#o}", d) == formatter.format("{:#o}", d));
	REQUIRE(std::format("{:#x}", d) == formatter.format("{:#x}", d));
	REQUIRE(std::format("{:#X}", d) == formatter.format("{:#X}", d));
}

TEST_CASE("Alternate Float Formatting") {
	ArgFormatter formatter;

	REQUIRE(std::format("{:#}", e) == formatter.format("{:#}", e));
	REQUIRE(std::format("{:#a}", e) == formatter.format("{:#a}", e));
	REQUIRE(std::format("{:#A}", e) == formatter.format("{:#A}", e));
}

TEST_CASE("Alternate Double Formatting") {
	ArgFormatter formatter;

	REQUIRE(std::format("{:#}", f) == formatter.format("{:#}", f));
	REQUIRE(std::format("{:#a}", f) == formatter.format("{:#a}", f));
	REQUIRE(std::format("{:#A}", f) == formatter.format("{:#A}", f));
}

TEST_CASE("Alternate Long Double Formatting") {
	ArgFormatter formatter;

	REQUIRE(std::format("{:#}", g) == formatter.format("{:#}", g));
	REQUIRE(std::format("{:#a}", g) == formatter.format("{:#a}", g));
	REQUIRE(std::format("{:#A}", g) == formatter.format("{:#A}", g));
}

TEST_CASE("String Type Formatting") {
	ArgFormatter formatter;

	REQUIRE(std::format("{:s}", h) == formatter.format("{:s}", h));
	REQUIRE(std::format("{:s}", i) == formatter.format("{:s}", i));
	REQUIRE(std::format("{:s}", j) == formatter.format("{:s}", j));
}

TEST_CASE("Bool Formatting") {
	ArgFormatter formatter;

	REQUIRE(std::format("{:b}", k) == formatter.format("{:b}", k));
	REQUIRE(std::format("{:B}", k) == formatter.format("{:B}", k));
	REQUIRE(std::format("{:o}", k) == formatter.format("{:o}", k));
	REQUIRE(std::format("{:x}", k) == formatter.format("{:x}", k));
	REQUIRE(std::format("{:X}", k) == formatter.format("{:X}", k));
}

TEST_CASE("Char Formatting") {
	ArgFormatter formatter;

	REQUIRE(std::format("{:b}", l) == formatter.format("{:b}", l));
	REQUIRE(std::format("{:B}", l) == formatter.format("{:B}", l));
	REQUIRE(std::format("{:c}", l) == formatter.format("{:c}", l));
	REQUIRE(std::format("{:d}", l) == formatter.format("{:d}", l));
	REQUIRE(std::format("{:o}", l) == formatter.format("{:o}", l));
	REQUIRE(std::format("{:x}", l) == formatter.format("{:x}", l));
	REQUIRE(std::format("{:X}", l) == formatter.format("{:X}", l));
}

TEST_CASE("Width Formatting") {
	ArgFormatter formatter;

	REQUIRE(std::format("{:10}", a) == formatter.format("{:10}", a));
	REQUIRE(std::format("{:10}", b) == formatter.format("{:10}", b));
	REQUIRE(std::format("{:10}", c) == formatter.format("{:10}", c));
	REQUIRE(std::format("{:10}", d) == formatter.format("{:10}", d));
	REQUIRE(std::format("{:10}", e) == formatter.format("{:10}", e));
	REQUIRE(std::format("{:10}", f) == formatter.format("{:10}", f));
	REQUIRE(std::format("{:10}", g) == formatter.format("{:10}", g));
	REQUIRE(std::format("{:10}", h) == formatter.format("{:10}", h));
	REQUIRE(std::format("{:10}", i) == formatter.format("{:10}", i));
	REQUIRE(std::format("{:10}", j) == formatter.format("{:10}", j));
	REQUIRE(std::format("{:10}", k) == formatter.format("{:10}", k));
	REQUIRE(std::format("{:10}", l) == formatter.format("{:10}", l));
	REQUIRE(std::format("{:10}", m) == formatter.format("{:10}", m));
	REQUIRE(std::format("{:10}", n) == formatter.format("{:10}", n));
}

TEST_CASE("Precision Formatting") {
	ArgFormatter formatter;

	REQUIRE(std::format("{:.5}", e) == formatter.format("{:.5}", e));
	REQUIRE(std::format("{:.5}", f) == formatter.format("{:.5}", f));
	REQUIRE(std::format("{:.5}", g) == formatter.format("{:.5}", g));
	REQUIRE(std::format("{:.5}", h) == formatter.format("{:.5}", h));
	REQUIRE(std::format("{:.5}", i) == formatter.format("{:.5}", i));
	REQUIRE(std::format("{:.5}", j) == formatter.format("{:.5}", j));
}

TEST_CASE("Sign Formatting") {
	ArgFormatter formatter;

	REQUIRE(std::format("{:+}", a) == formatter.format("{:+}", a));
	REQUIRE(std::format("{:+}", b) == formatter.format("{:+}", b));
	REQUIRE(std::format("{:+}", c) == formatter.format("{:+}", c));
	REQUIRE(std::format("{:+}", d) == formatter.format("{:+}", d));
	REQUIRE(std::format("{:+}", e) == formatter.format("{:+}", e));
	REQUIRE(std::format("{:+}", f) == formatter.format("{:+}", f));
	REQUIRE(std::format("{:+}", g) == formatter.format("{:+}", g));

	REQUIRE(std::format("{:-}", a) == formatter.format("{:-}", a));
	REQUIRE(std::format("{:-}", b) == formatter.format("{:-}", b));
	REQUIRE(std::format("{:-}", c) == formatter.format("{:-}", c));
	REQUIRE(std::format("{:-}", d) == formatter.format("{:-}", d));
	REQUIRE(std::format("{:-}", e) == formatter.format("{:-}", e));
	REQUIRE(std::format("{:-}", f) == formatter.format("{:-}", f));
	REQUIRE(std::format("{:-}", g) == formatter.format("{:-}", g));

	REQUIRE(std::format("{: }", a) == formatter.format("{: }", a));
	REQUIRE(std::format("{: }", b) == formatter.format("{: }", b));
	REQUIRE(std::format("{: }", c) == formatter.format("{: }", c));
	REQUIRE(std::format("{: }", d) == formatter.format("{: }", d));
	REQUIRE(std::format("{: }", e) == formatter.format("{: }", e));
	REQUIRE(std::format("{: }", f) == formatter.format("{: }", f));
	REQUIRE(std::format("{: }", g) == formatter.format("{: }", g));
}

TEST_CASE("Localization Formatting") {
	std::locale locale("en_US.UTF-8");
	ArgFormatter formatter;

	REQUIRE(std::format(locale, "{:L}", a) == formatter.format(locale, "{:L}", a));
	REQUIRE(std::format(locale, "{:L}", b) == formatter.format(locale, "{:L}", b));
	REQUIRE(std::format(locale, "{:L}", c) == formatter.format(locale, "{:L}", c));
	REQUIRE(std::format(locale, "{:L}", d) == formatter.format(locale, "{:L}", d));
	REQUIRE(std::format(locale, "{:L}", e) == formatter.format(locale, "{:L}", e));
	REQUIRE(std::format(locale, "{:L}", f) == formatter.format(locale, "{:L}", f));
	REQUIRE(std::format(locale, "{:L}", g) == formatter.format(locale, "{:L}", g));
	REQUIRE(std::format(locale, "{:L}", k) == formatter.format(locale, "{:L}", k));

	locale = std::locale("de_DE");
	REQUIRE(std::format(locale, "{:L}", a) == formatter.format(locale, "{:L}", a));
	REQUIRE(std::format(locale, "{:L}", b) == formatter.format(locale, "{:L}", b));
	REQUIRE(std::format(locale, "{:L}", c) == formatter.format(locale, "{:L}", c));
	REQUIRE(std::format(locale, "{:L}", d) == formatter.format(locale, "{:L}", d));
	REQUIRE(std::format(locale, "{:L}", e) == formatter.format(locale, "{:L}", e));
	REQUIRE(std::format(locale, "{:L}", f) == formatter.format(locale, "{:L}", f));
	REQUIRE(std::format(locale, "{:L}", g) == formatter.format(locale, "{:L}", g));
	REQUIRE(std::format(locale, "{:L}", k) == formatter.format(locale, "{:L}", k));

	locale = std::locale("zh-HK");
	REQUIRE(std::format(locale, "{:L}", a) == formatter.format(locale, "{:L}", a));
	REQUIRE(std::format(locale, "{:L}", b) == formatter.format(locale, "{:L}", b));
	REQUIRE(std::format(locale, "{:L}", c) == formatter.format(locale, "{:L}", c));
	REQUIRE(std::format(locale, "{:L}", d) == formatter.format(locale, "{:L}", d));
	REQUIRE(std::format(locale, "{:L}", e) == formatter.format(locale, "{:L}", e));
	REQUIRE(std::format(locale, "{:L}", f) == formatter.format(locale, "{:L}", f));
	REQUIRE(std::format(locale, "{:L}", g) == formatter.format(locale, "{:L}", g));
	REQUIRE(std::format(locale, "{:L}", k) == formatter.format(locale, "{:L}", k));

	locale = std::locale("hi_IN");
	REQUIRE(std::format(locale, "{:L}", a) == formatter.format(locale, "{:L}", a));
	REQUIRE(std::format(locale, "{:L}", b) == formatter.format(locale, "{:L}", b));
	REQUIRE(std::format(locale, "{:L}", c) == formatter.format(locale, "{:L}", c));
	REQUIRE(std::format(locale, "{:L}", d) == formatter.format(locale, "{:L}", d));
	REQUIRE(std::format(locale, "{:L}", e) == formatter.format(locale, "{:L}", e));
	REQUIRE(std::format(locale, "{:L}", f) == formatter.format(locale, "{:L}", f));
	REQUIRE(std::format(locale, "{:L}", g) == formatter.format(locale, "{:L}", g));
	REQUIRE(std::format(locale, "{:L}", k) == formatter.format(locale, "{:L}", k));
}

TEST_CASE("Fill-Align Formatting") {
	ArgFormatter formatter;

	REQUIRE(std::format("{:*^20}", a) == formatter.format("{:*^20}", a));
	REQUIRE(std::format("{:*<20}", a) == formatter.format("{:*<20}", a));
	REQUIRE(std::format("{:*>20}", a) == formatter.format("{:*>20}", a));

	REQUIRE(std::format("{:*^20}", b) == formatter.format("{:*^20}", b));
	REQUIRE(std::format("{:*<20}", b) == formatter.format("{:*<20}", b));
	REQUIRE(std::format("{:*>20}", b) == formatter.format("{:*>20}", b));

	REQUIRE(std::format("{:*^20}", c) == formatter.format("{:*^20}", c));
	REQUIRE(std::format("{:*<20}", c) == formatter.format("{:*<20}", c));
	REQUIRE(std::format("{:*>20}", c) == formatter.format("{:*>20}", c));

	REQUIRE(std::format("{:*^20}", d) == formatter.format("{:*^20}", d));
	REQUIRE(std::format("{:*<20}", d) == formatter.format("{:*<20}", d));
	REQUIRE(std::format("{:*>20}", d) == formatter.format("{:*>20}", d));

	REQUIRE(std::format("{:*^20}", e) == formatter.format("{:*^20}", e));
	REQUIRE(std::format("{:*<20}", e) == formatter.format("{:*<20}", e));
	REQUIRE(std::format("{:*>20}", e) == formatter.format("{:*>20}", e));

	REQUIRE(std::format("{:*^20}", f) == formatter.format("{:*^20}", f));
	REQUIRE(std::format("{:*<20}", f) == formatter.format("{:*<20}", f));
	REQUIRE(std::format("{:*>20}", f) == formatter.format("{:*>20}", f));

	REQUIRE(std::format("{:*^20}", g) == formatter.format("{:*^20}", g));
	REQUIRE(std::format("{:*<20}", g) == formatter.format("{:*<20}", g));
	REQUIRE(std::format("{:*>20}", g) == formatter.format("{:*>20}", g));

	REQUIRE(std::format("{:*^20}", h) == formatter.format("{:*^20}", h));
	REQUIRE(std::format("{:*<20}", h) == formatter.format("{:*<20}", h));
	REQUIRE(std::format("{:*>20}", h) == formatter.format("{:*>20}", h));

	REQUIRE(std::format("{:*^20}", i) == formatter.format("{:*^20}", i));
	REQUIRE(std::format("{:*<20}", i) == formatter.format("{:*<20}", i));
	REQUIRE(std::format("{:*>20}", i) == formatter.format("{:*>20}", i));

	REQUIRE(std::format("{:*^20}", j) == formatter.format("{:*^20}", j));
	REQUIRE(std::format("{:*<20}", j) == formatter.format("{:*<20}", j));
	REQUIRE(std::format("{:*>20}", j) == formatter.format("{:*>20}", j));

	REQUIRE(std::format("{:*^20}", k) == formatter.format("{:*^20}", k));
	REQUIRE(std::format("{:*<20}", k) == formatter.format("{:*<20}", k));
	REQUIRE(std::format("{:*>20}", k) == formatter.format("{:*>20}", k));

	REQUIRE(std::format("{:*^20}", l) == formatter.format("{:*^20}", l));
	REQUIRE(std::format("{:*<20}", l) == formatter.format("{:*<20}", l));
	REQUIRE(std::format("{:*>20}", l) == formatter.format("{:*>20}", l));

	REQUIRE(std::format("{:*^20}", m) == formatter.format("{:*^20}", m));
	REQUIRE(std::format("{:*<20}", m) == formatter.format("{:*<20}", m));
	REQUIRE(std::format("{:*>20}", m) == formatter.format("{:*>20}", m));

	REQUIRE(std::format("{:*^20}", n) == formatter.format("{:*^20}", n));
	REQUIRE(std::format("{:*<20}", n) == formatter.format("{:*<20}", n));
	REQUIRE(std::format("{:*>20}", n) == formatter.format("{:*>20}", n));
}

#if _MSC_VER >= 1930 && (_MSVC_LANG >= 202002L)
	#define CONTEXT                         std::back_insert_iterator<std::basic_string<char>>
	#define VFORMAT_TO(cont, loc, msg, ...) std::vformat_to<CONTEXT>(std::back_inserter(cont), loc, msg, std::make_format_args(__VA_ARGS__))
#elif(_MSC_VER >= 1929) && (_MSVC_LANG >= 202002L)
	#if _MSC_FULL_VER >= 192930145    // MSVC build that backported fixes for <format> under C++20 switch instead of C++ latest
		#define VFORMAT_TO(cont, loc, msg, ...) std::vformat_to(std::back_inserter(cont), loc, msg, std::make_format_args(__VA_ARGS__))
	#else
		#define CONTEXT                         std::basic_format_context<std::back_insert_iterator<std::basic_string<char>>, char>
		#define VFORMAT_TO(cont, loc, msg, ...) std::vformat_to(std::back_inserter(cont), loc, msg, std::make_format_args<CONTEXT>(__VA_ARGS__))
	#endif
#endif

TEST_CASE("Format Function Test") {
	ArgFormatter formatter;
	std::locale loc("");
	int width { 20 };
	std::string stdStr, argFmtStr;
	constexpr std::string_view fmt { "{0:*^#{1}x}" };

	VFORMAT_TO(stdStr, loc, fmt, a, width);
	formatter.format_to(std::back_inserter(argFmtStr), loc, fmt, a, width);
	REQUIRE(std::format(fmt, a, width) == formatter.format(fmt, a, width));
	REQUIRE(stdStr == argFmtStr);
}

////////////////////////////////////////////////////////////////////////////////////////
// This test is specifically to ensure that the problems encountered with Issues 1-3 are fully solved //
////////////////////////////////////////////////////////////////////////////////////////
struct TestStruct
{
	int a, b;
	std::string testMsg;
};
template<> struct formatter::CustomFormatter<TestStruct>
{
	enum class FmtView
	{
		empty,
		coordinate,
		x,
		y,
		msg,
		all,
		testErr
	};
	FmtView flag { FmtView::empty };

	inline constexpr void Parse(std::string_view parse) {
		auto size { parse.size() };
		auto pos { -1 };
		for( ;; ) {
				if( ++pos >= size ) {
						flag = FmtView::testErr;
						return;
				}
				if( parse[ pos ] == '}' ) return;
				if( parse[ pos ] != ':' ) continue;
				if( ++pos >= size ) {
						flag = FmtView::testErr;
						return;
				}
				switch( parse[ pos ] ) {
						case 'p': flag = FmtView::coordinate; continue;
						case 'x': flag = FmtView::x; continue;
						case 'y': flag = FmtView::y; continue;
						case 'm': flag = FmtView::msg; continue;
						case 'a': flag = FmtView::all; continue;
						default: continue;
					}
			}
	}

	template<typename resultCtx> constexpr auto Format(const TestStruct& p, resultCtx& ctx) const {
		switch( flag ) {
				case FmtView::coordinate: formatter::format_to(std::back_inserter(ctx), "({}, {})", p.a, p.b); break;
				case FmtView::x: formatter::format_to(std::back_inserter(ctx), "{}", p.a); break;
				case FmtView::y: formatter::format_to(std::back_inserter(ctx), "{}", p.b); break;
				case FmtView::msg: formatter::format_to(std::back_inserter(ctx), "{}", p.testMsg); break;
				case FmtView::empty: [[fallthrough]];
				case FmtView::all: formatter::format_to(std::back_inserter(ctx), "{}\n- Coordinate: ({}, {})", p.testMsg, p.a, p.b); break;
				case FmtView::testErr: [[fallthrough]];
				default: throw std::runtime_error("This Is Some Sort Of Error Message\n");
			}
	}
};

TEST_CASE("Custom Formatting: Position And Placement Test With And Without Other Arguments") {
	TestStruct test { .a = 42, .b = 52, .testMsg = "This Is Text From TestStruct" };

	// without other arguments but interchanging between manual and automatic argument advancement
	REQUIRE(formatter::format("{:p}", test) == "(42, 52)");
	REQUIRE(formatter::format("{0:p}", test) == "(42, 52)");
	REQUIRE(formatter::format("{:x}", test) == "42");
	REQUIRE(formatter::format("{0:x}", test) == "42");
	REQUIRE(formatter::format("{:y}", test) == "52");
	REQUIRE(formatter::format("{0:y}", test) == "52");
	REQUIRE(formatter::format("{:m}", test) == "This Is Text From TestStruct");
	REQUIRE(formatter::format("{0:m}", test) == "This Is Text From TestStruct");
	REQUIRE(formatter::format("{:a}", test) == "This Is Text From TestStruct\n- Coordinate: (42, 52)");
	REQUIRE(formatter::format("{0:a}", test) == "This Is Text From TestStruct\n- Coordinate: (42, 52)");
	REQUIRE(formatter::format("{}", test) == "This Is Text From TestStruct\n- Coordinate: (42, 52)");
	REQUIRE(formatter::format("{0}", test) == "This Is Text From TestStruct\n- Coordinate: (42, 52)");

	// without other arguments but with multiple brackets referencing the same custom type
	REQUIRE(formatter::format("{:m}\n- Coordinate: (X: {:x}, Y: {:y})", test) == "This Is Text From TestStruct\n- Coordinate: (X: 42, Y: 52)");

	// With other arguments AND interchanging between manual and automatic argument advancement
	REQUIRE(formatter::format("{:p}\n{}", test, a) == "(42, 52)\n424242424");
	REQUIRE(formatter::format("{0}\n{1:p}", a, test) == "424242424\n(42, 52)");
	REQUIRE(formatter::format("{:x}-{}", test, a) == "42-424242424");
	REQUIRE(formatter::format("{1:x}-{0}", a, test) == "42-424242424");
	REQUIRE(formatter::format("{:a}\n{}", test, a) == "This Is Text From TestStruct\n- Coordinate: (42, 52)\n424242424");
	REQUIRE(formatter::format("{1:a}\n{0}", a, test) == "This Is Text From TestStruct\n- Coordinate: (42, 52)\n424242424");
	// clang-format off
	REQUIRE(formatter::format("{1:m}\n- Coordinate: (X: {1:x}, Y: {1:y})\n-->{0}", a, test) ==
		                                                    "This Is Text From TestStruct\n- Coordinate: (X: 42, Y: 52)\n-->424242424");
	// clang-format on
}
////////////////////////////////////////////////////////////////////////////////////////
