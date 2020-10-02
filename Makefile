PREFIX = /usr/local
BASE = aregex
SHLIBEXT = .so

$(BASE)$(SHLIBEXT): $(BASE).c
	gcc $(CFLAGS) -shared -Wall -Wextra -Wno-unused-parameter -Wmissing-prototypes -Wpointer-arith -Wcast-qual -Wwrite-strings -Wshadow -g -O2 -fPIC -o $@ $< -lgawkextlib -ltre

check: $(BASE)$(SHLIBEXT) test/$(BASE).awk test/$(BASE).ok
	@gawk -f test/$(BASE).awk &> test/$(BASE).tmp 
	@bash -c "if [ -z `diff test/$(BASE).ok test/$(BASE).tmp` ] ; then echo '** PASS **'; else echo '** FAIL **' ; fi "
	@rm -f test/$(BASE).tmp

install: $(BASE)$(SHLIBEXT) doc/$(BASE).3am
	mkdir -p $(PREFIX)/lib/gawk
	cp -f $(BASE)$(SHLIBEXT) $(PREFIX)/lib/gawk/.
	mkdir -p $(PREFIX)/share/man/man3
	cp -f doc/$(BASE).3am $(PREFIX)/share/man/man3/.

man : doc/$(BASE).3am
doc/$(BASE).3am: doc/$(BASE).md
	which pandoc >/dev/null 2>&1 && pandoc -s -t man -o $@ $< || touch $@
