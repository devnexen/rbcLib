rbcLib
======

Portable C++ library for Robobuilder
Early version !

Needs boost, cmake, gcc/clang/VisualStudio compiler
1/ Plug and switch on Robobuilder on serial/USB port
2/ Type:
cmake .
build/bin/rbctool <tty> plugins/<pluginname>.<so|dylib>

* On Linux might be build/bin/rbctool /dev/ttyUSB0 plugins/diagtool.so
* On FreeBSD might be build/bin/rbctool /dev/ttyU0 plugins/diagtool.so


It will set it in Direct Mode Control and will check every servo (in HUNO configuration)
