#!/bin/zsh

if [ -z "$1" ]; then
  echo $0 theme_name colout_param... >&2
  exit 1
fi

t="$1"
shift
./colout -po "$@" > colout_"$t"
chmod u+x -- colout_"$t"
