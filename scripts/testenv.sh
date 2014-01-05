#!/bin/bash
export PATH=`node -e "require('util').puts(require('path').resolve('build-native/bin/'))"`:$PATH
