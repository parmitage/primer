interpreter = pri
compiler = prc
bin_i = 
bin_c = 
delete = rm

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

debug:
	ocamlc -c utils.ml
	ocamlc -c runtime.ml
	ocamlyacc parser.mly
	ocamlc -c parser.mli
	ocamlc -c parser.ml
	ocamllex lexer.mll
	ocamlc -c lexer.ml
	ocamlc -c interpreter.ml
	ocamlc -c compiler.ml
	ocamlc -o $(bin_i) str.cma utils.cmo runtime.cmo parser.cmo lexer.cmo interpreter.cmo
	ocamlc -o $(bin_c) str.cma utils.cmo runtime.cmo parser.cmo lexer.cmo compiler.cmo

release:
	ocamlopt -c utils.ml
	ocamlopt -c runtime.ml
	ocamlyacc parser.mly
	ocamlopt -c parser.mli
	ocamlopt -c parser.ml
	ocamllex lexer.mll
	ocamlopt -c lexer.ml
	ocamlopt -c eval.ml
	ocamlopt -c compiler.ml
	ocamlopt -o $(bin_i) str.cmxa utils.ml runtime.ml parser.ml lexer.ml interpreter.ml
	ocamlopt -o $(bin_c) str.cmxa utils.ml runtime.ml parser.ml lexer.ml compiler.ml

clean:
	$(delete) $(bin_i) $(bin_c) lexer.ml parser.ml parser.mli *.cmo *.cmi *.o *.cmx

docs:
	pandoc readme.md -o index.html -c doc.css

examples:
	$(bin_c) ../examples/dictionary.pri ../examples/dictionary.html
	$(bin_c) ../examples/euler.pri ../examples/euler.html
	$(bin_c) ../examples/factorial.pri ../examples/factorial.html
	$(bin_c) ../examples/fizzbuzz.pri ../examples/fizzbuzz.html
	$(bin_c) ../examples/knapsack.pri ../examples/knapsack.html
	$(bin_c) ../examples/object.pri ../examples/object.html
	$(bin_c) ../examples/rover.pri ../examples/rover.html
	$(bin_c) ../examples/search.pri ../examples/search.html