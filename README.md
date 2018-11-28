# gawk-aregex

Gawk extension for approximate regex (fuzzy) matching, using the TRE
regex library from the [TRE](https://laurikari.net/tre/) library.

Provides an `aregex()` function in gawk. For documentation of this
function and example usage, please see the [man](doc/aregex.3am) page.

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
 * Set `PREFIX` in `Makefile`. Install with `make install`

### Alternative installation

The file `aregex.c` can be easily incorporated into the standard
gawkextlib build chain:

      git clone git://git.code.sf.net/p/gawkextlib/code gawkextlib-code
      cd gawkextlib-code
      ./make_extension_directory.sh -g /.../local/bin/ -l /.../local/lib/ \
        -I aregex "Name" "email"
      cd aregex
      cp -f .../aregex.c .
      # edit aregex.c, adding “#include "common.h"” at L.7
      # edit Makefile: “aregex_la_LIBADD = -lgawkextlib -ltre $(LTLIBINTL)”
      ./configure # --prefix=/.../local/ 
      make
      make install

 * Program notes: 1. While the amatch() function is roughly equivalent
 * to the gawk match() function, I chose not to return [i,"start"]
 * position and [i,"length"] in the returned substring array, but to
 * return just the literal substring for each parenthetical
 * match. Gawk is multibyte aware, and match() works in terms of
 * characters, not bytes, but TRE is not character-based. Using the
 * 'wchar_t' versions of tre_regcomp and tre_regaexec does not help if
 * the input is a mix of single and multi-byte characters. A simple
 * routine must be used on the output array, if positions and lengths
 * of the substrings are needed.

See: https://github.com/camwebb/gawk-aregex/blob/115e400dbe6446d3138c7da2fba6d461026cad8d/aregex.c

----

Cam Webb <cw@camwebb.info>, 2018-11-24
