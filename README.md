# Colout -- Colorize text stream in your terminal

Inspired by http://nojhan.github.com/colout/


```
./colout -l [--] [COLORS_AND_STYLES...]
./colout [-rn] [--] PATTERN [COLORS_AND_STYLES...] [-- [-rn] [--] PATTERN [COLORS_AND_STYLES...]]...

 -l  Colorize by lines.

 -r  Apply the PATTERN on the line until it is no longer able to.

 -n  Switches to the next color for each match. Implies -r.


 styles:
   normal,N
   bold,B
   d[im]
   i[talic]
   u[nderline]
   blink,K
   reverse,R
   [h]idden

 named colors:
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

 rgb888 color:
  Hexadecimal triplet: #[0-9a-fA-F]{6} (ex: '#22fa44')

 rgb444 color:
  Hexadecimal triplet: #[0-9a-fA-F]{3}, (ex: '#ae3')

 256 colors: {0..255}

 ANSI/Escape color:
  e{0..255}

 background prefix:
  bg=

 example:
  colout '(..)..(..)' red,bg=#8f10e5,italic bg=yellow,k <<<abcdefgh
  ls -l | colout '^(.)(.)(.)(.)(.)(.)(.)(.)(.)(.)' b g g,i g,B y y,i y,B r r,i r,B
```
