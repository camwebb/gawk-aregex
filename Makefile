PREFIX = /usr/local
BASE = aregex

$(BASE).so: $(BASE).c
	gcc -shared -lgawkextlib -ltre -Wall -Wextra -Wno-unused-parameter -Wmissing-prototypes -Wpointer-arith -Wcast-qual -Wwrite-strings -Wshadow -g -O2 -fPIC -o $(BASE).so $(BASE).c

check: $(BASE).so test/$(BASE).awk test/$(BASE).ok
	gawk -f test/$(BASE).awk > test/$(BASE).tmp
	bash -c "if [ -z `diff test/$(BASE).ok test/$(BASE).tmp` ] ; then echo '** PASS **'; else echo '** FAIL **' ; fi "
	rm -f test/$(BASE).tmp

install: $(BASE).so doc/$(BASE).3am
	mkdir -p $(PREFIX)/lib/gawk
	cp -f $(BASE).so $(PREFIX)/lib/gawk/.
	mkdir -p $(PREFIX)/share/man/man3
	cp -f doc/$(BASE).3am $(PREFIX)/share/man/man3/.

man: doc/$(BASE).md
	pandoc -s -t man -o doc/$(BASE).3am doc/$(BASE).md
