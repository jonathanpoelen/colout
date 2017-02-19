#!/bin/zsh

echo -e "#!/usr/bin/awk -f\n# ./mk_theme.sh ${@:q}\n" > colout_"$1"
t="$1"
shift
./colout -o "$@" <<<''
sed 's/^.//' *.out >> colout_"$t"
chmod u+x -- colout_"$t"
rm *.out
