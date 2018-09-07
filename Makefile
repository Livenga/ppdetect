CC      = gcc
FLAGS   = -g -Wall
INCLUDE = -I./include
LINK    = -lpng -lz -lm
OPTION = -Wall -g -O2

TARGET = ppdetect
OBJDIR = objs

SOURCE = $(shell find src/ -name \*.c)
OBJS   = $(addprefix $(OBJDIR)/, $(patsubst %.c, %.o, $(SOURCE)))
BINDIR = $(dir $(OBJS))

default:
	[ -d  $(OBJDIR)  ] || mkdir -pv $(OBJDIR)
	[ -d "$(BINDIR)" ] || mkdir -pv $(BINDIR)
	make $(TARGET)

$(TARGET):$(OBJS)
	$(CC) -o $@ $^ $(FLAGS) $(LINK)

$(OBJDIR)/%.o:%.c
	$(CC) -o $@ -c $< $(FLAGS) $(INCLUDE) $(OPTION)

clear:
	[ ! -d $(OBJDIR) ] || rm -rv $(OBJDIR)
	[ ! -f $(TARGET) ] || rm -v $(TARGET)

all:
	make clear default
