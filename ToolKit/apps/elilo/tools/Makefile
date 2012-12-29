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

FILES=eliloalt.o
TARGET=eliloalt

all: $(TARGET)

#
# redefine local rule (we build a Linux/ia64 binary here)
#
%.o: %.c 
	$(CC) $(OPTIMFLAGS) $(DEBUGFLAGS) -c $< -o $@

$(TARGET):  %:%.o 
	$(CC) -o $@ $(OPTIMFLAGS) $(DEBUGFLAGS) $^

clean:
	$(RM) -f $(TARGET) $(FILES)

