# PCE.js

## How to build PCE.js

Make sure you've installed [node.js](http://nodejs.org/download/) and [grunt](http://gruntjs.com/getting-started)

Run `npm install` in this directory (the source root). This should install the 
required node.js modules. You might want to `npm install -g coffee-script http-server` also.

Clone [my fork of Emscripten](https://github.com/jsdf/emscripten) and checkout 
the `pcejs` branch. Similarly you should be working with [my fork of PCE](https://github.com/jsdf/pce) 
on the `pcejs` branch, but if you hadn't you wouldn't be reading this, right?

Most of the build process involves running Grunt tasks. Run `grunt --help` for a 
list of possiblities.

Make sure you've got a directory containing a working set of files for the emulator.
This means: a rom file, a config file, and some media (hard disk and/or floppy images).
You can use a normal build of PCE test these out eg. one obtained from the 
[website](http://www.hampa.ch/pce/download.html), or your package manager of 
choice (in my case, [homebrew](http://brew.sh/)). Your config file should be
named `pce-config.cfg`. Once you've got these together in a directory, run
`grunt romdir:[your directory]` which will symlink into the source tree at `roms/`.

Run `grunt build:[target]` to build the emulator, where [target] is `macplus`, 
`ibmpc` or `atarist`.

You'll find the built files at `pce-js/` and `grunt run` will serve them up for 
your browser on `http://localhost:8080/`. That's it.

## TODO
- main loop
	- replace fixed emscripten step with dynamic time accumulator
- mouse
	- interpolation over emscriptem step?
- configuration
	- inject configuration via client	
- interface
	- loading files
