#ASMTOOL := asminst -a x86_64_gcc -ck --
CXX := ${ASMTOOL} g++
LD := ${CXX}

#ASMTOOL_EXTRAS := -fPIC
GENFLAGS := -Wall -Wextra ${ASMTOOL_EXTRAS}
#LIBS :=
#OPTIMIZATION_FLAGS := -O2
DEBUGGING_FLAGS := -g3
CXXFLAGS := -std=c++17 ${GENFLAGS} ${OPTIMIZATION_FLAGS} ${DEBUGGING_FLAGS}
LDFLAGS := ${LIBS} ${OPTIMIZATION_FLAGS}
