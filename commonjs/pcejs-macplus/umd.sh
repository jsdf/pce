#!/usr/bin/env bash
browserify index.js \
  --noparse="pcejs-macplus.js" \
  --ignore-missing \
  --standalone=PCEJSMacplus \
  > pcejs-macplus.umd.js