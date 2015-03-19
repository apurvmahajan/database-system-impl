CC = g++ -O2 -w -Wno-deprecated 

tag = -i

src = ./source
bin = ./bin

ifdef linux
tag = -n
endif



test2.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o GenericDBFile.o HeapDBFile.o SortedDBFile.o Pipe.o y.tab.o lex.yy.o test22.o
	$(CC) -o $(bin)/test2.out $(bin)/Record.o $(bin)/Comparison.o $(bin)/ComparisonEngine.o $(bin)/Schema.o $(bin)/File.o $(bin)/BigQ.o $(bin)/DBFile.o $(bin)/GenericDBFile.o $(bin)/HeapDBFile.o $(bin)/SortedDBFile.o $(bin)/Pipe.o $(bin)/y.tab.o $(bin)/lex.yy.o $(bin)/test22.o -lfl -lpthread

test1.out: Record.o Comparison.o ComparisonEngine.o Schema.o File.o Pipe.o BigQ.o DBFile.o GenericDBFile.o HeapDBFile.o SortedDBFile.o y.tab.o lex.yy.o test21.o
	$(CC) -o $(bin)/test1.out $(bin)/Record.o $(bin)/Comparison.o $(bin)/ComparisonEngine.o $(bin)/Schema.o $(bin)/File.o $(bin)/BigQ.o $(bin)/DBFile.o $(bin)/GenericDBFile.o $(bin)/HeapDBFile.o $(bin)/SortedDBFile.o $(bin)/y.tab.o $(bin)/lex.yy.o $(bin)/Pipe.o  $(bin)/test21.o -lfl -lpthread


test22.o: $(src)/test22.cc
	$(CC) -o $(bin)/test22.o -g -c $(src)/test22.cc

test21.o: $(src)/test21.cc
	$(CC) -o $(bin)/test21.o -g -c $(src)/test21.cc
	
Comparison.o: $(src)/Comparison.cc
	$(CC) -o $(bin)/Comparison.o -g -c $(src)/Comparison.cc
	
ComparisonEngine.o: $(src)/ComparisonEngine.cc
	$(CC) -o $(bin)/ComparisonEngine.o -g -c $(src)/ComparisonEngine.cc

Pipe.o: $(src)/Pipe.cc
	$(CC) -o $(bin)/Pipe.o -g -c $(src)/Pipe.cc

BigQ.o: $(src)/BigQ.cc
	$(CC) -o $(bin)/BigQ.o -g -c -std=gnu++11 $(src)/BigQ.cc

SortedDBFile.o: $(src)/SortedDBFile.cc
	$(CC) -o $(bin)/SortedDBFile.o -g -c $(src)/SortedDBFile.cc

HeapDBFile.o: $(src)/HeapDBFile.cc
	$(CC) -o $(bin)/HeapDBFile.o -g -c $(src)/HeapDBFile.cc

GenericDBFile.o: $(src)/GenericDBFile.cc
	$(CC) -o $(bin)/GenericDBFile.o -g -c $(src)/GenericDBFile.cc

DBFile.o: $(src)/DBFile.cc
	$(CC) -o $(bin)/DBFile.o -g -c $(src)/DBFile.cc

File.o: $(src)/File.cc
	$(CC) -o $(bin)/File.o -g -c $(src)/File.cc

Record.o: $(src)/Record.cc
	$(CC) -o $(bin)/Record.o -g -c $(src)/Record.cc

Schema.o: $(src)/Schema.cc
	$(CC) -o $(bin)/Schema.o -g -c $(src)/Schema.cc
	
y.tab.o: $(src)/Parser.y
	yacc -b $(src)/y -d $(src)/Parser.y
	sed $(tag) $(src)/y.tab.c -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/" 
	g++ -w -o $(bin)/y.tab.o -c $(src)/y.tab.c

lex.yy.o: $(src)/Lexer.l
	flex  -o $(src)/lex.yy.c $(src)/Lexer.l
	gcc -w -o $(bin)/lex.yy.o -c $(src)/lex.yy.c

clean:
	rm -f $(bin)/*.exe 
	rm -f $(bin)/*.o
	rm -f $(bin)/*.out
	rm -f $(src)/y.tab.c
	rm -f $(src)/lex.yy.c
	rm -f $(src)/y.tab.h
