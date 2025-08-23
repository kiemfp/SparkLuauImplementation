#!/bin/bash

# Define common flags for readability
CXX_FLAGS="-std=c++17 -Wall -Wextra -g"

# Define include paths (added ./Dependencies/LuauVMSet)
INCLUDE_PATHS="-I. \
-I./Dependencies/Luau/Ast/include \
-I./Dependencies/Luau/Common/include \
-I./Dependencies/Luau/Compiler/include \
-I./Dependencies/Luau/VM/include \
-I./Dependencies/Luau/Compiler/src \
-I./Dependencies/Luau/VM/src \
-I./Dependencies/LuauVMSet"

# List all source files (Common.cpp is removed as it doesn't exist)
SOURCE_FILES="Entry.cpp \
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
Dependencies/Luau/VM/src/lvmutils.cpp"

# Construct the clang++ command
CLANG_CMD="clang++ $SOURCE_FILES $INCLUDE_PATHS $CXX_FLAGS -o my_luau_app -lc++ -lm -lSystem -lgcc_s"

# Execute the command, pipe stderr (where errors/warnings go) to stdout,
# then filter with grep to only show lines containing "error:",
# and finally redirect the filtered output to errors_filtered.txt.
eval "$CLANG_CMD" 2>&1 | grep "error:" > errors_filtered.txt
