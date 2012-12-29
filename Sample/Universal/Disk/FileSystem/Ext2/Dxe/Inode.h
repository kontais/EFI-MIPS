/*  $OpenBSD: ext2fs_dinode.h,v 1.12 2010/02/16 08:24:13 otto Exp $  */
/*  $NetBSD: ext2fs_dinode.h,v 1.6 2000/01/26 16:21:33 bouyer Exp $  */

/*
 * Copyright (c) 1997 Manuel Bouyer.
 * Copyright (c) 1982, 1989, 1993
 *  The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *  @(#)dinode.h  8.6 (Berkeley) 9/13/94
 *  Modified for ext2fs by Manuel Bouyer.
 */
#ifndef _EXT2_INODE_H_
#define _EXT2_INODE_H_

typedef INT16 ino_t;
/*
 * The root inode is the root of the file system.  Inode 0 can't be used for
 * normal purposes and bad blocks are normally linked to inode 1, thus
 * the root inode is 2.
 * Inode 3 to 10 are reserved in ext2fs.
 */
#define  EXT2_ROOTINO ((ino_t)2)
#define EXT2_RESIZEINO ((ino_t)7)
#define EXT2_FIRSTINO ((ino_t)11)

//
// Adopted from public domain code in FreeBSD libc.
//
#define SECSPERMIN      60
#define MINSPERHOUR     60
#define HOURSPERDAY     24
#define DAYSPERWEEK     7
#define DAYSPERNYEAR    365
#define DAYSPERLYEAR    366
#define SECSPERHOUR     (SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY      ((long) SECSPERHOUR * HOURSPERDAY)
#define MONSPERYEAR     12

#define EPOCH_YEAR      1970

#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))
#define PASS_LEAP_YEARS(y)    ((y) / 4 - (y) / 100 + (y) / 400)

/* File permissions. */
#define  EXT2_IEXEC     0000100    /* Executable. */
#define  EXT2_IWRITE    0000200    /* Writeable. */
#define  EXT2_IREAD     0000400    /* Readable. */
#define  EXT2_ISVTX     0001000    /* Sticky bit. */
#define  EXT2_ISGID     0002000    /* Set-gid. */
#define  EXT2_ISUID     0004000    /* Set-uid. */

/* File types. */
#define  EXT2_IFMT      0170000    /* Mask of file type. */
#define  EXT2_IFIFO     0010000    /* Named pipe (fifo). */
#define  EXT2_IFCHR     0020000    /* Character device. */
#define  EXT2_IFDIR     0040000    /* Directory file. */
#define  EXT2_IFBLK     0060000    /* Block device. */
#define  EXT2_IFREG     0100000    /* Regular file. */
#define  EXT2_IFLNK     0120000    /* Symbolic link. */
#define  EXT2_IFSOCK    0140000    /* UNIX domain socket. */

/* file flags */
#define EXT2_SECRM      0x00000001  /* Secure deletion */
#define EXT2_UNRM       0x00000002  /* Undelete */
#define EXT2_COMPR      0x00000004  /* Compress file */
#define EXT2_SYNC       0x00000008  /* Synchronous updates */
#define EXT2_IMMUTABLE  0x00000010  /* Immutable file */
#define EXT2_APPEND     0x00000020  /* writes to file may only append */
#define EXT2_NODUMP     0x00000040  /* do not dump file */

#define EXT2_ISLNK(m)      (((m) & EXT2_IFMT) == EXT2_IFLNK)
#define EXT2_ISREG(m)      (((m) & EXT2_IFMT) == EXT2_IFREG)
#define EXT2_ISDIR(m)      (((m) & EXT2_IFMT) == EXT2_IFDIR)
#define EXT2_ISCHR(m)      (((m) & EXT2_IFMT) == EXT2_IFCHR)
#define EXT2_ISBLK(m)      (((m) & EXT2_IFMT) == EXT2_IFBLK)
#define EXT2_ISFIFO(m)     (((m) & EXT2_IFMT) == EXT2_IFIFO)
#define EXT2_ISSOCK(m)     (((m) & EXT2_IFMT) == EXT2_IFSOCK)


#endif /* _EXT2_INODE_H_ */
