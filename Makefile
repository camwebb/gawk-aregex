PREFIX = /usr/local

aregex.so: aregex.c
	gcc -shared -lgawkextlib -ltre -Wall -g -O2 -fPIC -o aregex.so aregex.c

check: aregex.so test/test_aregex.awk test/test_aregex.ok
	gawk -f test/test_aregex.awk
	echo "  Passing result:"
	cat test/test_aregex.ok

install: aregex.so doc/aregex.3am
	mkdir -p $(PREFIX)/lib/gawk
	cp -f aregex.so $(PREFIX)/lib/gawk/.
	mkdir -p $(PREFIX)/share/man/man3
	cp -f doc/aregex.3am $(PREFIX)/share/man/man3/.

