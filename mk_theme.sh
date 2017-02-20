#!/bin/zsh

t="$1"
shift

{
echo '#!/usr/bin/awk -f'
echo -n '\n# ./mk_theme.sh '"$t"' \\\n#'
for p in "$@" ; do
  [ $p = -- ] \
  && echo -n ' -- \\\n#' \
  || echo -n " '${p//'/'\\''}'"
done
echo "\n"
}> colout_"$t"

./colout -o "$@" <<<''
sed 's/^.//' *.out >> colout_"$t"
chmod u+x -- colout_"$t"
rm *.out
