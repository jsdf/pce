'use strict';

// Tests for the JavaScript that scripts/module_build.sh wraps around the
// emscripten output: commonjs/script-directory.js, prelude.js and epilogue.js

const test = require('node:test');
const assert = require('node:assert');
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const commonjsDir = path.resolve(__dirname, '../../commonjs');

function load(name) {
  return fs.readFileSync(path.join(commonjsDir, name), 'utf8');
}

// run code in a fresh context with the given globals, return the context
function run(code, globals) {
  const context = vm.createContext(Object.assign({}, globals));
  vm.runInContext(code, context);
  return context;
}

test.describe('script-directory.js', () => {
  const code = load('script-directory.js');

  function scriptDirectory(src) {
    const document = { currentScript: src === undefined ? null : { src } };
    return run(code, { document }).pcejsScriptDirectory;
  }

  test('is the directory of a pcejs umd bundle', () => {
    assert.strictEqual(
      scriptDirectory('https://cdn.example.com/pkg/pcejs-macplus.umd.js'),
      'https://cdn.example.com/pkg/'
    );
    assert.strictEqual(
      scriptDirectory('http://localhost/a/b/pcejs-atarist.umd.min.js'),
      'http://localhost/a/b/'
    );
  });

  test('ignores the query string and hash', () => {
    assert.strictEqual(
      scriptDirectory('http://localhost/x/pcejs-ibmpc.umd.js?v=2#top'),
      'http://localhost/x/'
    );
  });

  test('is empty for other scripts, e.g. an app bundle', () => {
    assert.strictEqual(scriptDirectory('http://localhost/static/js/main.js'), '');
    assert.strictEqual(scriptDirectory('http://localhost/pcejs-macplus.js'), '');
    assert.strictEqual(
      scriptDirectory('http://localhost/pcejs-macplus.umd.js/app.js'),
      ''
    );
  });

  test('is empty without a current script or document', () => {
    assert.strictEqual(scriptDirectory(undefined), '');
    assert.strictEqual(scriptDirectory(''), '');
    assert.strictEqual(run(code, {}).pcejsScriptDirectory, '');
  });
});

test.describe('prelude.js', () => {
  const code = load('prelude.js');

  function prelude(opts, pcejsScriptDirectory) {
    return run(code, { opts, pcejsScriptDirectory }).Module;
  }

  test('uses opts as the Module', () => {
    const opts = { canvas: {} };
    assert.strictEqual(prelude(opts, ''), opts);
    assert.deepStrictEqual(Object.keys(prelude(undefined, '')), []);
  });

  test('loads files from the script directory', () => {
    const Module = prelude({}, 'https://cdn.example.com/pkg/');
    assert.strictEqual(
      Module.locateFile('pce-macplus.wasm'),
      'https://cdn.example.com/pkg/pce-macplus.wasm'
    );
  });

  test('keeps a locateFile provided by the page', () => {
    const locateFile = (p) => '/assets/' + p;
    const Module = prelude({ locateFile }, 'https://cdn.example.com/pkg/');
    assert.strictEqual(Module.locateFile, locateFile);
  });

  test('leaves locateFile to emscripten without a script directory', () => {
    assert.strictEqual(prelude({}, '').locateFile, undefined);
  });

  test('preloads autoloadFiles into the root directory', () => {
    const Module = prelude(
      { autoloadFiles: ['mac-plus.rom', 'disks/hd1.qed'] },
      ''
    );
    const calls = [];
    Module.FS_createPreloadedFile = (...args) => calls.push(args);

    assert.strictEqual(Module.preRun.length, 1);
    Module.preRun[0]();

    assert.deepStrictEqual(calls, [
      ['/', 'mac-plus.rom', 'mac-plus.rom', true, true],
      ['/', 'hd1.qed', 'disks/hd1.qed', true, true],
    ]);
  });

  test('runs autoloading before other preRun functions', () => {
    const other = () => {};
    const Module = prelude({ autoloadFiles: [], preRun: [other] }, '');
    assert.strictEqual(Module.preRun.length, 2);
    assert.strictEqual(Module.preRun[1], other);
  });

  test('holds startup until asyncInit calls back', () => {
    let callback = null;
    const deps = [];
    const Module = prelude(
      {
        asyncInit(module, cb) {
          callback = cb;
        },
      },
      ''
    );
    Module.addRunDependency = (id) => deps.push('+' + id);
    Module.removeRunDependency = (id) => deps.push('-' + id);

    Module.preRun[0]();
    assert.deepStrictEqual(deps, ['+pcejsAsyncInit']);

    callback();
    assert.deepStrictEqual(deps, ['+pcejsAsyncInit', '-pcejsAsyncInit']);
  });
});

test.describe('epilogue.js', () => {
  const code = load('epilogue.js');

  test('exposes the emscripten file system apis on the Module', () => {
    const globals = {
      Module: {},
      FS: {},
      IDBFS: {},
      NODEFS: {},
      PATH: {},
      ERRNO_CODES: {},
      ERRNO_MESSAGES: {},
    };
    const { Module } = run(code, globals);

    for (const name of Object.keys(globals).filter((n) => n !== 'Module')) {
      assert.strictEqual(Module[name], globals[name], name);
    }
  });

  test('skips apis that the build doesn\'t include', () => {
    const { Module } = run(code, { Module: {}, FS: {} });
    assert.deepStrictEqual(Object.keys(Module), ['FS']);
  });
});
