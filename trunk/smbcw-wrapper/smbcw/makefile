
LIBDIR ?= /usr/lib/
DESTDIR ?= 

INSTALL = /usr/bin/install -D

smbcw: smbcw.c smbcw.h
	gcc -c -fPIC smbcw.c -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE
	gcc -shared -o libsmbcw.so smbcw.o -lsmbclient
	strip libsmbcw.so

install:
	$(INSTALL) libsmbcw.so $(LIBDIR)/libsmbcw.so
	$(INSTALL) smbcw.h $(DESTDIR)/usr/include/smbcw.h

uninstall:
	rm /usr/lib/libsmbcw.so -f
	rm /usr/include/smbcw.h -f

clean:
	rm *.o -f
	rm *.so -f

