#
#  Copyright (C) 2001-2003 Hewlett-Packard Co.
#	Contributed by Stephane Eranian <eranian@hpl.hp.com>
#
# This file is part of the ELILO, the EFI Linux boot loader.
#
#  ELILO is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2, or (at your option)
#  any later version.
#
#  ELILO is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with ELILO; see the file COPYING.  If not, write to the Free
#  Software Foundation, 59 Temple Place - Suite 330, Boston, MA
#  02111-1307, USA.
#
# Please check out the elilo.txt for complete documentation on how
# to use this program.
#

include ../Make.defaults
include ../Make.rules

TOPDIR=$(CDIR)/..

FILES=system.o config.o bzimage.o plain_loader.o gzip_loader.o gzip.o

TARGET=sysdeps.o

all: $(TARGET)

system.o: rmswitch.h

rmswitch.h: bin_to_h.c rmswitch.S
	$(CC) -o bin_to_h bin_to_h.c
	$(AS) -o rmswitch.o rmswitch.S
	$(LD) -Ttext 0x0 -s --oformat binary -o rmswitch rmswitch.o
	./bin_to_h <rmswitch >rmswitch.h

$(TARGET): $(FILES)
	$(LD) -r -o $@ $(FILES)

clean:
	$(RM) -f $(TARGET) $(FILES)
	$(RM) -f bin_to_h.o bin_to_h
	$(RM) -f rmswitch.h rmswitch.o rmswitch
