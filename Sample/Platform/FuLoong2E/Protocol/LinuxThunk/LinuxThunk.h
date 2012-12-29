/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  LinuxThunk.h

Abstract:

  This protocol allows an EFI driver  in the Linux emulation envirnment
  to make Linux API calls.

  NEVER make a Linux call directly, always make the call via this protocol.

  There are no This pointers on the protocol member functions as they map
  exactly into Linux system calls.

  YOU MUST include EfiLinux.h in place of Efi.h to make this file compile.

--*/

#ifndef _LINUX_THUNK_H_
#define _LINUX_THUNK_H_

#include EFI_PROTOCOL_DEPENDENCY(UgaDraw)

#define EFI_LINUX_THUNK_PROTOCOL_GUID \
  { \
    0x58c518b1, 0x76f3, 0x11d4, {0xbc, 0xea, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} \
  }

//
// The following APIs require EfiLinux.h. In some environmnets the GUID
// definitions are needed but the EfiLinux.h is not included.
// EfiLinux.h is needed to support WINDOWS API requirements.
//
#ifdef _EFI_LINUX_H_

typedef
VOID
(*LinuxSleep) (
  unsigned long Milliseconds
);

typedef
VOID
(*LinuxExit) (
  int status  // exit code for all threads
);

typedef
VOID
(*LinuxSetTimer) (UINT64 PeriodMs, VOID (*CallBack)(UINT64 DeltaMs));
typedef
VOID
(*LinuxGetLocalTime) (EFI_TIME *Time);
struct pollfd;
typedef
struct tm *
(*LinuxGmTime)(const time_t *timep);
typedef
long
(*LinuxGetTimeZone)(void);
typedef
int
(*LinuxGetDayLight)(void);

typedef
int
(*LinuxPoll)(struct pollfd *pfd, int nfds, int timeout);
typedef
int
(*LinuxRead) (int fd, void *buf, int count);
typedef
int
(*LinuxWrite) (int fd, const void *buf, int count);
typedef
char *
(*LinuxGetenv) (const char *var);
typedef
int
(*LinuxOpen) (const char *name, int flags, int mode);
typedef
long int
(*LinuxSeek) (int fd, long int off, int whence);
typedef
int
(*LinuxFtruncate) (int fd, long int len);
typedef
int
(*LinuxClose) (int fd);
typedef
int
(*LinuxMkdir)(const char *pathname, mode_t mode);
typedef
int
(*LinuxRmDir)(const char *pathname);
typedef
int
(*LinuxUnLink)(const char *pathname);
typedef
int
(*LinuxGetErrno)(VOID);
typedef
DIR *
(*LinuxOpenDir)(const char *pathname);
typedef
void
(*LinuxRewindDir)(DIR *dir);
typedef
struct dirent *
(*LinuxReadDir)(DIR *dir);
typedef
int
(*LinuxCloseDir)(DIR *dir);
typedef
int
(*LinuxStat)(const char *path, struct stat *buf);
typedef
int
(*LinuxStatFs)(const char *path, struct statfs *buf);
typedef
int
(*LinuxRename)(const char *oldpath, const char *newpath);
typedef
time_t
(*LinuxMkTime)(struct tm *tm);
typedef
int
(*LinuxFSync)(int fd);
typedef
int
(*LinuxChmod)(const char *path, mode_t mode);
typedef
int
(*LinuxUTime)(const char *filename, const struct utimbuf *buf);

struct _EFI_LINUX_UGA_IO_PROTOCOL;
typedef
EFI_STATUS
(*LinuxUgaCreate)(struct _EFI_LINUX_UGA_IO_PROTOCOL **UgaIo,
		 CONST CHAR16 *Title);

typedef
int
(*LinuxTcflush) (int fildes, int queue_selector);

typedef
void
(*LinuxPerror) (__const char *__s);

typedef 
int 
(*LinuxIoCtl) (int fd, unsigned long int __request, ...);

typedef 
int 
(*LinuxFcntl) (int __fd, int __cmd, ...);

typedef
int 
(*LinuxCfsetispeed) (struct termios *__termios_p, speed_t __speed);

typedef 
int 
(*LinuxCfsetospeed) (struct termios *__termios_p, speed_t __speed);

typedef
int 
(*LinuxTcgetattr) (int __fd, struct termios *__termios_p);

typedef 
int 
(*LinuxTcsetattr) (int __fd, int __optional_actions,
		      __const struct termios *__termios_p);

typedef
VOID *
(*LinuxDlopen) (const char *FileName, int Flag);

typedef
char *
(*LinuxDlerror) (VOID);

typedef 
VOID *
(*LinuxDlsym) (VOID* Handle, const char* Symbol);

//
//  Linux Thunk Protocol
//
EFI_FORWARD_DECLARATION (EFI_LINUX_THUNK_PROTOCOL);

#define EFI_LINUX_THUNK_PROTOCOL_SIGNATURE EFI_SIGNATURE_32 ('L', 'N', 'X', 'T')

struct _EFI_LINUX_THUNK_PROTOCOL {
  UINT64                              Signature;

  LinuxSleep                           Sleep;
  LinuxExit                    	      Exit;
  LinuxSetTimer                        SetTimer;
  LinuxGetLocalTime		                GetLocalTime;
  LinuxGmTime                          GmTime;
  LinuxGetTimeZone                     GetTimeZone;
  LinuxGetDayLight                     GetDayLight;
  LinuxPoll	                          poll;
  LinuxRead                           Read;
  LinuxWrite                          Write;
  LinuxGetenv                         Getenv;
  LinuxOpen                           Open;
  LinuxSeek                           Lseek;
  LinuxFtruncate                      Ftruncate;
  LinuxClose                          Close;
  LinuxMkdir                           MkDir;
  LinuxRmDir                           RmDir;
  LinuxUnLink                          UnLink;
  LinuxGetErrno                        GetErrno;
  LinuxOpenDir                        OpenDir;
  LinuxRewindDir                      RewindDir;
  LinuxReadDir                        ReadDir;
  LinuxCloseDir                       CloseDir;
  LinuxStat                           Stat;
  LinuxStatFs                         StatFs;
  LinuxRename                         Rename;
  LinuxMkTime                         MkTime;
  LinuxFSync                          FSync;
  LinuxChmod                          Chmod;
  LinuxUTime                          UTime;
  LinuxTcflush                         Tcflush;
  LinuxUgaCreate			                  UgaCreate;
  LinuxPerror                          Perror;
  LinuxIoCtl                           IoCtl;
  LinuxFcntl                           Fcntl;
  LinuxCfsetispeed                     Cfsetispeed;
  LinuxCfsetospeed                    Cfsetospeed;
  LinuxTcgetattr                      Tcgetattr;
  LinuxTcsetattr                       Tcsetattr;
  LinuxDlopen                          Dlopen;
  LinuxDlerror                         Dlerror;
  LinuxDlsym                           Dlsym;
};

#endif

extern EFI_GUID gEfiLinuxThunkProtocolGuid;

#endif
