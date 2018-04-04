# ![cfMMOC LOGO](https://github.com/cfmmoc/cfmmoc/blob/master/cfmmoc.png) library for terrain rendering using OGRE
Licensed under The GNU General Public License v3.0 (GPLv3)

Any modification, re-utilization or copy of the source or binary format in other software or publications should mention a CITATION of this library.

Copyright (c) 2016-2018 by Authors (Jin Yan, Guanghong Gong, Ni Li and Luhao Xiao)



## Overview
cfMMOC represents a consolidated framework of multi-resolution management and occlusion culling.

cfMMOC library is a planet-scale out-of-core terrain rendering framework on top of OGRE v1.9.



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



## Compilation
Install compiler and dependences (given versions are not mandatory) for building cfMMOC:

 * `gcc-c++-7.2.1-2.fc27.x86_64`
 * `poco-devel-1.7.8p3-2.fc27.x86_64`
 * `ois-devel-1.3.0-14.fc27.x86_64`
 * `libcurl-devel-7.55.1-8.fc27.x86_64`

Download the source (https://github.com/cfmmoc/cfmmoc/archive/master.zip), extract it, and build cfMMOC as follows:

After installing the prerequisites, perform the following:

    mkdir obj 
    cd obj/
    make all -f ../Makefile
    make install -f ../Makefile

Running `make install -f ../Makefile, will copy the binary file to the `bin/` directory.

Datasets can be downloaded from: https://github.com/cfmmoc/cfmmoc-dataset-ll



## System Specific Installation Instructions

The following specifics augment the foregoing general instructions for rapid
installation on the following systems:

Install pre-requisites:

    sudo apt install libpoco-dev libois-dev


## Running cfMMOC:

    cd bin/
    ./cfMMOC #then start cfMMOC-back 
    ./cfMMOC #then start cfMMOC-fore



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
