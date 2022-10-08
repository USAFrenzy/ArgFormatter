# ArgFormatter

<div align="center">
<h2> IMPORTANT NOTE </h2>
</div>

- This library has been ported over from my [Serenity_Logger](https://github.com/USAFrenzy/Serenity_Logger) project and as of 08Oct22, still needs some minor modifications to work as a stand-alone library. Please note that the likelihood of this working out-of-the box right now is very low. If you're interested in seeing how this works though, please fill free to head over to [Serenity_Logger](https://github.com/USAFrenzy/Serenity_Logger) and grab the v1.0.0-beta release. You'll find the relevant files under "```include/serenity/MessageDetails/Arg*```" if grabbing the beta release for Windows, otherwise, the files will be found under "```Serenity/include/serenity/MessageDetails/Arg*```" and the one source file used is "```Serenity/src/MessageDetails/ArgFormatter.cpp```".

<div align="center">
<h2> What Is This Library? </h2>
</div>

This is a header-only library (Or at the very least, will be once the modifications are finished here -> right now there is one source file used) whose purpose is to serve as a limited replacement for ```<format>```/```libfmt```. This is more for use in my own projects more than anything else - the goal was simply to try to make something faster than ```<format>``` on Windows specifically, which this achieves. This hasn't been benched against ```libfmt``` itself though. This library is actually what I'm using over in my [Serenity_Logger](https://github.com/USAFrenzy/Serenity_Logger) project for the built in formatting, although, this current port is not a working stand-alone version as of right now (The ```Serenity_Logger``` usage is complete and fully functional).
