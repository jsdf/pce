#!/usr/bin/env bash
browserify index.js \
  --noparse="pcejs-atarist.js" \
  --ignore-missing \
  --standalone=PCEJSAtariST \
  > pcejs-atarist.umd.js