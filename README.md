# PCE.js

PCE.js runs classic computers in the browser. It's a port of Hampa Hug's excellent [PCE](http://www.hampa.ch/pce/) emulator, put together by [James Friend](https://jamesfriend.com.au/).

PCE.js currently emulates Mac Plus, IBM PC/XT and Atari ST functionally in recent versions of Chrome and Firefox.

More info: 

- [Demo running Mac Plus + System 7](https://jamesfriend.com.au/pce-js/) 
- [Why port emulators to the browser?](https://jamesfriend.com.au/why-port-emulators-browser)

![PCE.js Mac Plus](https://jamesfriend.com.au/files/pcejs.png)

## Emulator version

The emulators are based on [PCE](http://www.hampa.ch/pce/) 20250420-cc0c583c
(April 2025), with these changes for the browser:

- a main loop driven by `requestAnimationFrame` (`*_run_emscripten()` in
  `src/arch/*/cmd*.c`), which emulates the real time that passed since the
  previous browser frame instead of sleeping (sleeping busy waits in the
  browser)
- browser defaults in `src/arch/*/main.c` (config file `pce-config.cfg`,
  debug logging, VGA for the IBM PC)
- the Mac Plus mouse is set to the absolute host mouse position
- the Atari ST uses the `sdl` sound driver for the PSG by default
- key codes of some browser keys in `src/drivers/video/sdl.c`
- no terminal (termios) handling in `src/lib/sysdep.c`
- Web Audio autoplay handling in `libjs/library_sdl.js`
- fixed callback signatures in `src/devices/serport.c` (see below)

Functions that are called through a function pointer must have exactly the
signature of the pointer, otherwise the call fails in WebAssembly with
"function signature mismatch" (native builds tolerate this).

When updating to a newer PCE version, copy its `src/` directory and build
files over this tree and re-apply these changes.

## How to run PCE.js on your own website

See [this CodePen example](https://codepen.io/jsdf/pen/gOLryXM?editors=1100).

I recommend installing the [native version of PCE](http://www.hampa.ch/pce/) on your computer to create your own disk images. Alternatively, you could use [Mini vMac](https://www.gryphel.com/c/minivmac/).

## Installing from npm

PCE.js is available from npm as a set of [browserify](http://github.com/substack/node-browserify) compatible node packages, which also include [UMD](https://www.davidbcalhoun.com/2014/what-is-amd-commonjs-and-umd/) bundles.

There is one for each emulator build:
- [pcejs-macplus](http://npmjs.org/package/pcejs-macplus) - Mac Plus
- [pcejs-ibmpc](http://npmjs.org/package/pcejs-ibmpc) - IBM PC/XT
- [pcejs-atarist](http://npmjs.org/package/pcejs-atarist) - Atari ST

See each of the above links for install and usage instructions

## How to build PCE.js from source

**Note:** I recommend instead just using the npm packages listed above, unless you want to hack on the C source of the emulators themselves (which is not necessary if you just want to get them running on a page).

Make sure you've installed [node.js](http://nodejs.org/download/)

These instructions assume you're working with [my fork of PCE](https://github.com/jsdf/pce) on the 
`pcejs` branch. Presumably that's where you're reading this right now.

Run `npm install` in this directory (the source root). This should install the 
required node.js tools to build the commonjs modules and run the examples.

Install the [Emscripten SDK](https://kripken.github.io/emscripten-site/docs/getting_started/downloads.html).

Install and activate version 1.38.48 of the SDK

```bash
cd ../path/to/emsdk/
./emsdk install 1.38.48
./emsdk activate 1.38.48
source ./emsdk_env.sh

```

Check that running `emcc -v` successfully returns current Emscripten version.

Note: emscripten 1.38.48 runs its scripts with `python`, so on systems that
only have `python3` you need a `python` -> `python3` symlink in your `PATH`.
`./configure` also runs small test programs with node; on node 18 and newer
these fail ("Failed to parse URL") unless node's `fetch` is disabled, e.g. by
setting `NODE_JS = ['/usr/bin/node', '--no-experimental-fetch']` in the
emsdk's `.emscripten` config file.
Detailed installation instructions are on the [Emscripten SDK](https://kripken.github.io/emscripten-site/docs/getting_started/downloads.html) page.

In the same terminal, return to the pcejs repository. Run `./pcejs_build env` once which will create a `pcejs_build_conf.sh` file if it 
doesn't already exist. 

Most of the build process involves running the `./pcejs_build` bash script in the 
root of the repo. Commands should be run like `./pcejs_build [command]` or `pcejs_build [command] [arg]`

Run `./pcejs_build build [target]` to build the emulator, where `[target]` is `macplus`, 
`ibmpc` or `atarist`. This will output a `pce-[target].js` file to `dist/`.

After the output file for the target you're interested in has been built, you can:
- run the examples in the `example/` directory with `./pcejs_build example [target]` or `example/run_example.sh [target]`
- build the npm packages in the `commonjs/[target]/` directories by running 
  `npm run prepublish` in the respective directory.

Commands you might be interested in:

- build [target]: Configure, build and compile emulator to JS. [target] is either 
  one of `macplus`, `ibmpc`, `atarist` or `native`. Specifiying an emulator arch 
  builds the in-browser emulator JS file for that architecture. `native` builds 
  all PCE executables normally. If you don't specify a [target] then all JS 
  targets will be built.
- rebuild: Build last again (eg. after modifying C source)
- clean: Clean source tree
- [nothing]: Build all emulator JS targets and (commonjs modules for each)

Other commands (used internally by build scripts)

- env: Print build environment variables
- configure: Configure emulator build
- make: Compile emulator source to LLVM bitcode (used by 'build')
- remake: Recompile only changed files of emulator source to LLVM bitcode
- afterbuild: Convert LLVM bitcode to JS
- module: Build commonjs module (used by commonjs module prepublish scripts)





