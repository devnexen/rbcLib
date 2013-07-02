rbcLib
======

Unix compliant C++ library for Robobuilder
Very early version !

Needs boost and cmake
1/ Plug and switch on Robobuilder on serial/USB port
2/ Type:
cmake .
build/bin/rbctool <tty> plugins/<pluginname>.<so|dylib>

It will set it in Direct Mode Control and will check every servo (in HUNO configuration)
