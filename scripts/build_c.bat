@echo off

pushd ..
	cmake -B Compiled_Build -S . -DBUILD_COMPILED_LIB="ON"
popd