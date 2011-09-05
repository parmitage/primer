BINDIR	= ./
DBG		= $(CFLAGS) -g3 -D__DEBUG__
REL		= $(CFLAGS) -O2
BIN		= primer
CC		= gcc
CFLAGS	= -std=gnu99

debug: parser lexer eval
	$(CC) $(DBG) $(CFLAGS) -c y.tab.c lex.yy.c
	$(CC) $(DBG) *.o -o $(BINDIR)$(BIN)

release: parser lexer eval
	$(CC) $(REL) $(CFLAGS) -c y.tab.c lex.yy.c
	$(CC) $(REL) *.o -o $(BINDIR)$(BIN)

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
	rm lex.yy.c y.tab.c y.tab.h *.o
	rm primer