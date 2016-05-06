CXXFLAGS =	-O0 -g3 -Wall -fmessage-length=0 -std=c++0x -I./include
OBJ = MimoFun.o PracticalSocket.o ConfigRead.o MimoFunTransmit.o
LIBS = -L./build -lkodoc_static -lfifi -lcpuid
TARGETS = MIMO_FUN_Edge MIMO_FUN_Terminal MIMO_FUN_Backbone

SourceDir=src/
BinDir=bin/
OBJS =   $(addprefix $(SourceDir),$(OBJ))
TARGET = $(addprefix $(SourceDir),$(TARGETS))
TARGETOBJ =	$(addsuffix .o,$(TARGET))

.PHONY: all $(TARGET)

all:	$(TARGET)

$(TARGET):	$(OBJS) $(TARGETOBJ)
	$(CXX) -o $@ $@.o $(OBJS) $(LIBS)
	mkdir -p $(BinDir); mv $@ $(BinDir)$(@F)

clean:
	rm -f $(OBJS) $(TARGET) $(TARGETOBJ) $(BinDir)/*