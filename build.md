Build for two architectures
====

such a  build can be arranged by the following commands, but the code has to provide for it as well.


cmake -G "Visual Studio 16 2019" -A Win32 -S \path_to_source\ -B "build32"
cmake -G "Visual Studio 16 2019" -A x64 -S \path_to_source\ -B "build64"
cmake --build build32 --config Release
cmake --build build64 --config Release