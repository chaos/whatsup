all:
	# to compile and debug nodeupdown.c
	cp ../src/libnodeupdown/nodeupdown* .
	cp ../src/libnodeupdown/hostlist* .
	gcc -g -Wall -o libnodeupdown_test libnodeupdown_test.c nodeupdown.c hostlist.c -lgenders -lganglia


	# to test the real way
	# gcc -g -Wall -I../src/libnodeupdown/ -o libnodeupdown_test libnodeupdown_test.c -lnodeupdown

