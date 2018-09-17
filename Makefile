CC 		  = gcc
OPTION  = -O2 -Wall -g
LINK 	  = -lm -lpng -ljpeg
INCLUDE = -I./include

TARGET  = ppdetect
OBJDIR  = objs

SRC  = $(shell find src/ -name \*.c)
OBJS = $(addprefix $(OBJDIR)/,$(patsubst %.c,%.o,$(SRC)))

OBJDIRS = $(sort $(dir $(OBJS)))


default:
	[ -d  $(OBJDIR)   ] || mkdir -v $(OBJDIR)
	[ -d "$(OBJDIRS)" ] || mkdir -pv $(OBJDIRS)
	make $(TARGET)

$(TARGET):$(OBJS)
	$(CC) -o $@ $^ $(OPTION) $(LINK)

$(OBJDIR)/%.o:%.c
	$(CC) -c -o $@ $< $(INCLUDE) $(OPTION)

all:
	rm -rv $(OBJDIR) $(TARGET)
	make default
