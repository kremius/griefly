Griefly
===========

[![Linux Build Status](https://travis-ci.org/griefly/griefly.svg?branch=master)](https://travis-ci.org/griefly/griefly)
[![Windows Build Status](https://ci.appveyor.com/api/projects/status/github/griefly/griefly?branch=master&svg=true)](https://ci.appveyor.com/project/kremius/griefly)
[![Coverage Status](https://coveralls.io/repos/github/griefly/griefly/badge.svg?branch=master)](https://coveralls.io/github/griefly/griefly?branch=master)
[![Join the chat at https://gitter.im/griefly/griefly](https://badges.gitter.im/kremius/karya-valya.svg)](https://gitter.im/griefly/griefly?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

It is **Griefly: Yet Another Space Station Remake**.

Dependencies
-------------
1. Some C++ compiler (it works at least with compiler from **Visual Studio 2012** and **g++**)
2. Qt5 (e.g., **Desktop Qt 5.4.0 MSVC2012 OpenGL 32bit**). Qt Creator is optional, but desired
3. Python 2.7
4. Golang 1.5+ (https://golang.org/dl/)
5. Git
6. CMake 3.*

How to build everything on Windows
----------------------------------
_If you have any issues during the build process please report them_   

It should be possible to build everything without Qt Creator, but the guide assumes that Qt Creator is used.  

1. All tools and libs from the depencies list should be available on your machine. Tools and libs should be present in `%PATH%`
2. Open CMakeLists.txt in Qt Creator  
   Open Project -> Select CMakeLists.txt -> Select build folder
3. Run CMake with param `-DCMAKE_BUILD_TYPE=Release` for Release verison.
4. Build the project.  
   Client executables will appear in the `exec` folder, and the server executable will appear in the `griefly-server` folder.  
   Client executables depend from various dlls, so it is needed to manually place them to the `exec` folder.

**Note:** It's supposed to be built from an active git repository (`git describe --tags --abbrev=4` will be called during the process), otherwise it won't compile.  

How to build everything on Linux
--------------------------------

1. Install dependencies. Look into `.travis.yml` file for clues. (For Arch Linux only: All dependencies can be installed with ` sudo pacman -S git qt5-base qt5-multimedia go python2 mesa gcc`)
2. `cd` into installation directory.
3. `git clone https://github.com/griefly/griefly.git`
4. `cd griefly`
4. `./make.sh`. Built project will be placed under `exec` directory. Server will be
   built in `gopath/src/griefly-server` directory.

**Note:** It's supposed to be built from an active git repository (`git describe --tags --abbrev=4` will be called during the process), otherwise it won't compile.  
**Note:** gccgo is not supported! The current `FindGo.cmake` cannot parse the gccgo version string, so you will obtain an error. Use the official version instead.  
**Note:** There may be some issues with CMake 3.0.2, so try to update your CMake if issues appear.  

How to build everything on MacOS
--------------------------------

1. Install dependencies using brew. `brew install qt`
2. Look at the tips in `make.sh` and adjust your enviorment accordingly.
3. `./make.sh`. Build project will be placed under `exec` directory Server will be
   built in `gopath/src/griefly-server` directory.

How to start server
-------------------

Server available in directory `gopath/src/griefly-server`. When hosting server,
consider following adjustments:

1. Change password for admin user in db/auth.json. Passwords stored in plain
   text now.
2. When starting server, provide `-server-url` parameter. This parameter is an URL for asset server.
   It should be either host address (when server exposed to internet directly) or external ip
   (when running behind NAT). Server will bind on port, extracted from this url and
   clients will use url to connect to server for map exchange.

Other server options available in help: `griefly-server -h`

How to run game without launcher
--------------------------------

Pass those command line parameters to `KVEngine` or `KVEngine.exe`:

First (master) client:  
`mapgen_name=<path_to_mapgen> login=<admin login> password=<admin password>`  
where `login` and `password` params values should match values in an auth database (by default `griefly-server/db/auth.json`), and `path_to_mapgen` should be path to some mapgen file. Some default mapgen files are places in the `maps` (e.g. `maps/brig_small.gen`).  
Other clients:  
`login=Guest`

You can also specify game host by parameter `ip=game_host_address`. By default it connects to localhost.

Launcher basically does same things, but in a more convenient for usual users way.

Codestyle
----------
See `CODESTYLE.md` file for C++ codestyle. Use default ones for Python and Go (PEP8 and Go Coding Style accordingly).
