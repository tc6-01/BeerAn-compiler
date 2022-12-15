build: lexer.l  parser.y
	bison -d parser.y
	flex lexer.l
	gcc -o test lex.yy.c parser.tab.c
	./test lex.c > output
clean:
	rm parser.tab.c parser.tab.h lex.yy.c
	rm Symbol.out output
	rm test