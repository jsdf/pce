#!/usr/bin/env bash
browserify index.js \
  --noparse="pcejs-util.js" \
  --ignore-missing \
  --standalone=PCEJSUtil \
  > pcejs-util.umd.js