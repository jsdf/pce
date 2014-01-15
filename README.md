# PCE.js

PCE.js runs classic computers in the browser. It's a port of Hampa Hug's excellent [PCE](http://www.hampa.ch/pce/) emulator, put together by [James Friend](http://jamesfriend.com.au/).

PCE.js currently emulates Mac Plus, IBM PC/XT and Atari ST functionally in recent versions of Chrome and Firefox.

More info: 

- [Demo running Mac Plus + System 7](http://jamesfriend.com.au/pce-js/) 
- [Why port emulators to the browser?](http://jamesfriend.com.au/why-port-emulators-browser)

![PCE.js Mac Plus](http://jamesfriend.com.au/sites/default/files/pcejs.png)

## How to build PCE.js

Make sure you've installed [node.js](http://nodejs.org/download/) and [Grunt](http://gruntjs.com/getting-started)

Run `npm install` in this directory (the source root). This should install the 
required node.js modules. You might want to `npm install -g coffee-script http-server` also.

Clone [my fork of Emscripten](https://github.com/jsdf/emscripten) and checkout 
the `pcejs` branch. Add it to your `$PATH` so you can run `emcc`. Similarly you 
should be working with [my fork of PCE](https://github.com/jsdf/pce) on the 
`pcejs` branch, but that's where you're reading this, right?

Most of the build process involves running Grunt tasks. Run `grunt --help` for a 
list of possiblities.

Make sure you've got a directory containing a working set of files for the emulator.
This means: a rom file, a config file, and some media (hard disk and/or floppy images).
You can use a normal build of PCE test these out eg. one obtained from the 
[website](http://www.hampa.ch/pce/download.html), or your package manager of 
choice (in my case, [homebrew](http://brew.sh/)). 

Your config file should be named `pce-config.cfg`. Once you've got these together 
in a directory, run `grunt romdir:[your directory]` which will symlink the 
directory into the source tree at `roms/`. This is necessary for when the data 
files are packaged up for the browser.

Run `grunt build:[target]` to build the emulator, where [target] is `macplus`, 
`ibmpc` or `atarist`.

You'll find the built files at `pce-js/` and `grunt run` will serve them up for 
your browser on `http://localhost:8080/`. That's it.
