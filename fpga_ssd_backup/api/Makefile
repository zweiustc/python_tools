libssdapi.a:ssd_api.o
	rm -rf output
	mkdir output
	mkdir output/lib
	mkdir output/include
	ar crv libssdapi.a ssd_api.o
	cp libssdapi.a output/lib
	cp ssd_api.h output/include
	cp ../../crc32c-ds/bhp-lib.h output/include

ssd_api.o:ssd_api.c ssd_api.h
	gcc -I../../crc32c-ds -c ssd_api.c


clean:
	rm -rf output
	rm -f *.o *.a
