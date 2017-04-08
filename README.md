# PCE.js

PCE.js runs classic computers in the browser. It's a port of Hampa Hug's excellent [PCE](http://www.hampa.ch/pce/) emulator, put together by [James Friend](https://jamesfriend.com.au/).

PCE.js currently emulates Mac Plus, IBM PC/XT and Atari ST functionally in recent versions of Chrome and Firefox.

More info: 

- [Demo running Mac Plus + System 7](https://jamesfriend.com.au/pce-js/) 
- [Why port emulators to the browser?](https://jamesfriend.com.au/why-port-emulators-browser)

![PCE.js Mac Plus](https://jamesfriend.com.au/sites/default/files/pcejs.png)

## Installing from npm

PCE.js is available from npm as a set of [browserify](http://github.com/substack/node-browserify) compatible node packages.

There is one for each emulator build:
- [pcejs-macplus](http://npmjs.org/package/pcejs-macplus) - Mac Plus
- [pcejs-ibmpc](http://npmjs.org/package/pcejs-ibmpc) - IBM PC/XT
- [pcejs-atarist](http://npmjs.org/package/pcejs-atarist) - Atari ST

See each of the above links for install and usage instructions

## How to build PCE.js from source

**Note:** This is a bit complicated. I recommend instead  just using the npm packages listed above, unless you really want to hack on the C source of the emulators themselves (which is not necessary if you just want to get them running on a page).

Make sure you've installed [node.js](http://nodejs.org/download/)

Run `npm install` in this directory (the source root). This should install the 
required node.js tools to build the commonjs modules and run the examples.

Clone [my fork of Emscripten](https://github.com/jsdf/emscripten) and checkout 
the `pcejs-fastcomp` branch. Set it up according to the directions for running 
[emscripten-fastcomp](https://github.com/kripken/emscripten/wiki/LLVM-Backend). 
~~Add the path to the emscripten fork repo to your `$PATH` so you can run `emcc`.~~
Run `./pcejs_build env` once which will create a `pcejs_build_conf.sh` file if it 
doesn't already exist. Then edit this file so the `PCEJS_conf_emsdkpath` setting
points to where you cloned the emscripten fork repo (eg. where `emcc` can be found).

Similarly you should be working with [my fork of PCE](https://github.com/jsdf/pce) on the 
`pcejs` branch, but presumably that's where you're reading this right now.

Most of the build process involves running the `pcejs_build` bash script in the 
root of the repo. Commands should be run like `pcejs_build [command]` or `pcejs_build [command] [arg]`

Commands you might be interested in:

- build [target]: Configure, build and compile emulator to JS. [target] is either 
  one of `macplus`, `ibmpc`, `atarist` or `native`. Specifiying an emulator arch 
  builds the in-browser emulator JS file for that architecture. `native` builds all PCE 
  executables normally. If you don't specify a [target] then all JS targets will
  be built.
- rebuild: Build last again
- clean: Clean source tree
- [nothing]: Build all emulator JS targets and (commonjs modules for each)

Other commands (used internally by build scripts)

- env: Print build environment variables
- configure: Configure emulator build
- make: Compile emulator source to LLVM bitcode (used by 'build')
- remake: Recompile only changed files of emulator source to LLVM bitcode
- afterbuild: Convert LLVM bitcode to JS
- module: Build commonjs module (used by commonjs module prepublish scripts)

Run `pcejs_build build [target]` to build the emulator, where `[target]` is `macplus`, 
`ibmpc` or `atarist`. This will output a `pce-[target].js` file to `dist/`.

Once the output file for the target you're interested in has been built, you can:
- run the examples in the `example/` directory with `example/run_example.sh [target]`
- build the npm packages in the `commonjs/[target]/` directories by running 
  `npm run prepublish` in the respective directory.




