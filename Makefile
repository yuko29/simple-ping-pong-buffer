include ../../build-unix/Makefile.config

PROJECT := simple_ping_pong_buffer
SRCS    := $(wildcard *.cpp)
OBJS    := $(SRCS:.cpp=.o)

include ../../build-unix/Makefile.rules
