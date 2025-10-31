<!-- [![version.badge]](http://semver.org) -->

# Colout -- Add colors to a text stream in your terminal

Inspired by https://nojhan.github.io/colout/

This repo a bash script (`colout`) and a few predefined themes (`colout_*`).

## Synopsis

`colout` -h

`colout` -t [theme]

`colout` -l [COLORS_AND_STYLES...]

`colout [OPTIONS] PATTERN [COLORS_AND_STYLES...] [-- [OPTIONS] PATTERN [COLORS_AND_STYLES...] [-- ...]]

`colout` [-rcankRCeENpo] [-i group_indexes] [-s scale] [-S group_index] [-I group_index] [-u units] [-x awk_expr] [-X awk_expr] [-y awk_expr] [-Y awk_expr] [-f awk_func] [-v awk_variable] PATTERN [COLORS_AND_STYLES...] [-- ...]


## Installation

```bash
ln -s colout ~/bin
```

`~/bin` must be in your `PATH` variable.

## Dependencies

- `gawk`
- `bash`


## Options

- `-t [theme]`:  List or apply. (load `${COLOUT_PREFIX_PATH:-${0}_}${theme}`)
- `-l`:  (line) Colors each lines.
- `-r`:  (repeat/again) Apply PATTERN until there is no more matching.
- `-a`:  (all) Loop on all PATTERN until there is no more matching.
- `-c`:  (continue) Try PATTERN even if previous ones match.
- `-n`:  (next) Each match passes to the next color.
- `-k`:  (keep) Use the previous color map.
- `-R`:  (no-reset) Do not reset colors.
- `-C`:  (color-continue) The current color becomes the normal color.
- `-e`:  (esc-reset) Reset the normal color.
- `-E`:  (eol esc-reset) Reset the normal color at end of line.
- `-N`:  (lc-numeric) Use the locale's decimal point character when parsing input data.
- `-p`:  (print) print awk/sed command.
- `-o`:  (optimize) See man awk `-o`/`--optimize`.
- `-i group_indexes`:  Ignore groups (space and comma-separated list).
- `-s scale`:  'min,max' or 'max'. Apply colors linearly between min and max (0,100 by default).
- `-S group_index`:  Use this group to compute the interval value. Implies `-s`.
- `-I group_index`:  Shortcut for `-S group_index -i group_index`. Implies `-s`.
- `-u units`:  Cuts the color range per unit. Implies `-s`.
- `-x awk_expr`:  An expression that returns the color index.
  - `v`: the extracted value
  - `n`: the number of color
  - `u`: (only if `-u`) the unit position (`1` is first unit, `0` is an unrecognized unit).
- `-X awk_expr`:  An expression that returns a numeric value between min and max (see `-s scale`).
  - `v`: the extracted value
  - `n`: the number of color
  - `u`: (only if `-u`) the unit position (`1` is first unit, `0` is an unrecognized unit).
- `-y awk_expr`:  An expression that returns the color index. Equivalent to `-x 'getdigits(awk_expr)'`
  - `v`: the extracted digit value
  - `n`: the number of color
  - `u`: (only if `-u`) the unit position (`1` is first unit, `0` is an unrecognized unit).
- `-Y awk_expr`:  An expression that returns a numeric value between min and max (see `-s scale`). Equivalent to `-X 'getdigits(awk_expr)'`
  - `v`: the extracted digit value
  - `n`: the number of color
  - `u`: (only if `-u`) the unit position (`1` is first unit, `0` is an unrecognized unit).
- `-f awk_script`:  Awk user functions.
- `-v awk_script`:  Awk user variables.


## PATTERN

  Awk pattern expression.


## COLORS_AND_STYLES

- A style name
- A color name
- A colormap name
- A number from 0 to 255 (256-color mode)
- A triplet from 0 and 255 separated by `/` (true color mode)
- `#` followed by 3 or 6 hexadecimal values (true color mode)
- `e` followed by a number from 0 to 15 (4-bits color mode)
- `bg`
- Or a comma-separated list (or `=`) of those values.

### Styles

- `normal`,`n`
- `bold`,`o`
- `d[im]`
- `i[talic]`
- `u[nderline]`
- `d[ouble_]u[nderline],U`
- `s[trike]`
- `blink`,`l`
- `reverse`,`v`
- `h[idden]`
- `reset`,`rr`
- `r[eset_]bold`,`ro`
- `r[eset_]d[im]`
- `r[eset_]i[talic]`
- `r[eset_]u[nderline]`
- `r[eset_]d[ouble_]u[nderline]`,`rU`
- `r[eset_]s[trike]`
- `r[eset_]blink`,`rl`
- `r[eset_]reverse`,`rv`
- `r[eset_]h[idden]`

### Named colors

An uppercase character is considered to be bold.

- `default`,`none`
- `[blac]k`
- `w[hite]`
- `r[ed]`
- `g[reen]`
- `b[lue]`
- `y[ellow]`
- `m[agenta]`
- `c[yan]`
- `gray`,`a`
- `d[ark_]gray`,`da`
- `l[ight_]red`,`lr`
- `l[ight_]green`,`lg`
- `l[ight_]blue`,`lb`
- `l[ight_]yellow`,`ly`
- `l[ight_]magenta`,`lm`
- `l[ight_]cyan`,`lc`

### Colormap

`r:` for reverse order.

- `[r:]default`,`Default`
- `[r:]rainbow`
- `[r:]Rainbow`
- `[r:]rainbow2`
- `[r:]Rainbow2`
- `[r:]spectrum`
- `[r:]Spectrum`
- `[r:]set2`
- `[r:]tab10`
- `[r:]brewer`
- `[r:]excel`
- `[r:]reds`
- `[r:]Reds`
- `[r:]greens`
- `[r:]Greens`
- `[r:]blues`
- `[r:]Blues`
- `[r:]purples`
- `[r:]Purples`
- `[r:]darkpurples`
- `[r:]navy`
- `[r:]br`
- `[r:]br2`

### rgb888 color (TrueColor)

Decimal triplet from 0 to 255: `[0-9]{1,3}/[0-9]{1,3}/[0-9]{1,3}` (ex: `123/213/42`).

Hexadecimal triplet: `#[0-9a-fA-F]{6}` (ex: `#22fa44`).

### rgb444 color (TrueColor)

Hexadecimal triplet: `#[0-9a-fA-F]{3}` (ex: `#ae3`).

### 256 colors

A number from 0 to 255.

### ANSI/Escape color

`e` followed by a number from 0 to 15 (ex: `e3`).

### background prefix

`bg=` followed by a color.


## Example:

- `echo abcdefgh | ./colout '(..)..(..)' red,bg=#8f10e5,italic bg=yellow,k`
- `ls -shS ~/Videos | ./colout -Nu KMG -y 'log(v*(1024**(u-1)))*2*n/MAX-n' -v 'MAX=log(15000000)' '[0-9]+\.?,?[0-9]*.' rainbow2`
- `ls -shS ~/Videos | ./colout -Ns 1000 -u KMG '[0-9]+\.?,?[0-9]*.' rainbow`
- `ls -l | ./colout '(.)(.)(.)(.)(.)(.)(.)(.)(.)(.)' B {r,g,y},{,i,o}`
- `ls -l | colout -t perm`
- `echo $LS_COLORS | ./colout -r '([^=]+)=([^:]+):?'`
- `echo $PATH | ./colout -rn '([^:]+):'`
- `env | ./colout '^([^=]+)=' y -- -cr '^([^=]+)=([^:]+):?' -- -cr '^([^:]+)(:)' b bg=Y -- -c '/.*' b -- -c '.*' g`
- `echo 'Progress [########################] 100%' | ./colout -rn '#' hidden,bg=Rainbow2`
- `echo ' ab "abc\\tde\\"fg\\""hi' | ./colout -aC '"' r,o -- -cr '\\.' y,o -- -ec '"' r,o`


<!-- links -->
<!-- [version.badge]: https://badge.fury.io/gh/jonathanpoelen%2Fcolout.svg -->
