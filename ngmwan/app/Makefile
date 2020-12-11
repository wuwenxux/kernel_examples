#nwcli Makefile
CC		= gcc
CFLAGS 	= -O2 -g
TARGET  =nw1cli
OBJS 	=nw_cmd_chk.o nw_peer_cmd.o nw_cli_tbl.o

.SUFFIXES: .c .o

.PHONY: all
all:depend	$(TARGET)

$(TARGET):	$(OBJS)
		$(CC)	$(CFLAGS)	-o	$(TARGET)	$^
.c.o:
		$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	$(RM) $(OBJS) $(TARGET) depend.inc *~

.PHONY: depend
depend:	$(OBJS:.o=.c)
		-@	$(RM) depend.inc
		-@	for i  in $^;	do cpp -MM $$i	|	sed "s/\ [a-zA-Z0-9][_a-zA-Z0-9]*\.c//g" >> depend.inc;done	
-include depend.inc