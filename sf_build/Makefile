all: ../aregex.c ../doc/aregex.3am ../test/aregex.awk ../test/aregex.ok
	git clone git://git.code.sf.net/p/gawkextlib/code gawkextlib-code
	# cd gawkextlib-code && ./make_extension_directory.sh -g /.../local/bin/ \
	#   -l /.../local/lib/ -I aregex "Cam Webb" "cw@camwebb.info"
	cd gawkextlib-code && ./make_extension_directory.sh \
	   -I aregex "Cam Webb" "cw@camwebb.info"
	cp -f ../aregex.c        gawkextlib-code/aregex
	cp -f ../doc/aregex.3am  gawkextlib-code/aregex/doc
	cp -f ../test/aregex.awk gawkextlib-code/aregex/test
	sed -i 's|./aregex.so|../.libs/aregex.so|g' \
	   gawkextlib-code/aregex/test/aregex.awk
	# Note: the gawkextlib test suite uses gawk with --characters-as-bytes
	cp -f ../test/aregex_b.ok  gawkextlib-code/aregex/test/aregex.ok
	sed -i '7 i \#include "common.h"' gawkextlib-code/aregex/aregex.c
	# cd gawkextlib-code/aregex && ./configure --prefix=/.../local/
	cd gawkextlib-code/aregex && ./configure
	sed -i 's/-lgawkextlib/-lgawkextlib -ltre/g' \
	   gawkextlib-code/aregex/Makefile.am
	sed -i 's/-lgawkextlib/-lgawkextlib -ltre/g' gawkextlib-code/aregex/Makefile
	cd gawkextlib-code/aregex && make
	cd gawkextlib-code/aregex && make check
	# cd gawkextlib-code/aregex && make install

clean:
	rm -rf gawkextlib-code
