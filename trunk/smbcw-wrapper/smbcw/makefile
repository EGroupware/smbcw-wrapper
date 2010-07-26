smbcw: smbcw.c smbcw.h
	gcc -c -fPIC smbcw.c -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE
	gcc -shared -o libsmbcw.so smbcw.o -lsmbclient
	strip libsmbcw.so

install:
	cp libsmbcw.so /usr/lib/
	cp smbcw.h /usr/include/

uninstall:
	rm /usr/lib/libsmbcw.so -f
	rm /usr/include/smbcw.h -f

clean:
	rm *.o -f
	rm *.so -f

