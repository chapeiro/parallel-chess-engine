# DEBUG=1
# PROFILE=1

CXX=mpic++

DEBUGFLAGS= -g3 -pg -ggdb 
# DEBUGFLAGS+= -funsafe-loop-optimizations
# DEBUGFLAGS+= -Wunsafe-loop-optimizations

TFLAGS= -O3 
TFLAGS+= -finline 
TFLAGS+= -funit-at-a-time
TFLAGS+= -march=native
TFLAGS+= -fmerge-all-constants
TFLAGS+= -fmodulo-sched
TFLAGS+= -fmodulo-sched-allow-regmoves
#mstackrealign
TFLAGS+= -funsafe-loop-optimizations
TFLAGS+= -Wunsafe-loop-optimizations
TFLAGS+= -fsched-pressure
TFLAGS+= -fipa-pta
TFLAGS+= -fipa-matrix-reorg
TFLAGS+= -ftree-loop-distribution
TFLAGS+= -ftracer
TFLAGS+= -funroll-loops
TFLAGS+= -fwhole-program
TFLAGS+= -flto

# DEBUGFLAGS+=$(TFLAGS)
OPTFLAGS= $(TFLAGS)
OPTFLAGS+= -DNDEBUG

INCLUDE_PATH=-I. 

CXXFLAGS= -Wl,--no-as-needed -lpthread -pthread -std=c++11 -Wall -D_GNU_SOURCE 
CXXFLAGS+= -D__STDC_FORMAT_MACROS 
CXXFLAGS+= -DMULTIPROC 
# CXXFLAGS+= -DSIMULATE_SINGLETHREAD 
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
# SRC_DIR=source
# OBJ_DIR=obj
# DST_DIR=build
# BLD_DIR:=$(DST_DIR)
# DBG_DIR=$(BLD_DIR)/debug
# DOJ_DIR=$(OBJ_DIR)/debug

# DBG_SUB=debug
# DEF_SUB=release


# vpath %.cpp $(SRC_DIR)
# vpath %.o $(OBJ_DIR)

# CXX_SOURCES= $(notdir $(wildcard $(SRC_DIR)/*.cpp))
# CXX_OBJCTES= $(CXX_SOURCES:.cpp=.o)
# CXX_OBJCTESD= $(addprefix $(OBJ_DIR)/, $(CXX_OBJCTES))
# CXX_SOURCESD= $(wildcard $(SRC_DIR)/*.cpp)

# .PHONY: all

# #$(CXX) -c $(CPPFLAGS) $(CXXFLAGS)
# all: cchapeiro


# $(OBJ_DIR)/%.o: %.cpp
# 	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

# cchapeiro: $(CXX_OBJCTESD)
# 	$(CXX) $^ $(CPPFLAGS) $(CXXFLAGS) -o $(BLD_DIR)/$@
# 	chmod +x $(BLD_DIR)/$@

# realclean:
# 	$(RM) .depend $(CXX_OBJCTESD)
# 	$(RM) -r $(OBJ_DIR) $(BLD_DIR)
# 	touch .depend
# 	-mkdir -p $(OBJ_DIR) $(BLD_DIR) $(DST_DIR)/$(DBG_SUB) $(DST_DIR)/$(DEF_SUB)

# depend: $(CPP_SOURCES)
# 	$(CXX) $(CXXFLAGS) -MM $(CXX_SOURCESD) > .depend

# clean: realclean depend

# -include .depend

DBG_DIR=debug
RLS_DIR=release

BIN_ROOT=bin
OBJ_ROOT=obj
SRC_ROOT=src
DEP_ROOT=.depend

BIN_DBG=$(BIN_ROOT)/$(DBG_DIR)/
BIN_RLS=$(BIN_ROOT)/$(RLS_DIR)/

OBJ_DBG=$(OBJ_ROOT)/$(DBG_DIR)/
OBJ_RLS=$(OBJ_ROOT)/$(RLS_DIR)/

DEP_DBG=$(DEP_ROOT)/$(DBG_DIR)/
DEP_RLS=$(DEP_ROOT)/$(RLS_DIR)/

SED_ODD=$(subst /,\/,$(OBJ_DBG))
SED_ORD=$(subst /,\/,$(OBJ_RLS))

SED_DDD=$(subst /,\/,$(DEP_DBG))
SED_DRD=$(subst /,\/,$(DEP_RLS))

CXX_SOURCESD= $(shell find $(SRC_ROOT) -name "*.cpp")
# CXX_SOURCESD= $(wildcard $(SRC_ROOT)/*.cpp)
CXX_SOURCES= $(subst $(SRC_ROOT)/,,$(CXX_SOURCESD))
CXX_OBJECTS= $(CXX_SOURCES:.cpp=.o)
# CXX_OBJCTESD= $(addprefix $(OBJ_DIR)/, $(CXX_OBJCTES))


OBJ_FILES:=$(addprefix $(OBJ_DBG), $(CXX_OBJECTS)) $(addprefix $(OBJ_RLS), $(CXX_OBJECTS))

all: debug release

debug:CXXFLAGS+= $(DEBUGFLAGS) $(PROFFLAGS)
release:CXXFLAGS+= $(OPTFLAGS) $(PROFFLAGS)

release:BIN_DIR:= $(BIN_RLS)
release:IMP_DIR:= $(RLS_DIR)
release:OBJ_DIR:= $(OBJ_RLS)
release:CXX_OBJ_D:= $(addprefix $(OBJ_RLS), $(CXX_OBJECTS))

debug:BIN_DIR:= $(BIN_DBG)
debug:IMP_DIR:= $(DBG_DIR)
debug:OBJ_DIR:= $(OBJ_DBG)
debug:CXX_OBJ_D:= $(addprefix $(OBJ_DBG), $(CXX_OBJECTS))

-include $(addprefix $(DEP_DBG), $(CXX_SOURCES:.cpp=.d))
-include $(addprefix $(DEP_RLS), $(CXX_SOURCES:.cpp=.d))

$(BIN_RLS)cchapeiro:$(addprefix $(OBJ_RLS), $(CXX_OBJECTS))
$(BIN_DBG)cchapeiro:$(addprefix $(OBJ_DBG), $(CXX_OBJECTS))

release: $(BIN_RLS)cchapeiro
debug:   $(BIN_DBG)cchapeiro

.PHONY: all debug release 

space= 
#do no remove this lines!!! needed!!!
space+= 

vpath %.o $(subst $(space),:,$(dir $(OBJ_FILES)))
vpath %.cpp $(subst $(space),:,$(dir $(CXX_SOURCESD)))

%.dir:
	mkdir -p $(@D)

%.o: 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(subst $(OBJ_DIR),$(SRC_ROOT)/,$(@:.o=.cpp)) -o $@

%cchapeiro:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^

clean:
	-rm -r $(OBJ_ROOT) $(BIN_ROOT) $(DEP_ROOT)
	mkdir -p $(BIN_DBG) $(BIN_RLS) $(OBJ_DBG) $(OBJ_RLS) $(DEP_DBG) $(DEP_RLS)
	# mkdir -p $(subst //,/,$(dir $(OBJ_FILES)))

$(DEP_DBG)%.d: %.cpp Makefile
	@mkdir -p $(@D)
	$(CXX) -MM $(CPPFLAGS) $(CXXFLAGS) $< | sed -r 's/^(\S+).(\S+):/$(SED_ODD)$(subst /,\/,$(subst $(SRC_ROOT)/,,$(<:.cpp=.o))) $(SED_DDD)$(subst /,\/,$(<:.cpp=.d)): \\\nMakefile $(SED_ODD)$(subst /,\/,$(subst $(SRC_ROOT)/,,$(<:.cpp=.dir)))\\\n/g' > $@

$(DEP_RLS)%.d: %.cpp Makefile
	@mkdir -p $(@D)
	$(CXX) -MM $(CPPFLAGS) $(CXXFLAGS) $< | sed -r 's/^(\S+).(\S+):/$(SED_ORD)$(subst /,\/,$(subst $(SRC_ROOT)/,,$(<:.cpp=.o))) $(SED_DRD)$(subst /,\/,$(<:.cpp=.d)): \\\nMakefile $(SED_ORD)$(subst /,\/,$(subst $(SRC_ROOT)/,,$(<:.cpp=.dir)))\\\n/g' > $@

