all:
	gcc -Wall -o whatsup whatsup.c hostlist.c nodeupdown.c -lgenders -lganglia
debug:
	gcc -Wall -DWHATSUP_DEBUG -o whatsup whatsup.c hostlist.c nodeupdown.c -lgenders -lganglia
clean:
	rm whatsup
