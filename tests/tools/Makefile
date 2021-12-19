CC=gcc
CFLAGS=-Wall

all: pcre2demo udp_client

pcre2demo: pcre2demo.o
	$(CC) $(CFLAGS) -o pcre2demo pcre2demo.o -lpcre2-8

udp_client: udp_client.o
	$(CC) $(CFLAGS) -o udp_client udp_client.o

clean:
	rm -f pcre2demo udp_client
