# Define the C++ compiler
CXX = clang++

# Define compiler flags
CXX_FLAGS = -std=c++17 -Wall -Wextra -g -DLUAU_SHUFFLE_MEMBERS=0

# Define include paths
INCLUDE_PATHS = \
	-I. \
	-I./Dependencies/Luau/Ast/include \
	-I./Dependencies/Luau/Common/include \
	-I./Dependencies/Luau/Compiler/include \
	-I./Dependencies/Luau/VM/include \
	-I./Dependencies/Luau/Compiler/src \
	-I./Dependencies/Luau/VM/src \
	-I./Dependencies/LuauVMSet \
	-I./Misc

# Define library flags
LIBRARIES = -lc++ -lm

# Define the build directory for object files
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

# Define the executable name
TARGET = Spark

# List all source files
# Common.cpp is excluded as it's typically not compiled directly or doesn't exist in some Luau setups
SRCS = \
	Entry.cpp \
	Dependencies/Luau/Ast/src/Allocator.cpp \
	Dependencies/Luau/Ast/src/Ast.cpp \
	Dependencies/Luau/Ast/src/Confusables.cpp \
	Dependencies/Luau/Ast/src/Cst.cpp \
	Dependencies/Luau/Ast/src/Lexer.cpp \
	Dependencies/Luau/Ast/src/Location.cpp \
	Dependencies/Luau/Ast/src/Parser.cpp \
	Dependencies/Luau/Ast/src/StringUtils.cpp \
	Dependencies/Luau/Ast/src/TimeTrace.cpp \
	Dependencies/Luau/Compiler/src/BuiltinFolding.cpp \
	Dependencies/Luau/Compiler/src/Builtins.cpp \
	Dependencies/Luau/Compiler/src/BytecodeBuilder.cpp \
	Dependencies/Luau/Compiler/src/Compiler.cpp \
	Dependencies/Luau/Compiler/src/ConstantFolding.cpp \
	Dependencies/Luau/Compiler/src/CostModel.cpp \
	Dependencies/Luau/Compiler/src/lcode.cpp \
	Dependencies/Luau/Compiler/src/TableShape.cpp \
	Dependencies/Luau/Compiler/src/Types.cpp \
	Dependencies/Luau/Compiler/src/ValueTracking.cpp \
	Dependencies/Luau/VM/src/lapi.cpp \
	Dependencies/Luau/VM/src/laux.cpp \
	Dependencies/Luau/VM/src/lbaselib.cpp \
	Dependencies/Luau/VM/src/lbitlib.cpp \
	Dependencies/Luau/VM/src/lbuffer.cpp \
	Dependencies/Luau/VM/src/lbuflib.cpp \
	Dependencies/Luau/VM/src/lbuiltins.cpp \
	Dependencies/Luau/VM/src/lcorolib.cpp \
	Dependencies/Luau/VM/src/ldblib.cpp \
	Dependencies/Luau/VM/src/ldebug.cpp \
	Dependencies/Luau/VM/src/ldo.cpp \
	Dependencies/Luau/VM/src/lfunc.cpp \
	Dependencies/Luau/VM/src/lgc.cpp \
	Dependencies/Luau/VM/src/lgcdebug.cpp \
	Dependencies/Luau/VM/src/linit.cpp \
	Dependencies/Luau/VM/src/lmathlib.cpp \
	Dependencies/Luau/VM/src/lmem.cpp \
	Dependencies/Luau/VM/src/lnumprint.cpp \
	Dependencies/Luau/VM/src/lobject.cpp \
	Dependencies/Luau/VM/src/loslib.cpp \
	Dependencies/Luau/VM/src/lperf.cpp \
	Dependencies/Luau/VM/src/lstate.cpp \
	Dependencies/Luau/VM/src/lstring.cpp \
	Dependencies/Luau/VM/src/lstrlib.cpp \
	Dependencies/Luau/VM/src/ltable.cpp \
	Dependencies/Luau/VM/src/ltablib.cpp \
	Dependencies/Luau/VM/src/ltm.cpp \
	Dependencies/Luau/VM/src/ludata.cpp \
	Dependencies/Luau/VM/src/lutf8lib.cpp \
	Dependencies/Luau/VM/src/lveclib.cpp \
	Dependencies/Luau/VM/src/lvmexecute.cpp \
	Dependencies/Luau/VM/src/lvmload.cpp \
	Dependencies/Luau/VM/src/lvmutils.cpp

# Generate object file names from source files, placing them in OBJ_DIR
OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))
# For sources within Dependencies/, map their path to OBJ_DIR with appropriate .o extension
OBJS := $(patsubst Dependencies/%.cpp,$(OBJ_DIR)/Dependencies/%.o,$(OBJS))
OBJS := $(patsubst $(OBJ_DIR)/%.o,$(OBJ_DIR)/%.o,$(OBJS)) # Ensure Entry.o is directly in OBJ_DIR

# Rule to build the executable
$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	@echo "Linking $(TARGET)..."
	$(CXX) $(OBJS) $(LIBRARIES) -o $(TARGET)

# Rule to compile a .cpp file into a .o file
# $(OBJ_DIR)/%.o: %.cpp ensures that Entry.cpp maps to build/obj/Entry.o
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D) # Create parent directories if they don't exist
	@echo "Compiling $<..."
	$(CXX) $(CXX_FLAGS) $(INCLUDE_PATHS) -c $< -o $@

# Rule for compiling source files under Dependencies/
# This is a generic rule for any .cpp file inside Dependencies/
$(OBJ_DIR)/Dependencies/%.o: Dependencies/%.cpp
	@mkdir -p $(@D) # Create parent directories if they don't exist
	@echo "Compiling $<..."
	$(CXX) $(CXX_FLAGS) $(INCLUDE_PATHS) -c $< -o $@

# Phony targets
.PHONY: all clean run

all: $(TARGET)

clean:
	@echo "Cleaning build directory..."
	rm -rf $(BUILD_DIR) $(TARGET)

run: $(TARGET)
	@echo "Running $(TARGET)..."
	./$(TARGET)

