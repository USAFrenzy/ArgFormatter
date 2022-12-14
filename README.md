<div align="center">
<h1> ArgFormatter </h1>
</div>

------------------------------------------

<div align="center">
<h2> What Is This Library? </h2>
</div>

------------------------------------------

<div align="center">

This is a header-only library whose purpose is to serve as a very limited replacement for ```<format>```/```fmtlib```. This is more for use in my own projects more than anything else - the goal was simply to try to make something faster than ```<format>``` on Windows specifically, which this achieves. This hasn't been benched against ```fmtlib``` itself though. This library is actually what I'm using over in my [Serenity_Logger](https://github.com/USAFrenzy/Serenity_Logger) project for the built-in native formatting. 
 The premise of this library is to be fully compatible with both ```<format>``` and ```fmtlib``` and thus uses the same exact formatting grammar and syntax as they do. The overall intent of ```ArgFormatter``` is to be a drop-in replacement for these two specifically on Windows (where this outshines ```<format>``` for the natively supported types in terms of performance; again, please note my performance claims are for MSVC's implementation only, ```fmtlib``` hasn't been benched against yet). While this library doesn't support UTF-16BE or UTF-32BE yet, it does support UTF-16LE, UTF-32LE, and UTF-8 text encodings thanks to the inclusion of another project I was working on, [UTF-Utils](https://github.com/USAFrenzy/UTF-Utils). Other than ```UTF-Utils```, this library has no other dependancy. 


</div>

This library is able to format:
- strings 
- c-style strings
- string views
- ints
- unsigned ints
- long ints
- long long ints
- unigned long long ints
- floats
- doubles
- long doubles
- const void*
- void*
- booleans
- chars
- std::tm
- And any custom type that you've provided a template specialization for with the ```CustomFormatter``` struct.

<br>

All of the above can be localized for a specific locale as well:
- Time arguments will be localized with the format and textual representation for a specific locale
  - Note: this is in regards to c-time formatting using ```std::tm``` data, ```<chrono>``` time-dates are not yet supported
  - Due to the nature of having Unicode support, this will inherently allow you to display the glyphs represented<br> 
    by the codepoints instead of the commmonly used character "```?```" assuming the font you're using supports <br>
    that glyph. 
  - The above Unicode point is where this library seems to differ from ```<format>```, as ```<format>``` uses "```?```" irregardless.
- booleans will have the correct true/false textual representation
- All other arithmetic values will be localized based on decimal and thousands separator groupings or according to the format flag(s) present

<br>

<div align="center">

How you format arguments here is *exactly* the same as how you format arguments with ```<format>``` and ```fmtlib``` so please reference Victor Zverovich's project [fmt](https://github.com/fmtlib/fmt) or the [std::format's](https://en.cppreference.com/w/cpp/utility/format/format) documentation for how to use formatting flags according to the formatting grammar used. I can't make an argument for usage of this library over ```fmtlib``` (given this library is the product of massive admiration for that library) without benching it first, but at the very least, if you're on windows using ```<format>``` already and want to squeeze out a lot more performance in regards to formatting, I *CAN* make an argument for using this library instead for the above listed formatting argument types, *ESPECIALLY* given how light-weight ```ArgFormatter``` is compared to ```<format>```. (Full bench suite coming soon).

</div>

------------------------------------------
<br>
<div align="center">
<h2> If This Is Header Only, What Do I Need To Include? </h2>
</div>

------------------------------------------

<div align="center">

------------------------------- The *ONLY* file that needs to be included is ```ArgFormatter.h```. -------------------------------

</div>

<div align="center">
<h3> IMPORTANT NOTE </h3>
</div>

<div align="center">

When you clone this repo, please make sure to run <br>"```git clone --recursive https://github.com/USAFrenzy/ArgFormatter.git```"<br><br>
This is so that the dependancy project, ```UTF-Utils```, is also cloned alongside ```ArgFormatter```, otherwise, you'll run into
some compilation issues when using this library. Both ```ArgFormatter``` and ```UTF-Utils``` use permissive licenses (MIT) so you
won't run into any limiting usages using either of them.

</div>


------------------------------------------