all: BeerAn

OBJS = BeerAnSytnax.o \
		BeerAnLex.o  \
		CodeGen.o \
		main.o	 \

LLVMCONFIG = llvm-config
CPPFLAGS = `$(LLVMCONFIG) --cppflags` -std=c++14
LDFLAGS = `$(LLVMCONFIG) --ldflags` -lpthread -ldl -rdynamic -L/usr/local/lib
LIBS = `$(LLVMCONFIG) --libs`

clean:
	$(RM) -rf  BeerAn.txt BeerAn 

BeerAnSytnax.cpp: BeerAn.y
	bison -d -o $@ $<

BeerAnLex.cpp: BeerAn.l BeerAnSytnax.hpp
	flex -o $@ $<

%.o: %.cpp
	g++ -c $(CPPFLAGS) -o $@ $<

BeerAn: $(OBJS)
	g++ $(CPPFLAGS) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)
	$(RM) -rf BeerAnSytnax.cpp BeerAnLex.cpp *.hpp $(OBJS)
