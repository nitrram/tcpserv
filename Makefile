DEFINES:=

CFLAGS:=-O3 -Wall -Wno-deprecated-declarations -Wno-unused-function $(DEFINES) $(DAEMON_FLAGS)
CXXFLAGS:=-O3 -Wall -Wno-deprecated-declarations -Wno-unused-function $(DEFINES) -std=c++11 $(DAEMON_FLAGS)
CC=gcc
CXX=g++
CPP=cpp


BINDIR = bin
OBJDIR = obj
DAEMON_PATH = daemon
CLIENT_PATH = client
rm = rm -f

$(shell mkdir -p $(BINDIR) $(OBJDIR))

DAEMON_C_TARGET = daemon_c
DAEMON_C_SOURCES  := $(wildcard $(DAEMON_PATH)/*.c)
DAEMON_C_INCLUDES := $(wildcard $(DAEMON_PATH)/*.h)
DAEMON_C_OBJECTS  := $(DAEMON_C_SOURCES:$(DAEMON_PATH)/%.c=$(OBJDIR)/%.o)

DAEMON_CPP_TARGET = daemon_cpp
DAEMON_CPP_SOURCES  := $(wildcard $(DAEMON_PATH)/*.cpp)
DAEMON_CPP_OBJECTS  := $(DAEMON_CPP_SOURCES:$(DAEMON_PATH)/%.cpp=$(OBJDIR)/%.o)

CLIENT_CPP_TARGET = client_cpp
CLIENT_CPP_SOURCES  := $(wildcard $(CLIENT_PATH)/*.cpp)
CLIENT_CPP_OBJECTS  := $(CLIENT_CPP_SOURCES:$(CLIENT_PATH)/%.cpp=$(OBJDIR)/%.o)


all: $(BINDIR)/$(DAEMON_C_TARGET) $(BINDIR)/$(DAEMON_CPP_TARGET) $(BINDIR)/$(CLIENT_CPP_TARGET)

$(BINDIR)/$(DAEMON_C_TARGET): $(DAEMON_C_OBJECTS)
	@echo $(DAEMON_C_OBJECTS)
	@$(CC) $(CFLAGS) $(DAEMON_C_OBJECTS) -g -o $@

$(BINDIR)/$(DAEMON_CPP_TARGET): $(DAEMON_CPP_OBJECTS)
	@echo $(DAEMON_CPP_OBJECTS)
	@$(CXX) $(CFLAGS) $(DAEMON_CPP_OBJECTS) -g -o $@

$(BINDIR)/$(CLIENT_CPP_TARGET): $(CLIENT_CPP_OBJECTS)
	@echo $(CLIENT_CPP_OBJECTS)
	@$(CXX) $(CXXFLAGS) $(CLIENT_CPP_OBJECTS) -g -o $@

$(DAEMON_C_OBJECTS): $(OBJDIR)/%.o : $(DAEMON_PATH)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

$(DAEMON_CPP_OBJECTS): $(OBJDIR)/%.o : $(DAEMON_PATH)/%.cpp
	@$(CXX) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

$(CLIENT_CPP_OBJECTS): $(OBJDIR)/%.o : $(CLIENT_PATH)/%.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

.PHONY: clean
clean:
	@$(rm) $(DAEMON_C_OBJECTS) $(CLIENT_CPP_OBJECTS)
	@echo "Cleanup complete!"

.PHONY: remove
remove: clean
	@$(rm) $(BINDIR)/$(DAEMON_C_TARGET) $(BINDIR)/$(CLIENT_CPP_TARGET)
	@echo "Executable removed!"
