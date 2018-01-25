
all= MM

MM: mm.c
	@rm -fr mm.o mm.0 mm.2
	gcc mm.c -c -g
	objdump -dS mm.o > mm.0
	gcc mm.c -c -g -O2
	objdump -dS mm.o > mm.2

