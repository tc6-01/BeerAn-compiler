all: BeerAn

OBJS = BeerAnSytnax.o \
		BeerAnLex.o  \
		CodeGen.o \
		main.o	 \
		TypeSystem.o\

LLVMCONFIG = llvm-config
CPPFLAGS = `$(LLVMCONFIG) --cppflags` -std=c++14
LDFLAGS = `$(LLVMCONFIG) --ldflags` -lpthread -ldl -rdynamic -L/usr/local/lib
LIBS = `$(LLVMCONFIG) --libs`

clean:
	$(RM) -rf grammar.cpp grammar.hpp compiler  tokens.cpp  $(OBJS)


CodeGen.cpp: CodeGen.h ASTNodes.h

BeerAnSytnax.cpp: BeerAn.y
	bison -d -o $@ $<

BeerAnSytnax.hpp: BeerAnSytnax.cpp

BeerAnLex.cpp: BeerAn.l BeerAnSytnax.hpp
	flex -o $@ $<

%.o: %.cpp
	g++ -c $(CPPFLAGS) -o $@ $<

BeerAn: $(OBJS)
	g++ $(CPPFLAGS) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)
