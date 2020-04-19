#!/usr/bin/env bash
browserify index.js \
  --ignore-missing \
  --standalone=PCEJSUtil \
  > PCEJSUtil.js