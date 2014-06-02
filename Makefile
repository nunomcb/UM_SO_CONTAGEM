TYPE = final

debug: TYPE += debug 
debug: all

all:
	cd client_src; make $(TYPE)
	cd server_src; make $(TYPE)
	mv client_src/client client
	mv server_src/server server

server:
	cd server_src; make
	mv server_src/server server

client:
	cd client_src; make
	mv client_src/client client

clean:
	rm client
	rm server
	rm *.txt
