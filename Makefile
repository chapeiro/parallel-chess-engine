# DEBUG=1
# PROFILE=1

DEBUGFLAGS= -g3 -pg 
TFLAGS= -O3
TFLAGS+= -finline 
TFLAGS+= -march=native
TFLAGS+= -fmerge-all-constants
TFLAGS+= -fmodulo-sched
TFLAGS+= -fmodulo-sched-allow-regmoves
#TFLAGS+= -funsafe-loop-optimizations
#TFLAGS+= -Wunsafe-loop-optimizations
TFLAGS+= -fsched-pressure
TFLAGS+= -fipa-pta
TFLAGS+= -fipa-matrix-reorg
TFLAGS+= -ftree-loop-distribution
TFLAGS+= -ftracer
TFLAGS+= -funroll-loops
TFLAGS+= -fwhole-program
TFLAGS+= -flto

DEBUGFLAGS+=$(TFLAGS)
OPTFLAGS= $(TFLAGS)
OPTFLAGS+= -DNDEBUG

INCLUDE_PATH=-I. 

CXXFLAGS= -Wl,--no-as-needed -lpthread -pthread -std=c++11 -Wall -D_GNU_SOURCE 

CXXFLAGS+= $(INCLUDE_PATH)

# ifeq ($(PROFILE),1)
# PROFFLAGS= -g -pg 
# PLFLAGS= -g -pg
# else
# PROFFLAGS= 
# PLFLAGS=
# endif

# CPP=g++

# LDFLAGS= $(PLFLAGS)  
# LIBS=

# CPP_PROG= sysexplorer.cpp
# C_PROG= tests.c
# PROG=$(C_PROG:.c=) $(CPP_PROG:.cpp=)

# CPP_SRC= file_system.cpp sysexplorer_server.cpp socket_utils.cpp my_strutils.cpp
# C_SRC= linkedlist.c strutils.c hashtable.c urldecode.c
# C_OBJ=$(C_SRC:.c=.o) $(CPP_SRC:.cpp=.o)

# CPP_SOURCES= $(CPP_PROG) $(CPP_SRC)
# C_SOURCES= $(C_PROG) $(C_SRC)
# C_OBJECTS=$(C_SOURCES:.c=.o) $(CPP_SOURCES:.cpp=.o)

#SRC_DIR=src
SRC_DIR=source
OBJ_DIR=obj
DST_DIR=build
BLD_DIR:=$(DST_DIR)
# DBG_DIR=$(BLD_DIR)/debug
# DOJ_DIR=$(OBJ_DIR)/debug

DBG_SUB=debug
DEF_SUB=release

ifeq ($(DEBUG),1)
	CXXFLAGS+= $(DEBUGFLAGS) $(PROFFLAGS)
	BLD_DIR:=$(BLD_DIR)/$(DBG_SUB)
	OBJ_DIR:=$(OBJ_DIR)/$(DBG_SUB)
else
	CXXFLAGS+= $(OPTFLAGS) $(PROFFLAGS)
	BLD_DIR:=$(BLD_DIR)/$(DEF_SUB)
	OBJ_DIR:=$(OBJ_DIR)/$(DEF_SUB)
endif

vpath %.cpp $(SRC_DIR)
vpath %.o $(OBJ_DIR)

CXX_SOURCES= $(notdir $(wildcard $(SRC_DIR)/*.cpp))
CXX_OBJCTES= $(CXX_SOURCES:.cpp=.o)
CXX_OBJCTESD= $(addprefix $(OBJ_DIR)/, $(CXX_OBJCTES))
CXX_SOURCESD= $(wildcard $(SRC_DIR)/*.cpp)

.PHONY: all

#$(CXX) -c $(CPPFLAGS) $(CXXFLAGS)
all: cchapeiro


$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

cchapeiro: $(CXX_OBJCTESD)
	$(CXX) $^ $(CPPFLAGS) $(CXXFLAGS) -o $(BLD_DIR)/$@
	chmod +x $(BLD_DIR)/$@

realclean:
	$(RM) .depend $(CXX_OBJCTESD)
	$(RM) -r $(OBJ_DIR) $(BLD_DIR)
	touch .depend
	-mkdir -p $(OBJ_DIR) $(BLD_DIR) $(DST_DIR)/$(DBG_SUB) $(DST_DIR)/$(DEF_SUB)

depend: $(CPP_SOURCES)
	$(CXX) $(CXXFLAGS) -MM $(CXX_SOURCESD) > .depend

clean: realclean depend

-include .depend

