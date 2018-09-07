CC      = gcc
FLAGS   = -g -Wall
INCLUDE =
LINK    =

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
	$(CC) -o $@ -c $< $(FLAGS) $(INCLUDE)

clear:
	[ ! -d $(OBJDIR) ] || rm -rv $(OBJDIR)
	[ ! -f $(TARGET) ] || rm -v $(TARGET)

all:
	make clear default
