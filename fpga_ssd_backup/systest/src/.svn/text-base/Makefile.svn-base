CC	     := g++
CFLAGS       := -Wall -g 
LIBS         := /home/QA/SSD/include/api/ssd_api.c -lpthread
INCLUDE_PATH := ../include/
OUTPUT_DIR   := output/
OBJS         := $(patsubst %.c, %.o, $(wildcard *.c))
EXE          := $(patsubst %.c,%, $(wildcard *.c))
LOGS         := $(wildcard *.log)
HEADS        :=
SHARD_OBJS   :=
LIB_PATH     :=

.PHONY : all
all : $(EXE) demo

.PHONY : demo
demo :
	-mkdir $(OUTPUT_DIR)
	-mv $(EXE) $(OUTPUT_DIR)

% : %.c
	$(CC) $(CFLAGS) -o $@ $<  $(LIBS)

.PHYON : clean
clean :
	-rm -f *.o
	-rm -rf $(OUTPUT_DIR)
	-rm -f $(EXE)

