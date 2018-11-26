# gawk-aregex

Gawk extension for approximate regex (fuzzy) matching, using the TRE
regex library from the TRE library (https://laurikari.net/tre/).

Provides an `aregex()` function in `gawk`. For documentation of this
function and example usage, please see the [man](doc/aregex.3am) page.

To install:

      ./configure [ --prefix=/.../local/ ]
      make
      make check
      make install

To regenerate support files in this directory:

      git clone git://git.code.sf.net/p/gawkextlib/code gawkextlib-code
      cd gawkextlib-code
      ./make_extension_directory.sh -g /.../local/bin/ -l /.../local/lib/ \
        -I aregex "Name" "email"
      cd aregex
      cp -rf .../aregex.c .../test .../doc .../README.md  .

Edit `Makefile`: `aregex_la_LIBADD = -lgawkextlib -ltre $(LTLIBINTL)`

Then build and install as above.

Cam Webb
cw@camwebb.info
2018-11-24
