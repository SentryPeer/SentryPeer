CC=gcc
CFLAGS=-Wall

all: pcre2demo udp_client tcp_client

pcre2demo: pcre2demo.o
	$(CC) $(CFLAGS) -o pcre2demo pcre2demo.o -lpcre2-8

udp_client: udp_client.o
	$(CC) $(CFLAGS) -o udp_client udp_client.o

tcp_client: tcp_client.o
	$(CC) $(CFLAGS) -o tcp_client tcp_client.c

clean:
	rm -f pcre2demo udp_client
