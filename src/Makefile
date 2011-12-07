interpreter = pri
compiler    = prc
bin_i       = 
bin_c       = 
delete      = rm
DBG		    = $(CFLAGS) -g3 -D__DEBUG__
REL		    = $(CFLAGS) -O2
CC		    = gcc
CFLAGS	    = -std=gnu99

uname_S := $(shell uname -s)
uname_M := $(shell uname -m)
uname_O := $(shell uname -o)
uname_R := $(shell uname -r)
uname_P := $(shell uname -p)
uname_V := $(shell uname -v)

ifeq ($(uname_S), Linux)
	bin_i = ./$(interpreter)
	bin_c = ./$(compiler)
endif

ifeq ($(uname_S), Darwin)
	bin_i = ./$(interpreter)
	bin_c = ./$(compiler)
endif

ifeq ($(uname_O), Cygwin)
	bin_i = $(interpreter).exe
	bin_c = $(compiler).exe
	delete = del
endif

ifeq ($(uname_O), MinGW)
	bin_i = $(interpreter).exe
	bin_c = $(compiler).exe
	delete = del
endif

debug: parser lexer eval
	$(CC) $(DBG) $(CFLAGS) -c y.tab.c lex.yy.c
	$(CC) $(DBG) *.o -o $(bin_i)

release: parser lexer eval
	$(CC) $(REL) $(CFLAGS) -c y.tab.c lex.yy.c
	$(CC) $(REL) *.o -o $(bin_i)

eval: parser lexer gc eval.c eval.h
	$(CC) $(DBG) $(CFLAGS) -c eval.c

gc: gc.c gc.h
	$(CC) $(DBG) $(CFLAGS) -c gc.c

lexer: parser lexer.l
	flex lexer.l

parser: parser.y
	bison -y -d parser.y

docs: README.md
	pandoc README.md -o index.html -c doc.css

clean:
	delete $(bin_i) lex.yy.c y.tab.c y.tab.h *.o
	delete primer