OBJ_PATH = obj
BIN_PATH = bin
DIST_PATH = dist
EXE_NAME = gpuPlotGenerator.exe
PLATFORM = 32
OPENCL_INCLUDE = ../_opencl/include
OPENCL_LIB = ../_opencl/lib/win/x86

CC = g++
CC_FLAGS = -W -Wall -std=c++11 -O3 -I$(OPENCL_INCLUDE) -m$(PLATFORM)
LD = g++
LD_FLAGS = -fPIC -L$(OPENCL_LIB) -lOpenCL -m$(PLATFORM)

ECHO = echo
MKDIR = mkdir
CP = cp
RM = rm
TOUCH = touch

SRC = $(wildcard *.cpp)
OBJ = $(addprefix $(OBJ_PATH)/, $(SRC:.cpp=.o))
EXE = $(BIN_PATH)/$(EXE_NAME)

all: prepare $(EXE)

prepare:
	@$(MKDIR) -p $(OBJ_PATH)
	@$(MKDIR) -p $(BIN_PATH)

$(EXE): $(OBJ)
	@$(ECHO) Linking [$@]
	@$(LD) -o $@ $^ $(LD_FLAGS)

$(OBJ_PATH)/%.o: %.cpp
	@$(ECHO) Compiling [$<]
	@$(CC) -o $@ -c $< $(CC_FLAGS)

clean:
	@$(ECHO) Cleaning project
	@$(RM) -f $(OBJ_PATH)/*.o

dist: all
	@$(ECHO) Generating distribution
	@$(MKDIR) -p $(DIST_PATH)
	@$(CP) $(EXE) $(DIST_PATH)
	@$(CP) README.md $(DIST_PATH)
	@$(CP) CHANGELOG $(DIST_PATH)
	@$(CP) LICENSE $(DIST_PATH)
	@$(TOUCH) $(DIST_PATH)/devices.txt
	@$(MKDIR) -p $(DIST_PATH)/kernel
	@$(CP) kernel/util.cl $(DIST_PATH)/kernel
	@$(CP) kernel/shabal.cl $(DIST_PATH)/kernel
	@$(CP) kernel/nonce.cl $(DIST_PATH)/kernel
	@$(MKDIR) -p $(DIST_PATH)/plots

distclean: clean
	@$(ECHO) Dist cleaning project
	@$(RM) -f $(EXE)
	@$(RM) -f $(DIST_PATH)/$(EXE_NAME)
	@$(RM) -f $(DIST_PATH)/README
	@$(RM) -f $(DIST_PATH)/CHANGELOG
	@$(RM) -f $(DIST_PATH)/LICENSE
	@$(RM) -f $(DIST_PATH)/devices.txt
	@$(RM) -Rf $(DIST_PATH)/kernel
	@$(RM) -Rf $(DIST_PATH)/plots

rebuild: distclean all