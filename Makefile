BINDIR	= ./
DBG	= $(CFLAGS) -g3 -D__DEBUG__
REL	= $(CFLAGS) -O2
BIN	= primer
CC	= gcc
CFLAGS	= -std=gnu99

main: utils parser lexer eval
	$(CC) $(DBG) $(CFLAGS) -c y.tab.c lex.yy.c
	$(CC) $(DBG) *.o -o $(BINDIR)$(BIN)

eval: eval.c eval.h
	$(CC) $(DBG) $(CFLAGS) -c eval.c

lexer: parser
	flex lexer.l

parser:
	bison -y -d parser.y

utils: utils.c utils.h
	$(CC) $(DBG) $(CFLAGS) -c utils.c

docs: README.md
	pandoc README.md -o index.html -c doc.css

clean:
	rm lex.yy.c y.tab.c y.tab.h *.o primer

cleanwin:
	del lex.yy.c y.tab.c y.tab.h *.o primer.exe