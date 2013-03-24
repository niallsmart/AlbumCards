LIBNFC	= $(HOME)/local/packages/libnfc-1.7.0-rc5
CFLAGS	= -I$(LIBNFC)/include -L$(LIBNFC)/lib -Wl,-rpath,$(LIBNFC)/lib -lnfc -W -Wall

bin/nfc-poll: src/nfc-poll.c
	gcc $(CFLAGS) $< -o $@

clean:
	rm bin/nfc-poll

