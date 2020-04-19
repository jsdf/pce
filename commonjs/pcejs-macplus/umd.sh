#!/usr/bin/env bash
browserify index.js \
  --noparse="pcejs-macplus.js" \
  --ignore-missing \
  --standalone=PCEJSMacplus \
  > PCEJSMacplus.js