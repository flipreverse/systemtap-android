ifndef VERBOSE
VERBOSE = 0
endif
ifeq ($(VERBOSE),0)
OUTPUT= @
else
OUTPUT=
endif

LD_TEXT ="LD\t$@"
CC_TEXT ="CC\t$<"
DEB_TEXT ="DEB\t$<"

CXX=g++
CXXFLAGS= -g -Werror
LDFLAGS=-lprotobuf

BINARIES:= stapandroid

PWD:=$(shell pwd)
PROTOBUF_DEF_PATH:=$(PWD)/../../android-app
PROTOBUF_DEF_FILE:=$(PROTOBUF_DEF_PATH)/SystemTapMessage.proto
PROTOBUF_COMPILER=protoc --cpp_out=$(PWD) --proto_path=$(PROTOBUF_DEF_PATH)
PROTOBUF_SRC:=SystemTapMessage.pb.cc
PROTOBUF_OBJ:=$(PROTOBUF_SRC:%.cc=%.o)
STAP_ANDROID_SRC:=stapandroid.cpp net.cpp common.cpp
STAP_ANDROID_OBJ:=$(STAP_ANDROID_SRC:%.cpp=%.o)


all: $(BINARIES)

$(PROTOBUF_SRC): $(PROTOBUF_DEF_FILE)
		@echo "Generating $@"
		$(OUTPUT)$(PROTOBUF_COMPILER) $(PROTOBUF_DEF_FILE)

%.o: %.cpp
		@echo -e $(CC_TEXT)
		$(OUTPUT)$(CXX) -c $(CXXFLAGS) $< -o $@

%.o: %.cc
		@echo -e $(CC_TEXT)
		$(OUTPUT)$(CXX) -c $(CXXFLAGS) $< -o $@

stapandroid: $(PROTOBUF_OBJ) $(STAP_ANDROID_OBJ)
		@echo -e $(LD_TEXT)
		$(OUTPUT)$(CXX) $^ $(LDFLAGS) -o $@

clean:
		$(RM) $(BINARIES)
		$(RM) *.o
		$(RM) SystemTapMessage.pb.*

install: $(BINARIES)
		@for bin in "$(BINARIES)"; do \
		sudo cp $$bin /usr/local/bin/; \
		done

uninstall:
		@for bin in "$(BINARIES)"; do \
		sudo rm /usr/local/bin/$$bin; \
		done

.PHONY: all clean install uninstall
