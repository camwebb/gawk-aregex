aregex.so: aregex.c
	gcc -shared -lgawkextlib -ltre -Wall -g -O2 -fPIC -o aregex.so aregex.c
