# Colout -- Add colors to a text stream in your terminal

Inspired by http://nojhan.github.com/colout/


```
./colout -h
./colout -l [COLORS_AND_STYLES...]
./colout [-rcanp] [-s scale] [-u units] [-x awk_expr] [-X awk_expr] [-f awk_func] PATTERN [COLORS_AND_STYLES...] [-- ...]

 -l  (line) Colors the lines

 -r  (repeat/again) Apply PATTERN until there is no more matching.

 -a  (all) Loop on all PATTERN until there is no more matching.

 -c  (continue) Continue on the next PATTERN even if there was a match.

 -n  (next) Each match passes to the next color.

 -p  (print) print awk/sed command.

 -s scale  'min,max' or 'max'. Apply colors linearly between min and max (0,100 by default).

 -u units  Cuts the color range per unit.

 -x awk_expr  An expression that returns the color index.
              'v' the extracted value
              'n' the number of color
              'u' (only if -u) the unit position (1 is first unit, 0 is an unrecognized unit).

 -X awk_expr  An expression that returns a numeric value between min and max (see '-s scale').
              'v' the extracted value
              'n' the number of color
              'u' (only if -u) the unit position (1 is first unit, 0 is an unrecognized unit).

 -f awk_func  Awk user functions.


 styles:
   normal,N
   bold,B
   d[im]
   i[talic]
   u[nderline]
   blink,K
   reverse,R
   [h]idden

 named colors (Note: an uppercase character is considered to be bold)
   default,none
   [blac]k
   w[hite]
   r[ed]
   g[reen]
   b[lue]
   y[ellow]
   m[agenta]
   c[yan]
   gray,a
   d[ark_]gray,da
   l[ight_]red,lr
   l[ight_]green,lg
   l[ight_]blue,lb
   l[ight_]yellow,ly
   l[ight_]magenta,lm
   l[ight_]cyan,lc

 color map:
   default,Default
   rainbow
   Rainbow
   spectrum
   Spectrum

 rgb888 color:
   Hexadecimal triplet: #[0-9a-fA-F]{6} (ex: '#22fa44')

 rgb444 color:
   Hexadecimal triplet: #[0-9a-fA-F]{3}, (ex: '#ae3')

 256 colors:
   {0..255}

 ANSI/Escape color:
   e{0..15}

 background prefix:
   bg=

 example:
   echo abcdefgh | ./colout '(..)..(..)' red,bg=#8f10e5,italic bg=yellow,k
   ls -shS ~/Videos | ./colout -x 'log(v*(1000^u))' -u KMG '[0-9]+\.?[0-9]*.' Rainbow
   ls -shS ~/Videos | ./colout -s 2000 -u KMG '[0-9]+\.?[0-9]*.' rainbow
   ls -l C | ./colout '(.)(.)(.)(.)(.)(.)(.)(.)(.)(.)' B,b {r,g,y},{,i,B}
   echo $LS_COLORS | ./colout -r '([^=]+)=([^:]+:)'
   echo $PATH | ./colout -rn '([^:]+):'
   echo 'Progress [########################] 100%' | ./colout -rn '#' hidden,bg=Rainbow
```
