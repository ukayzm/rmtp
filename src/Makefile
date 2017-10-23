#
# Makefile
#

PROG = test

CC = gcc
LD = gcc
RM = /bin/rm -f

SRCDIR = .

DIRS = table port

INCS = -I$(SRCDIR)/src -I$(SRCDIR)/porting

#LIBS = -lsocket -lnsl
LIBS =

CFLAGS = -c
#-Wmissing-declarations
#-Wmissing-prototypes 

LDFLAGS = 

OBJS = sestable.o rmtp_core.o rmtp_if.o

SRCS =  $(OBJS:%.o=$(SRCDIR)/%.c)

all: $(PROG)

$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) -o $(PROG) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

#%.o: $(SRCDIR)/%.s
#	$(AS) -f x,c -p 860/F -o $@ $<

#%.o: $(SRCDIR)/%.c
#	$(CC) $(CFLAGS) $(INCS) -o $@ $<

req:
	$(CC) -o req req.c $(LIBS)

clean: 
	$(RM) *.o test req core $(OBJS)

