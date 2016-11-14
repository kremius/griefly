Griefly
===========

[![Build Status](https://travis-ci.org/griefly/griefly.svg?branch=master)](https://travis-ci.org/griefly/griefly)
[![Coverage Status](https://coveralls.io/repos/github/griefly/griefly/badge.svg?branch=master)](https://coveralls.io/github/griefly/griefly?branch=master)
[![Join the chat at https://gitter.im/griefly/griefly](https://badges.gitter.im/kremius/karya-valya.svg)](https://gitter.im/griefly/griefly?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

It is **Griefly: Yet Another Space Station Remake**.

Dependencies
-------------
1. Some C++ compiler (it works at least with compiler from **Visual Studio 2012** and **g++**)
2. Qt5 (e.g., **Desktop Qt 5.4.0 MSVC2012 OpenGL 32bit**). Qt Creator is optional, but desired
3. SFML 2.*
4. libpng
5. zlib
6. Python 2.7
7. Golang 1.5+ (https://golang.org/dl/)
8. Git
9. CMake 3.*

How to build everything on Windows
----------------------------------
_If you have any issues during the build process please report them_   

It should be possible to build everything without Qt Creator, but the guide assumes that Qt Creator is used.  

1. All tools and libs from the depencies list should be available on your machine
2. Open CMakeLists.txt in Qt Creator  
   Open Project -> Select CMakeLists.txt -> Select build folder
3. Environment variables  
   In the opened project: Projects _(left column in Qt Creator)_ -> Build Environment  
   It is needed to set environment or usual CMake variables for some libs. Other tools and libs should be present in `%PATH%`, so if you have installed them then everything should be fine.
   Mine setted as:  
  `Set PNG_INCLUDE_DIR to C:\Users\Kremius\Documents\ExternalLibs\lpng1612`  
  `Set PNG_LIBRARY_DIR to C:\Users\Kremius\Documents\ExternalLibs\lpng1612\projects\vstudio\Release`  
  `Set SFML_ROOT to C:\Users\Kremius\Documents\ExternalLibs\SFML-2.3`  
  `Set ZLIB_ROOT to C:\Users\Kremius\Documents\ExternalLibs\zlib128-dll`  
  If you cannot open the environment variables table (Qt Creator 3.\* does not allow that before successfull CMake generation) or prefer usual variables in CMake (Qt Creator 4.\* allows to use them in convenient way) then you can set them as `-DVARIABLE=VALUE` (e.g. `-DSFML_ROOT=C:\Users\Kremius\Documents\ExternalLibs\SFML-2.3`)
4. Run CMake with param `-DCMAKE_BUILD_TYPE=Release` for Release verison.
5. Build the project.  
   Client executables will appear in the `exec` folder, and the server executable will appear in the `griefly-server` folder.  
   Client executables depend from various dlls (Qt, SFML), so it is needed to manually place them to the `exec` folder.

**Note:** It is supposed to perform build from an active git repository (`git describe --tags --abbrev=4` will be called during the process).

How to build everything on Linux
--------------------------------

1. Install dependencies. Look into `.travis.yml` file for clues.
2. `./make.sh`. Built project will be placed under `exec` directory. Server will be
   built in `gopath/src/griefly-server` directory.

**Note:** It is supposed to perform build from an active git repository (`git describe --tags --abbrev=4` will be called during the process).  
**Note:** gccgo is not supported! The current `FindGo.cmake` cannot parse the gccgo version string, so you will obtain an error. Use the official version instead.  
**Note:** There may be some issues with CMake 3.0.2, so try to update your CMake if issues appear.  

How to start server
-------------------

Just run the executable from directory `gopath/src/griefly-server`.

How to run game without launcher
--------------------------------

Pass those command line parameters to `KVEngine` or `KVEngine.exe`:

First (master) client:  
`mapgen_name=maps/brig_small.gen -auto_connect login=<admin login> password=<admin password>`  
where `login` and `password` params values should match values in an auth database (by default `griefly-server/db/auth.json`)  
Other clients:  
`-auto_connect login=Guest`

You can also specify game host by parameter `ip=game_host_address`. By default it connects to localhost.

Codestyle
----------
See `CODESTYLE.md` file for C++ codestyle. Use default ones for Python and Go (PEP8 and Go Coding Style accordingly).
