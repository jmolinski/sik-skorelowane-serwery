CXX=g++
CPPFLAGS= -std=c++17 -O2
LDFLAGS= -lstdc++fs

SRCS=err.cpp filesystem_interactions.cpp http.cpp input_parsing.cpp tcp_server.cpp server.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: serwer

serwer: $(OBJS)
	$(CXX) -o serwer $(OBJS) $(LDFLAGS)

depend: .depend

.depend: $(SRCS)
	rm -rf ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	rm -rf $(OBJS) .depend serwer

include .depend
