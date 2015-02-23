#!/bin/bash
set -e
set -u

# $1 is the relative path to resolve
function resolve_path {
  node -e "process.stdout.write(require('path').resolve('$1'))"
}

target=$1
if [[ -z $target ]]; then
  echo "missing pcejs arch argument [macplus ibmpc atarist]" >&2
  exit 1
fi

output_dir=resolve_path "commonjs/pcejs-${target}/lib/"
output_file="${output_dir}/pcejs-${target}.js"

mkdir -p "$output_dir"
echo -n "" > "$output_file"

module_wrapper_start="module.exports = function(deps, opts) {\n"
echo -n "$module_wrapper_start" >> "$output_file"

prelude_file=resolve_path "commonjs/prelude.js"
cat "$prelude_file" >> "$output_file"

target_module_file=resolve_path "dist/pce-${target}.js"
cat "$target_module_file" >> "$output_file"

epilogue_file=resolve_path "commonjs/epilogue.js"
cat "$epilogue_file" >> "$output_file"

module_wrapper_end="\nreturn Module;\n}"
echo -n "$module_wrapper_end" >> "$output_file"
