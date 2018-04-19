# ![cfMMOC LOGO](https://github.com/cfmmoc/cfmmoc/blob/master/cfmmoc.png) library for terrain rendering using OGRE
Licensed under The GNU General Public License v3.0 (GPLv3)

Any modification, re-utilization or copy of the source or binary format in other software or publications should mention a CITATION of this library.

Copyright (c) 2016-2018 by Authors (Jin Yan, Guanghong Gong, Ni Li and Luhao Xiao)



## Overview
cfMMOC represents a consolidated framework of multi-resolution management and occlusion culling.

cfMMOC library is a planet-scale out-of-core terrain rendering framework on top of OGRE v1.9.

| Build | Status |
|-------|--------|
| Fedora 27 64-bit | ![Building Pass](https://github.com/cfmmoc/cfmmoc-mics/blob/master/build-passing.png) |
| Ubuntu 17.10 64-bit | ![Building Pass](https://github.com/cfmmoc/cfmmoc-mics/blob/master/build-passing.png) |

## Features

 * Two-processes based framework
 * Out-of-core rendering
 * Terrain with level-of-details
 * Occlusion culling for invisibles
 * Crack-avoidance
 * Data fetching over networks



## Screenshot and Video
![screenshot](https://raw.githubusercontent.com/cfmmoc/cfmmoc/master/SNAP.png)

A demo video could be found at https://github.com/cfmmoc/cfmmoc-mics/blob/master/cfMMOC-short.mp4?raw=true.

A longer version of video is here: https://www.youtube.com/watch?v=8BmP0gtMc1M.



## Run-time Prerequisites
Install the following dependences (given versions are not mandatory) for cfMMOC:

 * `ois-1.3.0-14.fc27.x86_64`
 * `freeimage-3.17.0-12.fc27.x86_64`
 * `zziplib-0.13.62-10.fc27.x86_64`
 * `libcurl-7.55.1-8.fc27.x86_64` for fetching data from server
 * `libatomic-7.2.1-2.fc27.x86_64`
 * `libXaw-1.0.13-7.fc27.x86_64`
 * `mesa-libGLU-9.0.0-13.fc27.x86_64`
 * `poco-foundation-1.7.8p3-2.fc27.x86_64` for supporting multi-threading

In addtion, OGRE is also a dependency for cfMMOC, headers and binary of OGRE v1.9 is included in this repository. 
For compiling binary of OGRE, please refers to https://github.com/OGRECave/ogre. 
ORGE should be compiled with multithreading support with config OGRE_THREAD_SUPPORT = 2 and OGRE_THREAD_PROVIDER = 2 (i.e., POCO libraries as thread provider).

Dependencies for cfMMOC are all open source except an implementation of a Restricted QuadTreeS library, libRQTS.so.
We claim that libRQTS.so does not depend on any open source library or software except GNU C Library. 
None of libRQTS's dependency requires libRQTS to be an open source library. 
We CURRENTLY do not make libRQTS as an open source due to intellectual property issues.

## Compilation
Install compiler and dependences (given versions are not mandatory) for building cfMMOC:

 * `gcc-c++-7.2.1-2.fc27.x86_64`
 * `poco-devel-1.7.8p3-2.fc27.x86_64`
 * `ois-devel-1.3.0-14.fc27.x86_64`
 * `libcurl-devel-7.55.1-8.fc27.x86_64`

Download the source (https://github.com/cfmmoc/cfmmoc/archive/master.zip), extract it.

Directories are listed as follows:

directory | description
---- | ---
  Ogre/		|	headers of OGRE
  bin/		|	binary files
  browser/	|	headers and sources for `./bin/cfMMOC`, the executable entry of cfMMOC
  include/	|	headers for shared libraries `./bin/cfMMOC-back.so.1.9.0` and `./bin/cfMMOC-fore.so.1.9.0`
  media/	|	run-time resources
  src/		|	sources for `./bin/cfMMOC-back.so.1.9.0` and `./bin/cfMMOC-fore.so.1.9.0`

After installing the prerequisites, build cfMMOC as follows::

	mkdir obj 
	cd obj/
	make all -f ../Makefile
	make install -f ../Makefile

Running `make install -f ../Makefile`, will copy the binary file to the `bin/` directory.

Datasets can be downloaded from: https://github.com/cfmmoc/cfmmoc-dataset-ll



## System Specific Installation Instructions

Fedora is recommendation system for compiling and running cfMMOC.

The following specifics augment the foregoing general instructions for rapid
installation on the following systems:

### Ubuntu:

Install pre-requisites for run-time:

    sudo apt install libzzip-0-13 libatomic1 libois-1.3.0v5 libfreeimage3 libpocofoundation48

Install pre-requisites for compiling:

	sudo apt install make g++ libpoco-dev libois-dev libcurl3-dev

## Running cfMMOC:

Change directory to bin/ using `cd bin/`.

Run `./cfMMOC` in terminal, and then select cfMMOC-back.

Run `./cfMMOC` in a new terminal, and then select cfMMOC-fore, 1280x700 resolution is used as default in cfMMOC-fore, you could change it in `frender.cpp`.

As `./cfMMOC` runs for the first time, do not select full screen mode in configuration dialog, and 800x600 (or lower) resolution is recommended.

If `./cfMMOC` returns `./cfMMOC: error while loading shared libraries: libOgreMain.so.1.9.0: cannot open shared object file: No such file or directory` or similar error, please run `./cfMMOC` as `LD_PRELOAD=./libOgreMain.so.1.9.0:./libOgreOverlay.so.1.9.0 ./cfMMOC`.

## Configuration

Do NOT run cfMMOC on virtual machines.

Refer to this page, https://github.com/cfmmoc/cfmmoc-dataset-ll, for downloading and configuring dataset and data server.

Remote resource directory could be configured in bin/resources.cfg file, locate the line starting with 'cURL=http://', change resource directory there (further see troubleshooting for server accessibility).

cfMMOC could also fetch data from https://raw.githubusercontent.com/cfmmoc/cfmmoc-dataset-ll/master/***/*/*/*/* (NOT recommended, ONLY for data connectivity and accessibility), but no real-time performace is guaranteed by using resources from  Internet URLs. Downloading dataset from https://github.com/cfmmoc/cfmmoc-dataset-ll to server on local storage is recommanded.

When data server is configured, change working directory to bin/, run `./cfMMOC` command, and select cfMMOC-back sample to run.

Then, run `./cfMMOC` command again, and select cfMMOC-fore sample to run.


## Troubleshooting

If terrain tiles are not renderred in either back-end process or fore-end process --> check resource directory starting with `cURL=http://` in `bin/resources.cfg`, access file http://localhost/com/u/s/s/t.mesh via web browser to check server accessibility. Note that a trailing slash (`/`) is required at the end of that resource directory line.

If other problem occurs, post an issue at https://github.com/cfmmoc/cfmmoc/issues/new.
