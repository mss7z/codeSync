CXX=g++
CXXFLAGS=-std=c++17 -Wall -Wextra -pedantic-errors
OBJS=codeSyncMain.o codeSyncClasses.o codeSyncInternalStr.o
PROGRAM=codeSync
ifeq ($(OS),Windows_NT)
	PROGRAM:=$(PROGRAM).exe
	RM=cmd.exe /C del
endif

.PHONY: release
release: CXXFLAGS+= -DNDEBUG
release: rebuild

.PHONY: debug
debug: build

.PHONY: build
build:$(PROGRAM)
$(PROGRAM): $(OBJS)
	$(CXX) $^ -o $@
codeSyncMain.o:codeSyncMain.cpp

codeSyncClasses.o:codeSyncClasses.cpp

codeSyncInternalStr.o:codeSyncInternalStr.cpp

$(OBJS):codeSync.hpp

.PHONY: clean
clean:
	$(RM) $(PROGRAM) $(OBJS)

.PHONY: rebuild
rebuild: clean build