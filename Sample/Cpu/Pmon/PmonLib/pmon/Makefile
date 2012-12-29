TARGETEL=YeeLoongEL
START=start.o
MEMSIZE=256


export CC=gcc -mno-abicalls -fno-pic
export CROSS_COMPILE=


S=	/home/loongson/pmon-gcc4
ENDIAN=el


.SUFFIXES:	.S .c .o

CROSS_COMPILE 	?=mips-elf-
#CROSS_COMPILE 	=mipsel-linux-

CPPFLAGS=-mips3

#
# Include the make variables (CC, etc...)
#

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		?= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump
RANLIB		= $(CROSS_COMPILE)ranlib
SIZE		= $(CROSS_COMPILE)size


ENDIAN_FLAG= -EL

OPT?=	-O2

all: pmon.elf
	./tftp.sh

# source tree is located via $S relative to the compilation directory

# Defines

MACHINE=mips
MACHINE_ARCH=mips
COMPILEDIR=${shell pwd}
OBJDIR=${COMPILEDIR}
PMONDIR=${S}


INCLUDES=	-I. -I${S}/include   \
					-I${S} -I${S}/sys       \
					-I${COMPILEDIR} -nostdinc

CPPFLAGS := ${CPPFLAGS} ${ENDIAN_FLAG}	${INCLUDES}  \
		  -mmemcpy -mno-abicalls -fno-builtin

CWARNFLAGS=	-Wall -Wstrict-prototypes \
		-Wno-uninitialized -Wno-format -Wno-main
CFLAGS=		${DEBUG} ${CWARNFLAGS} ${OPT} -G 0
AFLAGS=		-D_LOCORE -G 0
LFLAGS=	${ENDIAN_FLAG} -N -G 0 -Ttext 0x81000000 -e start
STRIPFLAGS=	-g -S --strip-debug

HOSTCC?=	${CC}
HOSTED_CPPFLAGS=${CPPFLAGS:S/^-nostdinc$//}
HOSTED_CFLAGS=	${CFLAGS}

include ${S}/lib/libc/Makefile.inc
LIBC=${CLIB}
#LIBC=${PMONDIR}/lib/libc.a

# compile rules: rules are named ${TYPE}_${SUFFIX}${CONFIG_DEP}
# where TYPE is NORMAL, DRIVER, or PROFILE}; SUFFIX is the file suffix,
# capitalized (e.g. C for a .c file), and CONFIG_DEP is _C if the file
# is marked as config-dependent.

USRLAND_C=	${CC} ${CFLAGS} ${CPPFLAGS} ${PROF} -c $<
USRLAND_C_C=	${CC} ${CFLAGS} ${CPPFLAGS} ${PROF} ${PARAM} -c $<

NORMAL_C=	${CC} ${CFLAGS} ${CPPFLAGS} ${PROF} -c $<
NORMAL_C_C=	${CC} ${CFLAGS} ${CPPFLAGS} ${PROF} ${PARAM} -c $<

DRIVER_C=	${CC} ${CFLAGS} ${CPPFLAGS} ${PROF} -c $<
DRIVER_C_C=	${CC} ${CFLAGS} ${CPPFLAGS} ${PROF} ${PARAM} -c $<

NORMAL_S=	${CC} ${AFLAGS} ${CPPFLAGS} -c $<
NORMAL_S_C=	${AS}  ${COPTS} ${PARAM} $< -o $@


OBJS=	cmdtable.o sbrk.o cfb_console.o kbd.o mips_machdep.o mips.o \
		pci_machdep.o tgt_machdep.o pciconf.o  	sm712.o

CFILES=	$S/pmon/cmdtable.c     \
		$S/pmon/sbrk.c         \
		$S/pmon/cfb_console.c  \
		$S/pmon/kbd.c          \
		$S/pmon/mips_machdep.c \
		$S/pmon/pci_machdep.c  \
		$S/pmon/tgt_machdep.c  \
		$S/pmon/sm712.c

SFILES=

# load lines for config "xxx" will be emitted as:
# xxx: ${SYSTEM_DEP}
#	${SYSTEM_LD_HEAD}
#	${SYSTEM_LD}
#	${SYSTEM_LD_TAIL}
SYSTEM_OBJ=	start.o  ${OBJS} ${LIBC}
SYSTEM_DEP=	Makefile ${SYSTEM_OBJ}
SYSTEM_LD_HEAD=	rm -f $@
SYSTEM_LD=	@echo ${LD} ${LFLAGS} -o $@ ${LIBDIR} '${SYSTEM_OBJ}' ; \
		${LD} ${LFLAGS} -o $@ ${LIBDIR} ${SYSTEM_OBJ}  -L. 
SYSTEM_LD_TAIL=	@${SIZE} $@; chmod 755 $@ ; \
	${OBJCOPY} -R .reginfo -O binary $@ $@.bin ; \
	${OBJCOPY} -R .reginfo $@ pmon

DEBUG?=
ifeq (${DEBUG}, "-g")
LFLAGS+=	-X
SYSTEM_LD_TAIL+=; \
		echo cp $@ $@.gdb; rm -f $@.gdb; cp $@ $@.gdb; \
		echo ${STRIP} ${STRIPFLAGS} $@; ${STRIP} ${STRIPFLAGS} $@
else
LFLAGS+=	-S
endif


pmon.elf: ${SYSTEM_DEP}
	${SYSTEM_LD_HEAD}
	${SYSTEM_LD}
	${SYSTEM_LD_TAIL}



clean::
	rm -rf pmon* *.o 


SRCS=	$S/pmon/start.S ${CFILES} ${SFILES}

start.o: $S/pmon/start.S Makefile
	${NORMAL_S}



cmdtable.o: $S/pmon/cmdtable.c
	${NORMAL_C}

sbrk.o: $S/pmon/sbrk.c
	${NORMAL_C}

cfb_console.o: $S/pmon/cfb_console.c
	${NORMAL_C}

kbd.o: $S/pmon/kbd.c
	${NORMAL_C}

mips_machdep.o: $S/pmon/mips_machdep.c
	${NORMAL_C}

mips.o: $S/pmon/mips.S
	${NORMAL_S}

pci_machdep.o: $S/pmon/pci_machdep.c
	${NORMAL_C}

tgt_machdep.o: $S/pmon/tgt_machdep.c
	${NORMAL_C}

pciconf.o: $S/pmon/pciconf.c
	${NORMAL_C}


sm712.o: $S/pmon/sm712.c
	${NORMAL_C}

