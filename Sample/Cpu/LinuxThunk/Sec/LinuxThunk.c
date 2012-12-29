/*++

Copyright (c) 2004 - 2009, Intel Corporation                                                         
Portions copyright (c) 2008-2009 Apple Inc. All rights reserved.
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  LinuxThunk.c

Abstract:

  Since the SEC is the only program in our emulation we 
  must use a Tiano mechanism to export APIs to other modules.
  This is the role of the EFI_LINUX_THUNK_PROTOCOL.

  The mLinuxThunkTable exists so that a change to EFI_LINUX_THUNK_PROTOCOL
  will cause an error in initializing the array if all the member functions
  are not added. It looks like adding a element to end and not initializing
  it may cause the table to be initaliized with the members at the end being
  set to zero. This is bad as jumping to zero will crash.
  

  gLinux is a a public exported global that contains the initialized
  data.

--*/

#include "SecMain.h"

int settimer_initialized;
struct timeval settimer_timeval;
void (*settimer_callback)(UINT64 delta);

void
settimer_handler (int sig)
{
  struct timeval timeval;
  UINT64 delta;

  gettimeofday (&timeval, NULL);
  delta = ((UINT64)timeval.tv_sec * 1000) + (timeval.tv_usec / 1000)
    - ((UINT64)settimer_timeval.tv_sec * 1000) 
    - (settimer_timeval.tv_usec / 1000);
  settimer_timeval = timeval;
  if (settimer_callback)
    (*settimer_callback)(delta);
}

VOID
SetTimer (UINT64 PeriodMs, VOID (*CallBack)(UINT64 DeltaMs))
{
  struct itimerval timerval;
  UINT32 remainder;

  if (!settimer_initialized) {
    struct sigaction act;

    settimer_initialized = 1;
    act.sa_handler = settimer_handler;
    act.sa_flags = 0;
    sigemptyset (&act.sa_mask);
    if (sigaction (SIGALRM, &act, NULL) != 0) {
      printf ("SetTimer: sigaction error %s\n", strerror (errno));
    }
    if (gettimeofday (&settimer_timeval, NULL) != 0) {
      printf ("SetTimer: gettimeofday error %s\n", strerror (errno));
    }
  }
  timerval.it_value.tv_sec = DivU64x32(PeriodMs, 1000,NULL);
  DivU64x32(PeriodMs, 1000, &remainder);
  timerval.it_value.tv_usec = remainder * 1000;
  timerval.it_value.tv_sec = DivU64x32(PeriodMs, 1000,NULL);
  timerval.it_interval = timerval.it_value;
  
  if (setitimer (ITIMER_REAL, &timerval, NULL) != 0) {
    printf ("SetTimer: setitimer error %s\n", strerror (errno));
  }
  settimer_callback = CallBack;
}

void
msSleep (unsigned long Milliseconds)
{
  struct timespec ts;

  ts.tv_sec = Milliseconds / 1000;
  ts.tv_nsec = (Milliseconds % 1000) * 1000000;

  while (nanosleep (&ts, &ts) != 0 && errno == EINTR)
    ;
}

void LinuxThunk_Exit (
  int Status
  )
{
  exit(Status);
}

VOID
GetLocalTime (EFI_TIME *Time)
{
  struct tm *tm;
  time_t t;

  t = time (NULL);
  tm = localtime (&t);

  Time->Year = 1900 + tm->tm_year;
  Time->Month = tm->tm_mon + 1;
  Time->Day = tm->tm_mday;
  Time->Hour = tm->tm_hour;
  Time->Minute = tm->tm_min;
  Time->Second = tm->tm_sec;
  Time->Nanosecond = 0;
  Time->TimeZone = timezone;
  Time->Daylight = (daylight ? EFI_TIME_ADJUST_DAYLIGHT : 0)
    | (tm->tm_isdst > 0 ? EFI_TIME_IN_DAYLIGHT : 0);
}

struct tm *
LinuxThunk_GmTime (
  const time_t *timep
  )
{
  return gmtime(timep);
}

void
TzSet (void)
{
  STATIC int done = 0;
  if (!done) {
    tzset ();
    done = 1;
  }
}

long
GetTimeZone(void)
{
  TzSet ();
  return timezone;
}

int
GetDayLight(void)
{
  TzSet ();
  return daylight;
}

int
LinuxThunk_poll (
  struct pollfd *pfd, 
  int nfds, 
  int timeout
  )
{
  return (LinuxPoll)poll(pfd, nfds, timeout);
}

int
LinuxThunk_Read (
  int  fd, 
  void *buf, 
  int  count
  )
{
  return (LinuxRead)read(fd, buf, count);
}

int
LinuxThunk_Write (
  int        fd, 
  const void *buf, 
  int        count
  )
{
  return (LinuxWrite)write(fd, buf, count);
}

int
LinuxThunk_Open (
  const char *name, 
  int        flags, 
  int        mode
  )
{
  return (LinuxOpen)open(name, flags, mode);
}

long int
LinuxThunk_Lseek (
  int      fd, 
  long int off, 
  int      whence
  )
{
  return (LinuxSeek)lseek(fd, off, whence);
}

int
LinuxThunk_Ftruncate (
  int      fd, 
  long int len
  )
{
  return (LinuxFtruncate)ftruncate(fd, len);
}

int
LinuxThunk_Close (
  int      fd
  )
{
  return close(fd);
}

int
LinuxThunk_MkDir (
  const char *pathname, 
  mode_t     mode
  )
{
  return mkdir(pathname, mode);
}

int
LinuxThunk_RmDir (
  const char *pathname
  )
{
  return rmdir(pathname);
}

int
LinuxThunk_UnLink (
  const char *pathname
  )
{
  return unlink(pathname);
}

int
GetErrno(void)
{
  return errno;
}


DIR *
LinuxThunk_OpenDir (
  const char *pathname
  )
{
  return opendir(pathname);
}

void
LinuxThunk_RewindDir (
  DIR *dir
  )
{
  return rewinddir(dir);
}

struct dirent *
LinuxThunk_ReadDir ( 
  DIR *dir
  )
{
  return readdir(dir);
}

int
LinuxThunk_CloseDir (
  DIR *dir
  )
{
  return closedir(dir);
}

int
LinuxThunk_Stat (
  const char  *path, 
  struct stat *buf
  )
{
  return stat(path, buf);
}

int
LinuxThunk_StatFs (
  const char    *path, 
  struct statfs *buf
  )
{
  return statfs(path, buf);
}

int
LinuxThunk_Rename (
  const char *oldpath, 
  const char *newpath
  )
{
  return rename(oldpath, newpath);
}

time_t
LinuxThunk_MkTime (
  struct tm *tm
  )
{
  return mktime(tm);
}

int
LinuxThunk_FSync (
  int      fd
  )
{
  return fsync(fd);
}

int
LinuxThunk_Chmod (
  const char *path, 
  mode_t     mode
  )
{
  return chmod(path, mode);
}

int
LinuxThunk_UTime (
  const char           *filename, 
  const struct utimbuf *buf
  )
{
  return utime(filename, buf);
}

int
LinuxThunk_Tcflush (
  int fildes, 
  int queue_selector
  )
{
  return tcflush(fildes, queue_selector);
}

extern EFI_STATUS
UgaCreate(struct _EFI_LINUX_UGA_IO_PROTOCOL **UgaIo, CONST CHAR16 *Title);


void
LinuxThunk_Perror (
  __const char *__s
  )
{
  return perror(__s);
}

int 
LinuxThunk_IoCtl (
  int fd, 
  unsigned long int __request, 
  int data
  )
{
  return ioctl(fd, __request, data);
}

int 
LinuxThunk_Fcntl (
  int __fd, 
  int __cmd, 
  int data
  )
{
  return fcntl(__fd, __cmd, data);
}

int 
LinuxThunk_Cfsetispeed (
  struct termios *__termios_p, 
  speed_t         __speed
  )
{
  return cfsetispeed(__termios_p, __speed);
}

int 
LinuxThunk_Cfsetospeed (
  struct termios *__termios_p, 
  speed_t         __speed
  )
{
  return cfsetospeed(__termios_p, __speed);
}

int 
LinuxThunk_Tcgetattr (
  int             __fd, 
  struct termios *__termios_p
  )
{
  return tcgetattr(__fd, __termios_p);
}

int 
LinuxThunk_Tcsetattr (
  int     __fd, 
  int     __optional_actions,
  __const struct termios *__termios_p
  )
{
  return tcsetattr(
  	       __fd,
  	       __optional_actions,
  	       __termios_p
  	       );
}
  	

VOID *
LinuxThunk_Dlopen (
  const char *FileName, 
  int Flag
  )
{
  return dlopen(FileName, Flag);
}

char *
LinuxThunk_Dlerror (
  VOID
  )
{
  return dlerror();
}

VOID *
LinuxThunk_Dlsym (
  VOID* Handle, 
  const char* Symbol
  )
{
  return dlsym(Handle, Symbol);
}

int
LinuxThunk_flush_cache (
  char      *addr,
  const int nbytes,
  const int op
  )
{
  return _flush_cache(addr, nbytes, op);
}
int
LinuxThunk_printf (
  const char * Format,
  ...
  )
{
  int Return;
  va_list Marker;

  va_start (Marker, Format);
  Return = vprintf (Format,Marker);
  va_end (Marker);

  return Return;
}

int GetEnv(const char *var, char *buffer, int size)
{
  char *pBuffer;
  int  len;

  pBuffer = getenv(var);
//  printf("Env(%s) = %s\n",var,pBuffer);
  if (pBuffer == NULL) {
    return 0;
  }
  len     = strlen(pBuffer);
  strcpy(buffer,pBuffer);

  return (len > size) ? 0:len;
}

EFI_LINUX_THUNK_PROTOCOL mLinuxThunkTable = {
  EFI_LINUX_THUNK_PROTOCOL_SIGNATURE,
  msSleep,
  LinuxThunk_Exit,
  SetTimer,
  GetLocalTime,
  LinuxThunk_GmTime,
  GetTimeZone,
  GetDayLight,
  LinuxThunk_poll,
  LinuxThunk_Read,
  LinuxThunk_Write,
  GetEnv,
  LinuxThunk_Open,
  LinuxThunk_Lseek,
  LinuxThunk_Ftruncate,
  LinuxThunk_Close,
  LinuxThunk_MkDir,
  LinuxThunk_RmDir,
  LinuxThunk_UnLink,
  GetErrno,
  LinuxThunk_OpenDir,
  LinuxThunk_RewindDir,
  LinuxThunk_ReadDir,
  LinuxThunk_CloseDir,
  LinuxThunk_Stat,
  LinuxThunk_StatFs,
  LinuxThunk_Rename,
  LinuxThunk_MkTime,
  LinuxThunk_FSync,
  LinuxThunk_Chmod,
  LinuxThunk_UTime,
  LinuxThunk_Tcflush,
  UgaCreate,
  LinuxThunk_Perror,
  LinuxThunk_IoCtl,
  LinuxThunk_Fcntl,
  LinuxThunk_Cfsetispeed,
  LinuxThunk_Cfsetospeed,
  LinuxThunk_Tcgetattr,
  LinuxThunk_Tcsetattr,
  LinuxThunk_Dlopen,
  LinuxThunk_Dlerror,
  LinuxThunk_Dlsym,
  LinuxThunk_printf,
  LinuxThunk_flush_cache,
};


EFI_LINUX_THUNK_PROTOCOL *gLinux = &mLinuxThunkTable;
