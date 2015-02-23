#!/bin/bash

# initialises build environment from PCEJS_config vars

export PCEJS_PREFIX=$(node -e "process.stdout.write(require('path').resolve('$PCEJS_conf_prefix'))")
export PCEJS_PACKAGEDIR=$(node -e "process.stdout.write(require('path').resolve('$PCEJS_conf_packagedir'))")
export PCEJS_TARGET=$PCEJS_conf_target
export PCEJS_OUTPUT_FORMAT=$PCEJS_conf_outputformat

if [[ -n $PCEJS_conf_emscripten ]]; then
  emflags=""
  # emflags+=" -s VERBOSE=1"
  if [[ -n $PCEJS_conf_imprecise64 ]]; then
    emflags+=" -s PRECISE_I64_MATH=0"
  fi

  if [[ -n $PCEJS_conf_memory ]]; then
    memory=$((PCEJS_conf_memory*1024*1024))
    emflags+=" -s TOTAL_MEMORY=${memory}"
  fi

  if [[ -n $PCEJS_conf_asmjs ]]; then
    emflags+=" -s ASM_JS=1"
  fi

  # emflags+=" -s OUTLINING_LIMIT=16000"
  if [[ -n $PCEJS_conf_outlining ]]; then
    emflags+=" -s OUTLINING_LIMIT=${PCEJS_conf_outlining}"
  fi

  if [[ -n $PCEJS_conf_exportfuncs ]]; then
    emflags+=" -s EXPORTED_FUNCTIONS=${PCEJS_conf_exportfuncs}"
  fi

  if [[ -n $PCEJS_conf_optlvl ]]; then
    emflags+=" -${PCEJS_conf_optlvl}"
  fi

  if [[ -n $PCEJS_conf_dbglvl ]]; then
    emflags+=" -g${PCEJS_conf_dbglvl}"
  fi

  export PCEJS_EMSDK_PATH="$PCEJS_conf_emsdkpath"
  export PCEJS_EMSCRIPTEN="yes"
  export PCEJS_EMFLAGS="$emflags"
  export PCEJS_CFLAGS="-Qunused-arguments -include src/include/pcedeps.h $emflags"
  export PCEJS_CONFIGURE="${PCEJS_conf_emsdkpath}/emconfigure ./configure"    
else
  export PCEJS_CFLAGS="-I/usr/local/opt/emscripten/system/include/emscripten/"
  export PCEJS_CONFIGURE="./configure"
fi
