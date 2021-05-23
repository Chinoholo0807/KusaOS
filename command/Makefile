# commands/Makefile

#ENTRYPOINT	= 0x1000
HD		= ../80m.img

ASM		= nasm
DASM		= objdump
CC		= gcc
LD		= ld
ASMFLAGS	= -I ../include/ -f elf
CFLAGS		= -I ../include/ -c -fno-builtin -fno-stack-protector -Wall
LDFLAGS		= -Ttext 0x1000
DASMFLAGS	= -D
LIB		= ../lib/orangescrt.a
BIN		= echo fs man testmm testproc ps kill csche buddy monitor mstat

# All Phony Targets
.PHONY : everything final clean realclean disasm all install

# Default starting position
everything : $(BIN)

install : all clean
	cp ../kernel.bin ./ -v
	tar vcf inst.tar kernel.bin $(BIN)
	dd if=inst.tar of=$(HD) seek=`echo "obase=10;ibase=16;(\`egrep -e '^ROOT_BASE' ../boot/include/load.inc | sed -e 's/.*0x//g'\`+\`egrep -e '#define[[:space:]]*INSTALL_START_SECT' ../include/sys/config.h | sed -e 's/.*0x//g'\`)*200" | bc` bs=1 count=`ls -l inst.tar | awk -F " " '{print $$5}'` conv=notrunc

all : realclean everything

final : all clean

clean :
	rm -f *.o

realclean :
	rm -f $(BIN) *.o

kernel.bin :
	cp ../kernel.bin ./

start.o : start.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

echo.o: echo.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

echo : echo.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

man.o: man.c ../include/type.h ../include/stdio.h ../include/sys/const.h
	$(CC) $(CFLAGS) -o $@ $<

man : man.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

testmm.o: testmm.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

testmm : testmm.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

testproc.o: testproc.c ../include/type.h ../include/stdio.h 
	$(CC) $(CFLAGS) -o $@ $<

testproc : testproc.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?
	
ps.o: ps.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

ps : ps.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

fs.o: fs.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

fs : fs.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

kill.o: kill.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

kill : kill.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

csche.o: csche.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

csche : csche.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

buddy.o: buddy.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

buddy : buddy.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

monitor.o: monitor.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

monitor : monitor.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?

mstat.o: mstat.c ../include/type.h ../include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<

mstat : mstat.o start.o $(LIB)
	$(LD) $(LDFLAGS) -o $@ $?
