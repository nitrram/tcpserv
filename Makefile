DEFINES:=

DAEMON_FLAGS = -lpthread

CFLAGS:=-O3 -Wall -Wno-deprecated-declarations -Wno-unused-function $(DEFINES) $(DAEMON_FLAGS)
CXXFLAGS:=-O3 -Wall -Wno-deprecated-declarations -Wno-unused-function $(DEFINES) -std=c++11 $(DAEMON_FLAGS)
CC=gcc
CXX=g++
CPP=cpp

#OBJECTS_CPP = $(patsubst %.cpp,%.o,$(wildcard *.cpp))
#OBJECTS_C = $(patsubst %.c,%.o,$(wildcard *.c))
#OBJECTS += $(OBJECTS_CPP)
#OBJECTS += $(OBJECTS_C)

BINDIR = bin
OBJDIR = obj
DAEMON_PATH = daemon
CLIENT_PATH = client
rm = rm -f


DAEMON_C_TARGET = daemon_c
DAEMON_C_SOURCES  := $(wildcard $(DAEMON_PATH)/*.c)
DAEMON_C_INCLUDES := $(wildcard $(DAEMON_PATH)/*.h)
DAEMON_C_OBJECTS  := $(SOURCES:$(DAEMON_PATH)/%.c=$(OBJDIR)/%.o)

CLIENT_CPP_TARGET = client_cpp
CLIENT_CPP_SOURCES  := $(wildcard $(CLIENT_PATH)/*.cpp)
CLIENT_CPP_INCLUDES := $(wildcard $(CLIENT_PATH)/*.h)
CLIENT_CPP_OBJECTS  := $(SOURCES:$(CLIENT_PATH)/%.cpp=$(OBJDIR)/%.o)


#all: $(BINDIR)/$(DAEMON_C_TARGET)

$(DAEMON_C_OBJECTS): $(OBJDIR)/%.o : $(DAEMON_PATH)/%.c
	@echo "yeah"
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

#$(BINDIR)/$(DAEMON_C_TARGET): $(DAEMON_C_OBJECTS)
#	@echo $(DAEMON_C_OBJECTS)
#	$(CC) $(CFLAGS) $(DAEMON_C_OBJECTS) -g -o $@



# $(BINDIR)/$(CLIENT_CPP_TARGET): $(CLIENT_CPP_OBJECTS)
#	@echo $(CLIENT_CPP_OBJECTS)
#	$(CXX) $(CXXFLAGS) $(CLIENT_CPP_OBJECTS) -g -o $@

# $(CLIENT_CPP_OBJECTS): $(OBJDIR)/%.o : $(CLIENT_PATH)/%.cpp
#	@$(CXX) $(CXXFLAGS) -c $< -o $@
#	@echo "Compiled "$<" successfully!"

.PHONY: clean
clean:
	@$(rm) $(DAEMON_C_OBJECTS)
	@echo "Cleanup complete!"

.PHONY: remove
remove: clean
	@$(rm) $(BINDIR)/$(DAEMON_C_TARGET)
	@echo "Executable removed!"
