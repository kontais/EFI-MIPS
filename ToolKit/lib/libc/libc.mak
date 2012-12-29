#
# Copyright (c) 1999, 2000
# Intel Corporation.
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 
# 3. All advertising materials mentioning features or use of this software must
#    display the following acknowledgement:
# 
#    This product includes software developed by Intel Corporation and its
#    contributors.
# 
# 4. Neither the name of Intel Corporation or its contributors may be used to
#    endorse or promote products derived from this software without specific
#    prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 

#
# Include sdk.env environment
#

include $(SDK_INSTALL_DIR)/build/$(SDK_BUILD_ENV)/sdk.env

#
#  Set the base output name
#

BASE_NAME = libc

#
# Globals needed by master.mak
#

TARGET_LIB = $(BASE_NAME)
SOURCE_DIR = $(SDK_INSTALL_DIR)/lib/$(BASE_NAME)
BUILD_DIR  = $(SDK_INSTALL_DIR)/lib/$(BASE_NAME)
#BUILD_DIR  = $(SDK_BUILD_DIR)/lib/$(BASE_NAME)

#
# Additional compile flags
#
#C_FLAGS += -D__STDC__
C_FLAGS += -DXPG4

#
#  If blkxx: devices are to be mapped along with fsxx:, uncomment this line
#
C_FLAGS += -DMAP_BLOCKIO_DEVICES

#
# Include paths
#

include $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR) \
      -I $(SDK_INSTALL_DIR)/include/$(EFI_INC_DIR)/$(PROCESSOR)

include $(SDK_INSTALL_DIR)/include/efishell/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/efishell \
      -I $(SDK_INSTALL_DIR)/include/efishell/$(PROCESSOR)

include $(SDK_INSTALL_DIR)/include/bsd/makefile.hdr
INC += -I $(SDK_INSTALL_DIR)/include/bsd

include include/makefile.hdr
INC += -I include \
      -I include/$(PROCESSOR)

include makefile.hdr
INC += -I . \
      -I locale

#
# Default target
#

#all : $(OBJECTS)
all : $(OBJECTS)

#
# Local sub directories
#

sub_dirs : $(BUILD_DIR)/efi \
           $(BUILD_DIR)/locale \
           $(BUILD_DIR)/Mips32/gen \
           $(BUILD_DIR)/Mips32/math \
           $(BUILD_DIR)/string \
	       $(BUILD_DIR)/stdlib \
	       $(BUILD_DIR)/stdio \
	       $(BUILD_DIR)/regex \
	       $(BUILD_DIR)/wchar \
           $(BUILD_DIR)/sys \
           $(BUILD_DIR)/gen \
           $(BUILD_DIR)/stdtime \
           $(BUILD_DIR)/nls


#
# Sub-directory targets
#

$(BUILD_DIR)/efi      : ; - mkdir -p $(BUILD_DIR)/efi
$(BUILD_DIR)/locale   : ; - mkdir -p $(BUILD_DIR)/locale
$(BUILD_DIR)/Mips32/gen : ; - mkdir -p $(BUILD_DIR)/Mips32/gen
$(BUILD_DIR)/Mips32/math : ; - mkdir -p $(BUILD_DIR)/Mips32/math
$(BUILD_DIR)/string   : ; - mkdir -p $(BUILD_DIR)/string
$(BUILD_DIR)/stdlib   : ; - mkdir -p $(BUILD_DIR)/stdlib
$(BUILD_DIR)/stdio    : ; - mkdir -p $(BUILD_DIR)/stdio
$(BUILD_DIR)/regex    : ; - mkdir -p $(BUILD_DIR)/regex
$(BUILD_DIR)/wchar    : ; - mkdir -p $(BUILD_DIR)/wchar
$(BUILD_DIR)/sys      : ; - mkdir -p $(BUILD_DIR)/sys
$(BUILD_DIR)/gen      : ; - mkdir -p $(BUILD_DIR)/gen
$(BUILD_DIR)/stdtime  : ; - mkdir -p $(BUILD_DIR)/stdtime
$(BUILD_DIR)/nls      : ; - mkdir -p $(BUILD_DIR)/nls


#
#  Library object files
#

OBJECTS += \
    $(BUILD_DIR)/efi/init.o \
    $(BUILD_DIR)/efi/efi_interface.o \
    $(BUILD_DIR)/efi/memory.o \
    $(BUILD_DIR)/efi/consoleio.o \
    $(BUILD_DIR)/efi/fileio.o \
    $(BUILD_DIR)/efi/blockio.o \
    $(BUILD_DIR)/efi/getpagesize.o \
    $(BUILD_DIR)/efi/env.o \
    $(BUILD_DIR)/efi/efi_time.o \
    $(BUILD_DIR)/efi/is_valid_addr.o \
    $(BUILD_DIR)/efi/LoadImage.o \
    $(BUILD_DIR)/efi/StartImage.o \
    $(BUILD_DIR)/efi/UnloadImage.o \
    $(BUILD_DIR)/efi/GetFileDevicePath.o \
\
    $(BUILD_DIR)/locale/ansi.o \
    $(BUILD_DIR)/locale/collate.o \
    $(BUILD_DIR)/locale/collcmp.o \
    $(BUILD_DIR)/locale/isctype.o \
    $(BUILD_DIR)/locale/none.o \
    $(BUILD_DIR)/locale/runetype.o \
    $(BUILD_DIR)/locale/table.o \
    $(BUILD_DIR)/locale/tolower.o \
    $(BUILD_DIR)/locale/toupper.o \
    $(BUILD_DIR)/locale/rune.o \
    $(BUILD_DIR)/locale/setlocale.o \
    $(BUILD_DIR)/locale/lconv.o \
    $(BUILD_DIR)/locale/localeconv.o \
\
    $(BUILD_DIR)/Mips32/gen/isinf.o \
\
    $(BUILD_DIR)/string/bcmp.o \
    $(BUILD_DIR)/string/bcopy.o \
    $(BUILD_DIR)/string/bzero.o \
    $(BUILD_DIR)/string/ffs.o \
    $(BUILD_DIR)/string/index.o \
    $(BUILD_DIR)/string/memccpy.o \
    $(BUILD_DIR)/string/memchr.o \
    $(BUILD_DIR)/string/memcmp.o \
    $(BUILD_DIR)/string/memcpy.o \
    $(BUILD_DIR)/string/memmove.o \
    $(BUILD_DIR)/string/memset.o \
    $(BUILD_DIR)/string/rindex.o \
    $(BUILD_DIR)/string/strcasecmp.o \
    $(BUILD_DIR)/string/strcat.o \
    $(BUILD_DIR)/string/strchr.o \
    $(BUILD_DIR)/string/strcmp.o \
    $(BUILD_DIR)/string/strcoll.o \
    $(BUILD_DIR)/string/strcpy.o \
    $(BUILD_DIR)/string/strcspn.o \
    $(BUILD_DIR)/string/strdup.o \
    $(BUILD_DIR)/string/strerror.o \
    $(BUILD_DIR)/string/strlen.o \
    $(BUILD_DIR)/string/strmode.o \
    $(BUILD_DIR)/string/strncat.o \
    $(BUILD_DIR)/string/strncmp.o \
    $(BUILD_DIR)/string/strncpy.o \
    $(BUILD_DIR)/string/strpbrk.o \
    $(BUILD_DIR)/string/strrchr.o \
    $(BUILD_DIR)/string/strsep.o \
    $(BUILD_DIR)/string/strspn.o \
    $(BUILD_DIR)/string/strstr.o \
    $(BUILD_DIR)/string/strtok.o \
    $(BUILD_DIR)/string/strxfrm.o \
    $(BUILD_DIR)/string/swab.o \
\
    $(BUILD_DIR)/stdlib/abort.o \
    $(BUILD_DIR)/stdlib/abs.o \
    $(BUILD_DIR)/stdlib/atexit.o \
    $(BUILD_DIR)/stdlib/atoi.o \
    $(BUILD_DIR)/stdlib/atol.o \
    $(BUILD_DIR)/stdlib/exit.o \
    $(BUILD_DIR)/stdlib/getopt.o \
    $(BUILD_DIR)/stdlib/labs.o \
    $(BUILD_DIR)/stdlib/malloc.o \
    $(BUILD_DIR)/stdlib/rand.o \
    $(BUILD_DIR)/stdlib/random.o \
    $(BUILD_DIR)/stdlib/strtod.o \
    $(BUILD_DIR)/stdlib/strtol.o \
    $(BUILD_DIR)/stdlib/strtoul.o \
    $(BUILD_DIR)/stdlib/strtoq.o \
    $(BUILD_DIR)/stdlib/strtouq.o \
    $(BUILD_DIR)/stdlib/qsort.o \
    $(BUILD_DIR)/stdlib/getenv.o \
    $(BUILD_DIR)/stdlib/putenv.o \
    $(BUILD_DIR)/stdlib/setenv.o \
    $(BUILD_DIR)/stdlib/system.o \
\
    $(BUILD_DIR)/stdio/_flock_stub.o \
    $(BUILD_DIR)/stdio/asprintf.o \
    $(BUILD_DIR)/stdio/clrerr.o \
    $(BUILD_DIR)/stdio/fclose.o \
    $(BUILD_DIR)/stdio/fdopen.o \
    $(BUILD_DIR)/stdio/feof.o \
    $(BUILD_DIR)/stdio/ferror.o \
    $(BUILD_DIR)/stdio/fflush.o \
    $(BUILD_DIR)/stdio/fgetc.o \
    $(BUILD_DIR)/stdio/fgetln.o \
    $(BUILD_DIR)/stdio/fgetpos.o \
    $(BUILD_DIR)/stdio/fgets.o \
    $(BUILD_DIR)/stdio/fileno.o \
    $(BUILD_DIR)/stdio/findfp.o \
    $(BUILD_DIR)/stdio/flags.o \
    $(BUILD_DIR)/stdio/fopen.o \
    $(BUILD_DIR)/stdio/fprintf.o \
    $(BUILD_DIR)/stdio/fpurge.o \
    $(BUILD_DIR)/stdio/fputc.o \
    $(BUILD_DIR)/stdio/fputs.o \
    $(BUILD_DIR)/stdio/fread.o \
    $(BUILD_DIR)/stdio/freopen.o \
    $(BUILD_DIR)/stdio/fscanf.o \
    $(BUILD_DIR)/stdio/fseek.o \
    $(BUILD_DIR)/stdio/fsetpos.o \
    $(BUILD_DIR)/stdio/ftell.o \
    $(BUILD_DIR)/stdio/funopen.o \
    $(BUILD_DIR)/stdio/fvwrite.o \
    $(BUILD_DIR)/stdio/fwalk.o \
    $(BUILD_DIR)/stdio/fwrite.o \
    $(BUILD_DIR)/stdio/gets.o \
    $(BUILD_DIR)/stdio/getw.o \
    $(BUILD_DIR)/stdio/makebuf.o \
    $(BUILD_DIR)/stdio/mktemp.o \
    $(BUILD_DIR)/stdio/perror.o \
    $(BUILD_DIR)/stdio/printf.o \
    $(BUILD_DIR)/stdio/puts.o \
    $(BUILD_DIR)/stdio/putw.o \
    $(BUILD_DIR)/stdio/refill.o \
    $(BUILD_DIR)/stdio/remove.o \
    $(BUILD_DIR)/stdio/rewind.o \
    $(BUILD_DIR)/stdio/rget.o \
    $(BUILD_DIR)/stdio/scanf.o \
    $(BUILD_DIR)/stdio/setbuf.o \
    $(BUILD_DIR)/stdio/setbuffer.o \
    $(BUILD_DIR)/stdio/setvbuf.o \
    $(BUILD_DIR)/stdio/snprintf.o \
    $(BUILD_DIR)/stdio/sprintf.o \
    $(BUILD_DIR)/stdio/sscanf.o \
    $(BUILD_DIR)/stdio/stdio.o \
    $(BUILD_DIR)/stdio/tempnam.o \
    $(BUILD_DIR)/stdio/tmpfile.o \
    $(BUILD_DIR)/stdio/tmpnam.o \
    $(BUILD_DIR)/stdio/ungetc.o \
    $(BUILD_DIR)/stdio/vasprintf.o \
    $(BUILD_DIR)/stdio/vfprintf.o \
    $(BUILD_DIR)/stdio/vfscanf.o \
    $(BUILD_DIR)/stdio/vprintf.o \
    $(BUILD_DIR)/stdio/vscanf.o \
    $(BUILD_DIR)/stdio/vsnprintf.o \
    $(BUILD_DIR)/stdio/vsprintf.o \
    $(BUILD_DIR)/stdio/vsscanf.o \
    $(BUILD_DIR)/stdio/wbuf.o \
    $(BUILD_DIR)/stdio/wsetup.o \
\
    $(BUILD_DIR)/regex/regcomp.o \
    $(BUILD_DIR)/regex/regerror.o \
    $(BUILD_DIR)/regex/regexec.o \
    $(BUILD_DIR)/regex/regfree.o \
\
    $(BUILD_DIR)/wchar/wcscat.o \
    $(BUILD_DIR)/wchar/wcschr.o \
    $(BUILD_DIR)/wchar/wcscmp.o \
    $(BUILD_DIR)/wchar/wcscpy.o \
    $(BUILD_DIR)/wchar/wcscspn.o \
    $(BUILD_DIR)/wchar/wcslen.o \
    $(BUILD_DIR)/wchar/wcsncat.o \
    $(BUILD_DIR)/wchar/wcsncmp.o \
    $(BUILD_DIR)/wchar/wcsncpy.o \
    $(BUILD_DIR)/wchar/wcspbrk.o \
    $(BUILD_DIR)/wchar/wcsrchr.o \
    $(BUILD_DIR)/wchar/wcsspn.o \
    $(BUILD_DIR)/wchar/wcssep.o \
    $(BUILD_DIR)/wchar/wcsstr.o \
    $(BUILD_DIR)/wchar/wcstok.o \
    $(BUILD_DIR)/wchar/wgetopt.o \
    $(BUILD_DIR)/wchar/wmemchr.o \
    $(BUILD_DIR)/wchar/wmemcmp.o \
    $(BUILD_DIR)/wchar/wmemcpy.o \
    $(BUILD_DIR)/wchar/wmemmove.o \
    $(BUILD_DIR)/wchar/wmemset.o \
    $(BUILD_DIR)/wchar/iswctype.o \
    $(BUILD_DIR)/wchar/wctrans.o \
    $(BUILD_DIR)/wchar/wctype.o \
    $(BUILD_DIR)/wchar/wwbuf.o \
    $(BUILD_DIR)/wchar/fputwc.o \
    $(BUILD_DIR)/wchar/wrefill.o \
    $(BUILD_DIR)/wchar/rgetw.o \
    $(BUILD_DIR)/wchar/fgetwc.o \
    $(BUILD_DIR)/wchar/ungetwc.o \
    $(BUILD_DIR)/wchar/fputws.o \
    $(BUILD_DIR)/wchar/fgetws.o \
    $(BUILD_DIR)/wchar/putws.o \
    $(BUILD_DIR)/wchar/getws.o \
    $(BUILD_DIR)/wchar/wopen.o \
    $(BUILD_DIR)/wchar/wfopen.o \
    $(BUILD_DIR)/wchar/wstat.o \
    $(BUILD_DIR)/wchar/wfaststat.o \
    $(BUILD_DIR)/wchar/wmkdir.o \
    $(BUILD_DIR)/wchar/wrmdir.o \
    $(BUILD_DIR)/wchar/vfwprintf.o \
    $(BUILD_DIR)/wchar/vswprintf.o \
    $(BUILD_DIR)/wchar/vwprintf.o \
    $(BUILD_DIR)/wchar/fwprintf.o \
    $(BUILD_DIR)/wchar/swprintf.o \
    $(BUILD_DIR)/wchar/wprintf.o \
    $(BUILD_DIR)/wchar/vfwscanf.o \
    $(BUILD_DIR)/wchar/vswscanf.o \
    $(BUILD_DIR)/wchar/vwscanf.o \
    $(BUILD_DIR)/wchar/fwscanf.o \
    $(BUILD_DIR)/wchar/swscanf.o \
    $(BUILD_DIR)/wchar/wscanf.o \
    $(BUILD_DIR)/wchar/wcstod.o \
    $(BUILD_DIR)/wchar/wcstol.o \
    $(BUILD_DIR)/wchar/wcstoul.o \
    $(BUILD_DIR)/wchar/wcstoq.o \
    $(BUILD_DIR)/wchar/wcstouq.o \
    $(BUILD_DIR)/wchar/wcscoll.o \
    $(BUILD_DIR)/wchar/wcsftime.o \
    $(BUILD_DIR)/wchar/wcsxfrm.o \
    $(BUILD_DIR)/wchar/wasctime.o \
    $(BUILD_DIR)/wchar/wctime.o \
\
    $(BUILD_DIR)/sys/init.o \
    $(BUILD_DIR)/sys/__error.o \
    $(BUILD_DIR)/sys/_exit.o \
    $(BUILD_DIR)/sys/close.o \
    $(BUILD_DIR)/sys/rename.o \
    $(BUILD_DIR)/sys/fcntl.o \
    $(BUILD_DIR)/sys/filedesc.o \
    $(BUILD_DIR)/sys/getpid.o \
    $(BUILD_DIR)/sys/gettimeofday.o \
    $(BUILD_DIR)/sys/ioctl.o \
    $(BUILD_DIR)/sys/isatty.o \
    $(BUILD_DIR)/sys/open.o \
    $(BUILD_DIR)/sys/lseek.o \
    $(BUILD_DIR)/sys/read.o \
    $(BUILD_DIR)/sys/stat.o \
    $(BUILD_DIR)/sys/faststat.o \
    $(BUILD_DIR)/sys/unlink.o \
    $(BUILD_DIR)/sys/write.o \
    $(BUILD_DIR)/sys/writev.o \
    $(BUILD_DIR)/sys/map.o \
    $(BUILD_DIR)/sys/getdirentries.o \
    $(BUILD_DIR)/sys/mkdir.o \
    $(BUILD_DIR)/sys/rmdir.o \
    $(BUILD_DIR)/sys/select.o \
    $(BUILD_DIR)/sys/sysctl.o \
    $(BUILD_DIR)/sys/dup.o \
    $(BUILD_DIR)/sys/dup2.o \
    $(BUILD_DIR)/sys/cwd.o \
    $(BUILD_DIR)/sys/utimes.o \
    $(BUILD_DIR)/sys/_start_a.o \
    $(BUILD_DIR)/sys/_start_u.o \
    $(BUILD_DIR)/sys/_shell_start_a.o \
    $(BUILD_DIR)/sys/_shell_start_u.o \
\
    $(BUILD_DIR)/gen/arc4random.o \
    $(BUILD_DIR)/gen/assert.o \
    $(BUILD_DIR)/gen/errlst.o \
    $(BUILD_DIR)/gen/err.o \
    $(BUILD_DIR)/gen/opendir.o \
    $(BUILD_DIR)/gen/readdir.o \
    $(BUILD_DIR)/gen/seekdir.o \
    $(BUILD_DIR)/gen/telldir.o \
    $(BUILD_DIR)/gen/rewinddir.o \
    $(BUILD_DIR)/gen/closedir.o \
    $(BUILD_DIR)/gen/scandir.o \
    $(BUILD_DIR)/gen/sleep.o \
    $(BUILD_DIR)/gen/time.o \
    $(BUILD_DIR)/gen/glob.o \
    $(BUILD_DIR)/gen/stringlist.o \
    $(BUILD_DIR)/gen/ResolveFileName.o \
    $(BUILD_DIR)/gen/utime.o \
\
    $(BUILD_DIR)/stdtime/asctime.o \
    $(BUILD_DIR)/stdtime/localtime.o \
    $(BUILD_DIR)/stdtime/difftime.o \
    $(BUILD_DIR)/stdtime/strftime.o \
    $(BUILD_DIR)/stdtime/strptime.o \
    $(BUILD_DIR)/stdtime/timelocal.o \
\
    $(BUILD_DIR)/nls/catclose.o \
    $(BUILD_DIR)/nls/catgets.o \
    $(BUILD_DIR)/nls/catopen.o \
    $(BUILD_DIR)/nls/msgcat.o 

#
#  Processor specific objects
#

ifeq ($(PROCESSOR),Mips32)
OBJECTS += \
	$(BUILD_DIR)/Mips32/gen/setjmp.o \
	$(BUILD_DIR)/Mips32/math/math.o \
	$(BUILD_DIR)/Mips32/math/mathC.o
endif

#
# Source file dependencies
#

$(BUILD_DIR)/efi/init.o              : efi/init.c                 $(INC_DEPS)
$(BUILD_DIR)/efi/efi_interface.o     : efi/efi_interface.c        $(INC_DEPS)
$(BUILD_DIR)/efi/memory.o            : efi/memory.c               $(INC_DEPS)
$(BUILD_DIR)/efi/consoleio.o         : efi/consoleio.c            $(INC_DEPS)
$(BUILD_DIR)/efi/fileio.o            : efi/fileio.c               $(INC_DEPS)
$(BUILD_DIR)/efi/getpagesize.o       : efi/getpagesize.c          $(INC_DEPS)
$(BUILD_DIR)/efi/env.o               : efi/env.c                  $(INC_DEPS)
$(BUILD_DIR)/efi/efi_time.o          : efi/efi_time.c             $(INC_DEPS)
$(BUILD_DIR)/efi/is_valid_addr.o     : efi/is_valid_addr.c        $(INC_DEPS)
$(BUILD_DIR)/efi/LoadImage.o         : efi/LoadImage.c            $(INC_DEPS)
$(BUILD_DIR)/efi/StartImage.o        : efi/StartImage.c           $(INC_DEPS)
$(BUILD_DIR)/efi/UnloadImage.o       : efi/UnloadImage.c          $(INC_DEPS)
$(BUILD_DIR)/efi/GetFileDevicePath.o : efi/GetFileDevicePath.c    $(INC_DEPS)
                                                                                 
$(BUILD_DIR)/locale/ansi.o           : locale/ansi.c              $(INC_DEPS)
$(BUILD_DIR)/locale/collate.o        : locale/collate.c           $(INC_DEPS)
$(BUILD_DIR)/locale/collcmp.o        : locale/collcmp.c           $(INC_DEPS)
$(BUILD_DIR)/locale/isctype.o        : locale/isctype.c           $(INC_DEPS)
$(BUILD_DIR)/locale/none.o           : locale/none.c              $(INC_DEPS)
$(BUILD_DIR)/locale/runetype.o       : locale/runetype.c          $(INC_DEPS)
$(BUILD_DIR)/locale/table.o          : locale/table.c             $(INC_DEPS)
$(BUILD_DIR)/locale/tolower.o        : locale/tolower.c           $(INC_DEPS)
$(BUILD_DIR)/locale/toupper.o        : locale/toupper.c           $(INC_DEPS)
$(BUILD_DIR)/locale/rune.o           : locale/rune.c              $(INC_DEPS)
$(BUILD_DIR)/locale/setlocale.o      : locale/setlocale.c         $(INC_DEPS)
$(BUILD_DIR)/locale/lconv.o          : locale/lconv.c             $(INC_DEPS)
$(BUILD_DIR)/locale/localeconv.o     : locale/localeconv.c        $(INC_DEPS)
                                                             
$(BUILD_DIR)/Mips32/gen/isinf.o      : Mips32/gen/isinf.c         $(INC_DEPS)
$(BUILD_DIR)/Mips32/gen/setjmp.o     : Mips32/gen/setjmp.s        $(INC_DEPS)
$(BUILD_DIR)/Mips32/math/math.o      : Mips32/math/math.s         $(INC_DEPS)
$(BUILD_DIR)/Mips32/math/mathC.o     : Mips32/math/mathC.c        $(INC_DEPS)
                                                             
$(BUILD_DIR)/string/bcmp.o           : string/bcmp.c              $(INC_DEPS)
$(BUILD_DIR)/string/bcopy.o          : string/bcopy.c             $(INC_DEPS)
$(BUILD_DIR)/string/bzero.o          : string/bzero.c             $(INC_DEPS)
$(BUILD_DIR)/string/ffs.o            : string/ffs.c               $(INC_DEPS)
$(BUILD_DIR)/string/index.o          : string/index.c             $(INC_DEPS)
$(BUILD_DIR)/string/memccpy.o        : string/memccpy.c           $(INC_DEPS)
$(BUILD_DIR)/string/memchr.o         : string/memchr.c            $(INC_DEPS)
$(BUILD_DIR)/string/memcmp.o         : string/memcmp.c            $(INC_DEPS)
$(BUILD_DIR)/string/memcpy.o         : string/memcpy.c            $(INC_DEPS)
$(BUILD_DIR)/string/memmove.o        : string/memmove.c           $(INC_DEPS)
$(BUILD_DIR)/string/memset.o         : string/memset.c            $(INC_DEPS)
$(BUILD_DIR)/string/rindex.o         : string/rindex.c            $(INC_DEPS)
$(BUILD_DIR)/string/strcasecmp.o     : string/strcasecmp.c        $(INC_DEPS)
$(BUILD_DIR)/string/strcat.o         : string/strcat.c            $(INC_DEPS)
$(BUILD_DIR)/string/strchr.o         : string/strchr.c            $(INC_DEPS)
$(BUILD_DIR)/string/strcmp.o         : string/strcmp.c            $(INC_DEPS)
$(BUILD_DIR)/string/strcoll.o        : string/strcoll.c           $(INC_DEPS)
$(BUILD_DIR)/string/strcpy.o         : string/strcpy.c            $(INC_DEPS)
$(BUILD_DIR)/string/strcspn.o        : string/strcspn.c           $(INC_DEPS)
$(BUILD_DIR)/string/strdup.o         : string/strdup.c            $(INC_DEPS)
$(BUILD_DIR)/string/strerror.o       : string/strerror.c          $(INC_DEPS)
$(BUILD_DIR)/string/strlen.o         : string/strlen.c            $(INC_DEPS)
$(BUILD_DIR)/string/strmode.o        : string/strmode.c           $(INC_DEPS)
$(BUILD_DIR)/string/strncat.o        : string/strncat.c           $(INC_DEPS)
$(BUILD_DIR)/string/strncmp.o        : string/strncmp.c           $(INC_DEPS)
$(BUILD_DIR)/string/strncpy.o        : string/strncpy.c           $(INC_DEPS)
$(BUILD_DIR)/string/strpbrk.o        : string/strpbrk.c           $(INC_DEPS)
$(BUILD_DIR)/string/strrchr.o        : string/strrchr.c           $(INC_DEPS)
$(BUILD_DIR)/string/strsep.o         : string/strsep.c            $(INC_DEPS)
$(BUILD_DIR)/string/strspn.o         : string/strspn.c            $(INC_DEPS)
$(BUILD_DIR)/string/strstr.o         : string/strstr.c            $(INC_DEPS)
$(BUILD_DIR)/string/strtok.o         : string/strtok.c            $(INC_DEPS)
$(BUILD_DIR)/string/strxfrm.o        : string/strxfrm.c           $(INC_DEPS)
$(BUILD_DIR)/string/swab.o           : string/swab.c              $(INC_DEPS)
$(BUILD_DIR)/stdlib/abort.o          : stdlib/abort.c             $(INC_DEPS)
$(BUILD_DIR)/stdlib/abs.o            : stdlib/abs.c               $(INC_DEPS)
$(BUILD_DIR)/stdlib/atexit.o         : stdlib/atexit.c            $(INC_DEPS)
$(BUILD_DIR)/stdlib/atoi.o           : stdlib/atoi.c              $(INC_DEPS)
$(BUILD_DIR)/stdlib/atol.o           : stdlib/atol.c              $(INC_DEPS)
$(BUILD_DIR)/stdlib/exit.o           : stdlib/exit.c              $(INC_DEPS)
$(BUILD_DIR)/stdlib/getopt.o         : stdlib/getopt.c            $(INC_DEPS)
$(BUILD_DIR)/stdlib/labs.o           : stdlib/labs.c              $(INC_DEPS)
$(BUILD_DIR)/stdlib/malloc.o         : stdlib/malloc.c            $(INC_DEPS)
$(BUILD_DIR)/stdlib/rand.o           : stdlib/rand.c              $(INC_DEPS)
$(BUILD_DIR)/stdlib/random.o         : stdlib/random.c            $(INC_DEPS)
$(BUILD_DIR)/stdlib/strtod.o         : stdlib/strtod.c            $(INC_DEPS)
$(BUILD_DIR)/stdlib/strtol.o         : stdlib/strtol.c            $(INC_DEPS)
$(BUILD_DIR)/stdlib/strtoul.o        : stdlib/strtoul.c           $(INC_DEPS)
$(BUILD_DIR)/stdlib/strtoq.o         : stdlib/strtoq.c            $(INC_DEPS)
$(BUILD_DIR)/stdlib/strtouq.o        : stdlib/strtouq.c           $(INC_DEPS)
$(BUILD_DIR)/stdlib/qsort.o          : stdlib/qsort.c             $(INC_DEPS)
$(BUILD_DIR)/stdlib/getenv.o         : stdlib/getenv.c            $(INC_DEPS)
$(BUILD_DIR)/stdlib/putenv.o         : stdlib/putenv.c            $(INC_DEPS)
$(BUILD_DIR)/stdlib/setenv.o         : stdlib/setenv.c            $(INC_DEPS)
$(BUILD_DIR)/stdlib/system.o         : stdlib/system.c            $(INC_DEPS)
                                                             
$(BUILD_DIR)/stdio/_flock_stub.o     : stdio/_flock_stub.c        $(INC_DEPS)
$(BUILD_DIR)/stdio/asprintf.o        : stdio/asprintf.c           $(INC_DEPS)
$(BUILD_DIR)/stdio/clrerr.o          : stdio/clrerr.c             $(INC_DEPS)
$(BUILD_DIR)/stdio/fclose.o          : stdio/fclose.c             $(INC_DEPS)
$(BUILD_DIR)/stdio/fdopen.o          : stdio/fdopen.c             $(INC_DEPS)
$(BUILD_DIR)/stdio/feof.o            : stdio/feof.c               $(INC_DEPS)
$(BUILD_DIR)/stdio/ferror.o          : stdio/ferror.c             $(INC_DEPS)
$(BUILD_DIR)/stdio/fflush.o          : stdio/fflush.c             $(INC_DEPS)
$(BUILD_DIR)/stdio/fgetc.o           : stdio/fgetc.c              $(INC_DEPS)
$(BUILD_DIR)/stdio/fgetln.o          : stdio/fgetln.c             $(INC_DEPS)
$(BUILD_DIR)/stdio/fgetpos.o         : stdio/fgetpos.c            $(INC_DEPS)
$(BUILD_DIR)/stdio/fgets.o           : stdio/fgets.c              $(INC_DEPS)
$(BUILD_DIR)/stdio/fileno.o          : stdio/fileno.c             $(INC_DEPS)
$(BUILD_DIR)/stdio/findfp.o          : stdio/findfp.c             $(INC_DEPS)
$(BUILD_DIR)/stdio/flags.o           : stdio/flags.c              $(INC_DEPS)
$(BUILD_DIR)/stdio/fopen.o           : stdio/fopen.c              $(INC_DEPS)
$(BUILD_DIR)/stdio/fprintf.o         : stdio/fprintf.c            $(INC_DEPS)
$(BUILD_DIR)/stdio/fpurge.o          : stdio/fpurge.c             $(INC_DEPS)
$(BUILD_DIR)/stdio/fputc.o           : stdio/fputc.c              $(INC_DEPS)
$(BUILD_DIR)/stdio/fputs.o           : stdio/fputs.c              $(INC_DEPS)
$(BUILD_DIR)/stdio/fread.o           : stdio/fread.c              $(INC_DEPS)
$(BUILD_DIR)/stdio/freopen.o         : stdio/freopen.c            $(INC_DEPS)
$(BUILD_DIR)/stdio/fscanf.o          : stdio/fscanf.c             $(INC_DEPS)
$(BUILD_DIR)/stdio/fseek.o           : stdio/fseek.c              $(INC_DEPS)
$(BUILD_DIR)/stdio/fsetpos.o         : stdio/fsetpos.c            $(INC_DEPS)
$(BUILD_DIR)/stdio/ftell.o           : stdio/ftell.c              $(INC_DEPS)
$(BUILD_DIR)/stdio/funopen.o         : stdio/funopen.c            $(INC_DEPS)
$(BUILD_DIR)/stdio/fvwrite.o         : stdio/fvwrite.c            $(INC_DEPS)
$(BUILD_DIR)/stdio/fwalk.o           : stdio/fwalk.c              $(INC_DEPS)
$(BUILD_DIR)/stdio/fwrite.o          : stdio/fwrite.c             $(INC_DEPS)
$(BUILD_DIR)/stdio/gets.o            : stdio/gets.c               $(INC_DEPS)
$(BUILD_DIR)/stdio/getw.o            : stdio/getw.c               $(INC_DEPS)
$(BUILD_DIR)/stdio/makebuf.o         : stdio/makebuf.c            $(INC_DEPS)
$(BUILD_DIR)/stdio/mktemp.o          : stdio/mktemp.c             $(INC_DEPS)
$(BUILD_DIR)/stdio/perror.o          : stdio/perror.c             $(INC_DEPS)
$(BUILD_DIR)/stdio/printf.o          : stdio/printf.c             $(INC_DEPS)
$(BUILD_DIR)/stdio/puts.o            : stdio/puts.c               $(INC_DEPS)
$(BUILD_DIR)/stdio/putw.o            : stdio/putw.c               $(INC_DEPS)
$(BUILD_DIR)/stdio/refill.o          : stdio/refill.c             $(INC_DEPS)
$(BUILD_DIR)/stdio/remove.o          : stdio/remove.c             $(INC_DEPS)
$(BUILD_DIR)/stdio/rewind.o          : stdio/rewind.c             $(INC_DEPS)
$(BUILD_DIR)/stdio/rget.o            : stdio/rget.c               $(INC_DEPS)
$(BUILD_DIR)/stdio/scanf.o           : stdio/scanf.c              $(INC_DEPS)
$(BUILD_DIR)/stdio/setbuf.o          : stdio/setbuf.c             $(INC_DEPS)
$(BUILD_DIR)/stdio/setbuffer.o       : stdio/setbuffer.c          $(INC_DEPS)
$(BUILD_DIR)/stdio/setvbuf.o         : stdio/setvbuf.c            $(INC_DEPS)
$(BUILD_DIR)/stdio/snprintf.o        : stdio/snprintf.c           $(INC_DEPS)
$(BUILD_DIR)/stdio/sprintf.o         : stdio/sprintf.c            $(INC_DEPS)
$(BUILD_DIR)/stdio/sscanf.o          : stdio/sscanf.c             $(INC_DEPS)
$(BUILD_DIR)/stdio/stdio.o           : stdio/stdio.c              $(INC_DEPS)
$(BUILD_DIR)/stdio/tempnam.o         : stdio/tempnam.c            $(INC_DEPS)
$(BUILD_DIR)/stdio/tmpfile.o         : stdio/tmpfile.c            $(INC_DEPS)
$(BUILD_DIR)/stdio/tmpnam.o          : stdio/tmpnam.c             $(INC_DEPS)
$(BUILD_DIR)/stdio/ungetc.o          : stdio/ungetc.c             $(INC_DEPS)
$(BUILD_DIR)/stdio/vasprintf.o       : stdio/vasprintf.c          $(INC_DEPS)
$(BUILD_DIR)/stdio/vfprintf.o        : stdio/vfprintf.c           $(INC_DEPS)
$(BUILD_DIR)/stdio/vfscanf.o         : stdio/vfscanf.c            $(INC_DEPS)
$(BUILD_DIR)/stdio/vprintf.o         : stdio/vprintf.c            $(INC_DEPS)
$(BUILD_DIR)/stdio/vscanf.o          : stdio/vscanf.c             $(INC_DEPS)
$(BUILD_DIR)/stdio/vsnprintf.o       : stdio/vsnprintf.c          $(INC_DEPS)
$(BUILD_DIR)/stdio/vsprintf.o        : stdio/vsprintf.c           $(INC_DEPS)
$(BUILD_DIR)/stdio/vsscanf.o         : stdio/vsscanf.c            $(INC_DEPS)
$(BUILD_DIR)/stdio/wbuf.o            : stdio/wbuf.c               $(INC_DEPS)
$(BUILD_DIR)/stdio/wsetup.o          : stdio/wsetup.c             $(INC_DEPS)
                                                                  
$(BUILD_DIR)/regex/regcomp.o         : regex/regcomp.c            $(INC_DEPS)
$(BUILD_DIR)/regex/regerror.o        : regex/regerror.c           $(INC_DEPS)
$(BUILD_DIR)/regex/regexec.o         : regex/regexec.c            $(INC_DEPS)
$(BUILD_DIR)/regex/regfree.o         : regex/regfree.c            $(INC_DEPS)
                                                                  
$(BUILD_DIR)/wchar/wcscat.o          : wchar/wcscat.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/wcschr.o          : wchar/wcschr.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/wcscmp.o          : wchar/wcscmp.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/wcscpy.o          : wchar/wcscpy.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/wcscspn.o         : wchar/wcscspn.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/wcslen.o          : wchar/wcslen.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/wcsncat.o         : wchar/wcsncat.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/wcsncmp.o         : wchar/wcsncmp.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/wcsncpy.o         : wchar/wcsncpy.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/wcspbrk.o         : wchar/wcspbrk.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/wcsrchr.o         : wchar/wcsrchr.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/wcsspn.o          : wchar/wcsspn.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/wcssep.o          : wchar/wcssep.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/wcsstr.o          : wchar/wcsstr.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/wcstok.o          : wchar/wcstok.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/wgetopt.o         : wchar/wgetopt.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/wmemchr.o         : wchar/wmemchr.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/wmemcmp.o         : wchar/wmemcmp.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/wmemcpy.o         : wchar/wmemcpy.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/wmemmove.o        : wchar/wmemmove.c           $(INC_DEPS)
$(BUILD_DIR)/wchar/wmemset.o         : wchar/wmemset.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/iswctype.o        : wchar/iswctype.c           $(INC_DEPS)
$(BUILD_DIR)/wchar/wctrans.o         : wchar/wctrans.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/wctype.o          : wchar/wctype.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/wwbuf.o           : wchar/wwbuf.c              $(INC_DEPS)
$(BUILD_DIR)/wchar/fputwc.o          : wchar/fputwc.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/wrefill.o         : wchar/wrefill.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/rgetw.o           : wchar/rgetw.c              $(INC_DEPS)
$(BUILD_DIR)/wchar/fgetwc.o          : wchar/fgetwc.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/ungetwc.o         : wchar/ungetwc.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/fputws.o          : wchar/fputws.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/fgetws.o          : wchar/fgetws.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/putws.o           : wchar/putws.c              $(INC_DEPS)
$(BUILD_DIR)/wchar/getws.o           : wchar/getws.c              $(INC_DEPS)
$(BUILD_DIR)/wchar/fwide.o           : wchar/fwide.c              $(INC_DEPS)
$(BUILD_DIR)/wchar/wopen.o           : wchar/wopen.c              $(INC_DEPS)
$(BUILD_DIR)/wchar/wfopen.o          : wchar/wfopen.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/wstat.o           : wchar/wstat.c              $(INC_DEPS)
$(BUILD_DIR)/wchar/wfaststat.o       : wchar/wfaststat.c          $(INC_DEPS)
$(BUILD_DIR)/wchar/wmkdir.o          : wchar/wmkdir.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/wrmdir.o          : wchar/wrmdir.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/vfwprintf.o       : wchar/vfwprintf.c          $(INC_DEPS)
$(BUILD_DIR)/wchar/vswprintf.o       : wchar/vswprintf.c          $(INC_DEPS)
$(BUILD_DIR)/wchar/vwprintf.o        : wchar/vwprintf.c           $(INC_DEPS)
$(BUILD_DIR)/wchar/fwprintf.o        : wchar/fwprintf.c           $(INC_DEPS)
$(BUILD_DIR)/wchar/swprintf.o        : wchar/swprintf.c           $(INC_DEPS)
$(BUILD_DIR)/wchar/wprintf.o         : wchar/wprintf.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/vfwscanf.o        : wchar/vfwscanf.c           $(INC_DEPS)
$(BUILD_DIR)/wchar/vswscanf.o        : wchar/vswscanf.c           $(INC_DEPS)
$(BUILD_DIR)/wchar/vwscanf.o         : wchar/vwscanf.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/fwscanf.o         : wchar/fwscanf.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/swscanf.o         : wchar/swscanf.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/wscanf.o          : wchar/wscanf.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/wcstod.o          : wchar/wcstod.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/wcstol.o          : wchar/wcstol.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/wcstoul.o         : wchar/wcstoul.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/wcstoq.o          : wchar/wcstoq.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/wcstouq.o         : wchar/wcstouq.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/wcscoll.o         : wchar/wcscoll.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/wcsftime.o        : wchar/wcsftime.c           $(INC_DEPS)
$(BUILD_DIR)/wchar/wcsxfrm.o         : wchar/wcsxfrm.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/wasctime.o        : wchar/wasctime.c           $(INC_DEPS)
$(BUILD_DIR)/wchar/wctime.o          : wchar/wctime.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/btowc.o           : wchar/btowc.c              $(INC_DEPS)
$(BUILD_DIR)/wchar/wctob.o           : wchar/wctob.c              $(INC_DEPS)
$(BUILD_DIR)/wchar/mbsinit.o         : wchar/mbsinit.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/mbrlen.o          : wchar/mbrlen.c             $(INC_DEPS)
$(BUILD_DIR)/wchar/mbrtowc.o         : wchar/mbrtowc.c            $(INC_DEPS)
$(BUILD_DIR)/wchar/wcrtomb.o         : wchar/wcrtomb.c            $(INC_DEPS)
                                                             
$(BUILD_DIR)/sys/init.o              : sys/init.c                 $(INC_DEPS)
$(BUILD_DIR)/sys/__error.o           : sys/__error.c              $(INC_DEPS)
$(BUILD_DIR)/sys/_exit.o             : sys/_exit.c                $(INC_DEPS)
$(BUILD_DIR)/sys/close.o             : sys/close.c                $(INC_DEPS)
$(BUILD_DIR)/sys/rename.o            : sys/rename.c               $(INC_DEPS)
$(BUILD_DIR)/sys/fcntl.o             : sys/fcntl.c                $(INC_DEPS)
$(BUILD_DIR)/sys/filedesc.o          : sys/filedesc.c             $(INC_DEPS)
$(BUILD_DIR)/sys/getpid.o            : sys/getpid.c               $(INC_DEPS)
$(BUILD_DIR)/sys/gettimeofday.o      : sys/gettimeofday.c         $(INC_DEPS)
$(BUILD_DIR)/sys/ioctl.o             : sys/ioctl.c                $(INC_DEPS)
$(BUILD_DIR)/sys/isatty.o            : sys/isatty.c               $(INC_DEPS)
$(BUILD_DIR)/sys/open.o              : sys/open.c                 $(INC_DEPS)
$(BUILD_DIR)/sys/lseek.o             : sys/lseek.c                $(INC_DEPS)
$(BUILD_DIR)/sys/read.o              : sys/read.c                 $(INC_DEPS)
$(BUILD_DIR)/sys/stat.o              : sys/stat.c                 $(INC_DEPS)
$(BUILD_DIR)/sys/faststat.o          : sys/faststat.c             $(INC_DEPS)
$(BUILD_DIR)/sys/unlink.o            : sys/unlink.c               $(INC_DEPS)
$(BUILD_DIR)/sys/write.o             : sys/write.c                $(INC_DEPS)
$(BUILD_DIR)/sys/writev.o            : sys/writev.c               $(INC_DEPS)
$(BUILD_DIR)/sys/map.o               : sys/map.c                  $(INC_DEPS)
$(BUILD_DIR)/sys/getdirentries.o     : sys/getdirentries.c        $(INC_DEPS)
$(BUILD_DIR)/sys/mkdir.o             : sys/mkdir.c                $(INC_DEPS)
$(BUILD_DIR)/sys/rmdir.o             : sys/rmdir.c                $(INC_DEPS)
$(BUILD_DIR)/sys/select.o            : sys/select.c               $(INC_DEPS)
$(BUILD_DIR)/sys/sysctl.o            : sys/sysctl.c               $(INC_DEPS)
$(BUILD_DIR)/sys/dup.o               : sys/dup.c                  $(INC_DEPS)
$(BUILD_DIR)/sys/dup2.o              : sys/dup2.c                 $(INC_DEPS)
$(BUILD_DIR)/sys/cwd.o               : sys/cwd.c                  $(INC_DEPS)
$(BUILD_DIR)/sys/utimes.o            : sys/utimes.c               $(INC_DEPS)
$(BUILD_DIR)/sys/_start_a.o          : sys/_start_a.c             $(INC_DEPS)
$(BUILD_DIR)/sys/_start_u.o          : sys/_start_u.c             $(INC_DEPS)
$(BUILD_DIR)/sys/_shell_start_a.o    : sys/_shell_start_a.c       $(INC_DEPS)
$(BUILD_DIR)/sys/_shell_start_u.o    : sys/_shell_start_u.c       $(INC_DEPS)
                                                             
$(BUILD_DIR)/gen/arc4random.o        : gen/arc4random.c           $(INC_DEPS)
$(BUILD_DIR)/gen/assert.o            : gen/assert.c               $(INC_DEPS)
$(BUILD_DIR)/gen/errlst.o            : gen/errlst.c               $(INC_DEPS)
$(BUILD_DIR)/gen/err.o               : gen/err.c                  $(INC_DEPS)
$(BUILD_DIR)/gen/opendir.o           : gen/opendir.c              $(INC_DEPS)
$(BUILD_DIR)/gen/readdir.o           : gen/readdir.c              $(INC_DEPS)
$(BUILD_DIR)/gen/seekdir.o           : gen/seekdir.c              $(INC_DEPS)
$(BUILD_DIR)/gen/telldir.o           : gen/telldir.c              $(INC_DEPS)
$(BUILD_DIR)/gen/rewinddir.o         : gen/rewinddir.c            $(INC_DEPS)
$(BUILD_DIR)/gen/closedir.o          : gen/closedir.c             $(INC_DEPS)
$(BUILD_DIR)/gen/scandir.o           : gen/scandir.c              $(INC_DEPS)
$(BUILD_DIR)/gen/sleep.o             : gen/sleep.c                $(INC_DEPS)
$(BUILD_DIR)/gen/time.o              : gen/time.c                 $(INC_DEPS)
$(BUILD_DIR)/gen/glob.o              : gen/glob.c                 $(INC_DEPS)
$(BUILD_DIR)/gen/stringlist.o        : gen/stringlist.c           $(INC_DEPS)
$(BUILD_DIR)/gen/ResolveFileName.o   : gen/ResolveFileName.c      $(INC_DEPS)
$(BUILD_DIR)/gen/utime.o             : gen/utime.c                $(INC_DEPS)
                                                                             
$(BUILD_DIR)/stdtime/asctime.o       : stdtime/asctime.c          $(INC_DEPS)
$(BUILD_DIR)/stdtime/localtime.o     : stdtime/localtime.c        $(INC_DEPS)
$(BUILD_DIR)/stdtime/difftime.o      : stdtime/difftime.c         $(INC_DEPS)
$(BUILD_DIR)/stdtime/strftime.o      : stdtime/strftime.c         $(INC_DEPS)
$(BUILD_DIR)/stdtime/strptime.o      : stdtime/strptime.c         $(INC_DEPS)
$(BUILD_DIR)/stdtime/timelocal.o     : stdtime/timelocal.c        $(INC_DEPS)
                                                              
$(BUILD_DIR)/nls/catclose.o          : nls/catclose.c             $(INC_DEPS)
$(BUILD_DIR)/nls/catgets.o           : nls/catgets.c              $(INC_DEPS)
$(BUILD_DIR)/nls/catopen.o           : nls/catopen.c              $(INC_DEPS)
$(BUILD_DIR)/nls/msgcat.o            : nls/msgcat.c               $(INC_DEPS)

#
# Handoff to master.mak
#

include $(SDK_INSTALL_DIR)/build/master.mak

