CFLAGS=-Wall -Werror -fPIC -fPIE -Os -I$(shell pwd) -L$(shell pwd)

.PHONY: all clean

all: tools/fsk_demod tools/fsk_mod tools/fsk_get_test_bits tools/fsk_put_test_bits libfsk.a

clean:
	rm -f tools/fsk_demod tools/fsk_mod tools/fsk_get_test_bits tools/fsk_put_test_bits libfsk.a

tools/fsk_demod: tools/fsk_demod.c mpdecode_core.c libfsk.a
	$(CC) $(CFLAGS) -o $@ $^ -lm

tools/fsk_mod: tools/fsk_mod.c libfsk.a
	$(CC) $(CFLAGS) -o $@ $^ -lm

tools/fsk_get_test_bits: tools/fsk_get_test_bits.c
	$(CC) $(CFLAGS) -o $@ $^

tools/fsk_put_test_bits: tools/fsk_put_test_bits.c
	$(CC) $(CFLAGS) -o $@ $^

libfsk.a: fsk.c kiss_fft.c
	$(CC) $(CFLAGS) -c $^ -lm
	ar rcs $@ *.o
	rm -f *.o
