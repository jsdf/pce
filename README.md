# PCE.js

PCE.js runs classic computers in the browser. It's a port of Hampa Hug's excellent [PCE](http://www.hampa.ch/pce/) emulator, put together by [James Friend](http://jamesfriend.com.au/).

PCE.js currently emulates Mac Plus, IBM PC/XT and Atari ST functionally in recent versions of Chrome and Firefox.

More info: 

- [Demo running Mac Plus + System 7](http://jamesfriend.com.au/pce-js/) 
- [Why port emulators to the browser?](http://jamesfriend.com.au/why-port-emulators-browser)

![PCE.js Mac Plus](http://jamesfriend.com.au/sites/default/files/pcejs.png)

## Installing from npm

PCE.js is available from npm as a set of [browserify](http://github.com/substack/node-browserify) compatible node packages.

There is one for each emulator build:
- [pcejs-macplus](http://npmjs.org/package/pcejs-macplus) - Mac Plus
- [pcejs-ibmpc](http://npmjs.org/package/pcejs-ibmpc) - IBM PC/XT
- [pcejs-atarist](http://npmjs.org/package/pcejs-atarist) - Atari ST

See each of the above links for install and usage instructions

## How to build PCE.js from source

**Note:** This is a bit complicated. I recommend using the npm packages listed above, unless you really want to hack on the C source of the emulators themselves (which is not necessary if you just want to get them running on your website).

Make sure you've installed [node.js](http://nodejs.org/download/) and [Grunt](http://gruntjs.com/getting-started)

Run `npm install` in this directory (the source root). This should install the 
required node.js modules. You might want to `npm install -g coffee-script http-server` also.

Clone [my fork of Emscripten](https://github.com/jsdf/emscripten) and checkout 
the `pcejs-fastcomp` branch (now that we're using [emscripten-fastcomp](https://github.com/kripken/emscripten/wiki/LLVM-Backend), you'll want to get that working, or disable it by running your commands prefixed with EMCC_FAST_COMPILER=0). Add the path to the emscripten fork repo to your `$PATH` so you can run `emcc`. Similarly you 
should be working with [my fork of PCE](https://github.com/jsdf/pce) on the 
`pcejs` branch, but that's where you're reading this, right?

Most of the build process involves running Grunt tasks. Run `grunt --help` for a 
list of possiblities.

Run `grunt build:[target]` to build the emulator, where [target] is `macplus`, 
`ibmpc` or `atarist`. This will output a `pce-[target].js` file to `dist/`.

Once the output file for the target you're interested in has been built, you can:
- run the examples in the `example/` directory with `example/run_example.sh [target]`
- build the npm packages in the `commonjs/[target]/` directories using their respective `prepublish.sh` scripts


Check out the emulators in which pce runs on:

http://jamesfriend.com.au/pce-js/



