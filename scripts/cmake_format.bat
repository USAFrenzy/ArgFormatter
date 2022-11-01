@echo -- Running cmake_format script...
@echo off
REM #####################################################################
REM #        Requires cmakelang to be installed and added to PATH       #
REM #####################################################################
pushd ..
cmake-format -i "CMakeLists.txt" -c "cmake-format.yaml"

	pushd src
			cmake-format -i "CMakeLists.txt" -c "../cmake-format.yaml"
	popd
	pushd tests
			cmake-format -i "CMakeLists.txt" -c "../cmake-format.yaml"
	popd
	pushd sandbox
		cmake-format -i "CMakeLists.txt" -c "../cmake-format.yaml"
	popd
popd
@echo -- Finished running cmake_format script