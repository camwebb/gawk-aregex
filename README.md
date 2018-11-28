# gawk-aregex

Gawk extension for approximate regex (fuzzy) matching, using the TRE
regex library from the [TRE](https://laurikari.net/tre/) library (also [here](https://github.com/laurikari/tre)).

Provides an `amatch()` function, roughly equivalent to the built-in
`match()` function in gawk. For documentation of this function and
example usage, please see the [man](doc/aregex.md) page.

As of 2018-11-25, this gawk extension is a stand-alone release. It may
later be incorporated into combined
[Gawkextlib](http://gawkextlib.sourceforge.net/) project, and appear
[here](https://sourceforge.net/projects/gawkextlib/files/).

## Installation

 * (Install gawk, version 4.2+)
 * Install [gawkextlib](http://gawkextlib.sourceforge.net/) ([AUR](https://aur.archlinux.org/packages/gawkextlib/), [Fedora](https://apps.fedoraproject.org/packages/gawkextlib-devel)). Tested with version 1.0.4.
 * Install [TRE](https://laurikari.net/tre/) ([Arch](https://www.archlinux.org/packages/community/x86_64/tre/), [Fedora](https://apps.fedoraproject.org/packages/tre-devel)). Tested with version 0.8.0.
 * Make sure the libraries (`libgawkextlib.so` and `libtre.so`) and
   header files (`gawkapi.h` and `tre/tre.h`) can be found by the
   compiler. Add `-Ldir` and `Idir` arguments to `gcc` if needed.
 * Compile with `make`
 * Test with `make check`
 * Set install location `PREFIX` in `Makefile`. Install with `make install`

### Alternative installation

The file `aregex.c` can be easily incorporated into the standard
gawkextlib build chain:

      git clone git://git.code.sf.net/p/gawkextlib/code gawkextlib-code
      cd gawkextlib-code
      ./make_extension_directory.sh -g /.../local/bin/ -l /.../local/lib/ \
        -I aregex "Name" "email"
      cd aregex
      cp -f .../aregex.c .
      sed -i '7 i \#include "common.h"' aregex.c 
      ./configure # --prefix=/.../local/ 
      sed -i 's/-lgawkextlib/-lgawkextlib -ltre/g' Makefile
      make
      make install

## A note on bytes and characters

While the `amatch()` function is roughly equivalent to the gawk
`match()` function, I chose not to return `[i,"start"]` position and
`[i,"length"]` in the returned substring array (e.g., see [here](https://github.com/camwebb/gawk-aregex/blob/115e400dbe6446d3138c7da2fba6d461026cad8d/aregex.c)),
but to return just the literal substring for each parenthetical
match. Gawk is multibyte aware, and `match()` works in terms of
characters, not bytes, but TRE seems not to be character-based. Using
the `wchar_t` versions of `tre_regcomp()` and `tre_regaexec()` does
not help if the input is a mix of single and multi-byte characters.

A simple routine must be used on the output array (`out`), if
positions and lengths of the substrings are needed:

      print "i", "substring", "posn", "length"
      p = 1
      for (i = 1; i < length(out); i++) {
        idx = index(substr(str, p), out[i])
        len = length(out[i])
        print i, out[i], idx+p-1, len
        p = p + idx + len
      }

## Thanks to...

 * Ville Laurikari (@laurikari) for TRE
 * Arnold Robbins for maintaining Gawk
 * `gawkextlib` developers, and the developers of other extensions for
   their examples
 * Benjamin Eckel (@bhelx) for [this gist](https://gist.github.com/bhelx/1498622/e442385814e64625cc286ad3fa4ef9c0517ce785).
 * user sashoalm on StackOverflow for [this answer](https://stackoverflow.com/a/35200252/563709).
 * User Stefan on StackOverflow for [this answer](https://stackoverflow.com/a/17764716/563709).

----

Cam Webb <cw@camwebb.info>, 2018-11-24
