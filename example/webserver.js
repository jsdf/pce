#!/usr/bin/env node
// @flow

'use strict';

const spawn = require('child_process').spawn;
const exec = require('child_process').execSync;
const execFileSync = require('child_process').execFileSync;
const http = require('http');
const util = require('util');
const path = require('path');
const fs = require('fs');

const port = 8080;
const host = `http://127.0.0.1:${port}`;
const serveDir = path.resolve(process.argv[2] || process.cwd());

const filesMimeTypesCache = {};

function isPathInside(parent, child) {
  const relative = path.relative(parent, child);
  return (
    relative === '' ||
    (
      relative &&
      !relative.startsWith('..') &&
      !path.isAbsolute(relative)
    )
  );
}

function getSafeRequestPath(reqUrl) {
  const reqPath = reqUrl.replace(/\?.*/, '').replace(/_cb.*/, '');

  let decodedPath;
  try {
    decodedPath = decodeURIComponent(reqPath);
  } catch (e) {
    const err = new Error('Invalid URL encoding');
    err.statusCode = 400;
    throw err;
  }

  const reqPathFSPath = path.resolve(serveDir, '.' + decodedPath);

  if (!isPathInside(serveDir, reqPathFSPath)) {
    const err = new Error('Path traversal is not allowed');
    err.statusCode = 403;
    throw err;
  }

  return {
    reqPath: decodedPath,
    reqPathFSPath: reqPathFSPath
  };
}

function getMimeType(filepath) {
  if (!filesMimeTypesCache[filepath]) {
    switch (path.extname(filepath)) {
      case '.css':
        filesMimeTypesCache[filepath] = 'text/css';
        break;
      case '.js':
        filesMimeTypesCache[filepath] = 'application/javascript';
        break;
      case '.wasm':
        filesMimeTypesCache[filepath] = 'application/wasm';
        break;
      default:
        filesMimeTypesCache[filepath] = execFileSync(
          'file',
          ['--mime-type', '--brief', filepath]
        )
          .toString()
          .trim();
    }
  }
  return filesMimeTypesCache[filepath];
}

function handler(req, res) {
  let reqPath;
  let reqPathFSPath;

  try {
    const safePath = getSafeRequestPath(req.url);
    reqPath = safePath.reqPath;
    reqPathFSPath = safePath.reqPathFSPath;
  } catch (pathErr) {
    console.log(`${pathErr.statusCode || 400} ${req.url} ${pathErr}`);
    res.writeHead(pathErr.statusCode || 400, {'Content-Type': 'text/plain'});
    res.write(pathErr.stack);
    res.end();
    return;
  }

  function errRes(err, code) {
    console.log(`${code} ${req.url} ${err}`);
    res.writeHead(code, {'Content-Type': 'text/plain'});
    res.write(err.stack);
    res.end();
  }

  // does the request point to a valid file or dir at all?
  let reqPathStat = null;
  try {
    reqPathStat = fs.lstatSync(reqPathFSPath);
  } catch (reqPathStatErr) {
    // nothing there
    return errRes(reqPathStatErr, 404);
  }

  try {
    // return file or index file contents
    const filepath = reqPathStat.isDirectory()
      ? path.join(reqPathFSPath, 'index.html')
      : reqPathFSPath;

    if (!isPathInside(serveDir, filepath)) {
      return errRes(new Error('Path traversal is not allowed'), 403);
    }

    const mimeType = getMimeType(filepath);
    const contents = fs.readFileSync(filepath);
    console.log(`200 ${req.url} ${mimeType}`);
    res.setHeader('Cross-Origin-Opener-Policy', 'same-origin');
    res.setHeader('Cross-Origin-Embedder-Policy', 'require-corp');
    res.writeHead(200, {'Content-Type': mimeType});
    res.write(contents);
    res.end();
  } catch (fileReadErr) {
    if (reqPathStat.isDirectory()) {
      // render directory listing
      try {
        const filepath = path.join(serveDir, reqPath);

        if (!isPathInside(serveDir, filepath)) {
          return errRes(new Error('Path traversal is not allowed'), 403);
        }

        const dirlinks = ['..', ...fs.readdirSync(filepath)]
          .map((file) => {
            const fileStat = fs.lstatSync(path.join(reqPathFSPath, file));
            const filename = fileStat.isDirectory() ? `${file}/` : file;

            return `<li><a href="${path.join(
              reqPath,
              filename
            )}">${filename}</a></li>`;
          })
          .join('\n');
        console.log(`200 ${req.url} [dir listing] 'text/html'`);
        res.writeHead(200, {'Content-Type': 'text/html'});
        res.write(`<!DOCTYPE html>
  <html>
  <head>
    <title>Directory listing of ${reqPath}</title>
  </head>
  <body>
  <h1>Directory listing of ${reqPath}</h1>
  <ul>${dirlinks}</ul>
  </body>
  </html>`);
        res.end();
        return;
      } catch (dirlistErr) {
        // directory listing failed somehow
        return errRes(dirlistErr, 500);
      }
    } else {
      // there was a file or dir but we couldn't read it
      return errRes(fileReadErr, 500);
    }
  }
}

const server = http.createServer(handler);
server.listen(port);

console.log(
  `
opening http://127.0.0.1:${port}/ in your browser

press CTRL-C to quit this program
`
);
exec(`open http://127.0.0.1:${port}/`);
