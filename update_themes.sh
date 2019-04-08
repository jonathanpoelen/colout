#!/bin/bash

cd "$(dirname "$0")"

for f in colout_* ; do
  cmd=$(grep -m1 '^# colout' "$f")
  eval ./"${cmd:2}" > $f
done
