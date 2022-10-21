@echo off

echo -- Formatting Files...

pushd ..
		clang-format -i -style=file include/ArgFormatter/*.h
		clang-format -i -style=file src/*.cpp		
		clang-format -i -style=file tests/*.cpp
popd

echo -- Formatting Finished
