CXXFLAGS =	-O0 -g3 -Wall -fmessage-length=0 -std=c++0x -I./include

OBJS =		test.o MIMO_FUN_Edge.o MIMO_FUN_Terminal.o MIMO_FUN_Backbone.o\
			MimoFun.o PracticalSocket.o ConfigRead.o MimoFunTransmit.o

LIBS =		-L./build -lkodoc_static -lfifi -lcpuid

TARGET =	test MIMO_FUN_Edge MIMO_FUN_Terminal MIMO_FUN_Backbone

.PHONY: all $(TARGET)

all:	$(TARGET)

test: $(OBJS)
	$(CXX) -o test test.o MimoFun.o PracticalSocket.o ConfigRead.o MimoFunTransmit.o $(LIBS)

MIMO_FUN_Edge:	$(OBJS)
	$(CXX) -o MIMO_FUN_Edge MIMO_FUN_Edge.o MimoFun.o PracticalSocket.o ConfigRead.o MimoFunTransmit.o $(LIBS)
	
MIMO_FUN_Terminal:	$(OBJS)
	$(CXX) -o MIMO_FUN_Terminal MIMO_FUN_Terminal.o MimoFun.o PracticalSocket.o ConfigRead.o MimoFunTransmit.o $(LIBS)

MIMO_FUN_Backbone:	$(OBJS)
	$(CXX) -o MIMO_FUN_Backbone MIMO_FUN_Backbone.o MimoFun.o PracticalSocket.o ConfigRead.o MimoFunTransmit.o $(LIBS)

clean:
	rm -f $(OBJS) $(TARGET) 
