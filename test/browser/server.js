#!/usr/bin/env node
'use strict';

// Static file server for the browser tests: node test/browser/server.js <port>
//
// /<arch>/ is the test page (page.html). Other files under /<arch>/ come from
// the emulator's commonjs package (UMD bundle, .wasm, pcex ROM), the
// pcejs-util package and the downloaded system files, in that order, so the
// pcex ROM that matches the build is used instead of the one in the system zip.

const http = require('http');
const fs = require('fs');
const path = require('path');

const ARCHS = ['macplus', 'ibmpc', 'atarist'];

const root = path.resolve(__dirname, '../..');
const port = Number(process.argv[2] || 8181);

const MIME_TYPES = {
  '.html': 'text/html',
  '.js': 'application/javascript',
  '.wasm': 'application/wasm',
  '.cfg': 'text/plain',
};

function searchDirs(arch) {
  return [
    path.join(root, 'commonjs', `pcejs-${arch}`),
    path.join(root, 'commonjs', 'pcejs-util'),
    path.join(__dirname, '..', '.cache', 'systems', arch),
  ];
}

function findFile(arch, name) {
  if (name === '') {
    return path.join(__dirname, 'page.html');
  }

  // only plain file names, no paths
  if (name !== path.basename(name) || name.startsWith('.')) {
    return null;
  }

  for (const dir of searchDirs(arch)) {
    const file = path.join(dir, name);
    if (fs.existsSync(file) && fs.statSync(file).isFile()) {
      return file;
    }
  }

  return null;
}

const server = http.createServer((req, res) => {
  const url = new URL(req.url, 'http://localhost');
  const [, arch, name = ''] = url.pathname.split('/');

  if (url.pathname === '/healthz') {
    res.writeHead(200, { 'Content-Type': 'text/plain' });
    res.end('ok');
    return;
  }

  const file = ARCHS.includes(arch) ? findFile(arch, name) : null;

  if (file === null) {
    res.writeHead(404, { 'Content-Type': 'text/plain' });
    res.end(`not found: ${url.pathname}`);
    return;
  }

  res.writeHead(200, {
    'Content-Type': MIME_TYPES[path.extname(file)] || 'application/octet-stream',
    'Cache-Control': 'no-store',
  });
  fs.createReadStream(file).pipe(res);
});

server.listen(port, '127.0.0.1', () => {
  console.log(`test server on http://127.0.0.1:${port}/`);
});
