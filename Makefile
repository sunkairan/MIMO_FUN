CXXFLAGS =	-O0 -g3 -Wall -fmessage-length=0 -std=c++0x -I./include
OBJ = MimoFun.o PracticalSocket.o ConfigRead.o MimoFunTransmit.o
LIBS = -L./build -lkodoc_static -lfifi -lcpuid
TARGETS =	test MIMO_FUN_Edge MIMO_FUN_Terminal MIMO_FUN_Backbone

#ObjectDir=obj/
#SourceDir=src/
#BinDir=bin/
OBJS =   $(addprefix $(ObjectDir),$(OBJ))
TARGET = $(addprefix $(BinDir),$(TARGETS))
TARGETOBJ =	$(addsuffix .o,$(TARGET))

.PHONY: all $(TARGET)

all:	$(TARGET)

$(TARGET):	$(OBJS) $(TARGETOBJ)
	$(CXX) -o $@ $@.o $(OBJS) $(LIBS)
#$(ObjectDir)%.o: $(SourceDir)%.cpp
#	$(CXX) $< -o $@ $(LIBS)

clean:
	rm -f $(OBJS) $(TARGET) $(TARGETOBJ)