NAME = "crpc"
PREFIX ?= /usr/local
MANDIR ?= $(PREFIX)/share/man
CFLAGS = -Wall -g
CC = g++

default: crps crpc

crps: crps.cc common.h crps.h
	$(CC) -o crps crps.cc common.h crps.h -lpthread
crpc: crpc.cc common.h
	$(CC) -o crpc crpc.cc common.h -lpthread

install: 
	@cp -p crpc $(DESTDIR)$(PREFIX)/bin/crpc
	@chmod a+x $(DESTDIR)$(PREFIX)/bin/crpc
	@echo "[OK] $(NAME) installed."

uninstall:
	@rm $(DESTDIR)$(PREFIX)/bin/crpc
	@echo "[OK] $(NAME) uninstalled."


clean:
	rm -rf *.o crps crpc

