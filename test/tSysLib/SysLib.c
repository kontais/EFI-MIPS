#include <dirent.h>
#include <sys/stat.h>
#include "SysLib.h"

//
// implement WIN32 _spawnv
//
int _spawnv (INTN mode, CHAR8 * file, CHAR8 *argv[])
{
  pid_t child_pid;
  int WaitStatus;

  if (file == NULL) {
    return -1;
  }

  child_pid = fork();

  if (child_pid == -1) {
    return -1; // fork error
  }

  if (child_pid == 0) {
    return execvp((char*)file,(char**)argv);
  }
  else
  {
    if (mode & _P_WAIT) {
      if (child_pid == wait(&WaitStatus)) {
        if (WIFEXITED(WaitStatus)) {
          return WEXITSTATUS(WaitStatus);
        }
      }
    }
  }
  return -1; // execvp error
}

//
// implement _strlwr
//
void _strlwr (CHAR8* s)
{
  if (s == NULL) {
    return;
  }

  while (*s) {
    if ((*s >= 'A') && (*s <= 'Z')) {
      *s += 0x20;  // 'a' - 'A' = 0x20
    }
    s++;
  }
  return ;
}

UINTN GetFileSize(FILE *fp)
{
  UINTN FileSize;
  fpos_t PosBackup;
  fpos_t PosEnd;

  if (fp == NULL) {
    return -1;
  }

  //
  // backup the file postion & state
  //
  if (0 != fgetpos(fp,&PosBackup)) {
    return -1;
  }

  //
  // move file postion to end
  //
  if (0 != fseek(fp,0,SEEK_END)) {
    return -1;
  }

  // 
  // get postion and state
  //
  if (0 != fgetpos(fp,&PosEnd)) {
    return -1;
  }

  FileSize = _FPOSOFF(PosEnd);
  
  //
  // set postion & state back
  //
  if (0 != fsetpos(fp,&PosBackup)) {
    return -1;
  }

  return FileSize;
}

DIR *   FindFirstFile(char * Path, LIN32_FIND_DATA *FindData)
{
  DIR    *DirHandle = NULL;
  struct dirent * DirEntry;
  struct stat     FileStatus;
  char   FullName[_MAX_PATH];

  if (Path == NULL) {
    return DirHandle;
  }

  DirHandle = opendir(Path);
  if (DirHandle == NULL) {
    return DirHandle;
  }

  DirEntry = readdir(DirHandle);
  if (DirEntry == NULL) {
    closedir(DirHandle);
    DirHandle = NULL;
    return DirHandle;
  }

  strcpy(FindData->DirPath, Path);

  //
  // save file name
  //
  strcpy(FindData->cFileName, DirEntry->d_name);

  memset(FullName,0x00,_MAX_PATH);
  strcpy(FullName,FindData->DirPath);
  strcat(FullName,"/");
  strcat(FullName,DirEntry->d_name);

  if (stat(FullName, &FileStatus) == -1) {
    closedir(DirHandle);
    DirHandle = NULL;
    return DirHandle;
  }
  
  //
  // save file attribute
  //
  FindData->dwFileAttributes = FileStatus.st_mode;

  return DirHandle;
}

BOOLEAN FindNextFile(DIR *DirHandle, LIN32_FIND_DATA *FindData)
{
  struct dirent * DirEntry;
  struct stat     FileStatus;
  char   FullName[_MAX_PATH];

  if (DirHandle == NULL) {
    return FALSE;
  }

  DirEntry = readdir(DirHandle);

  if (DirEntry == NULL) {
    return FALSE;
  }

  //
  // save file name
  //
  strcpy(FindData->cFileName, DirEntry->d_name);

  memset(FullName,0x00,_MAX_PATH);
  strcpy(FullName,FindData->DirPath);
  strcat(FullName,"/");
  strcat(FullName,DirEntry->d_name);

  if (stat(FullName, &FileStatus) == -1) {
    return FALSE;
  }
  
  //
  // save file attribute
  //
  FindData->dwFileAttributes = FileStatus.st_mode;

  return TRUE;
}
void    FindClose(DIR *DirHandle)
{
  if (DirHandle == NULL) {
    return ;
  }

  closedir(DirHandle);
  DirHandle = NULL;

  return;
}

void FindTest()
{
  char DirectoryPath[] = "/home/loongson/EFI-MIPS/test/tSysLib";
  LIN32_FIND_DATA  FindData;
  DIR * DirHandle;

  DirHandle = FindFirstFile(DirectoryPath, &FindData);
  if (DirHandle == NULL) {
    return ;
  }
  
  printf("First File = %s\n",FindData.cFileName);

  while (FindNextFile(DirHandle, &FindData)) {
    printf("Next File = %s\n",FindData.cFileName);
  }

  FindClose(DirHandle);

  return ;

}



int file_test()
{
  FILE* stream;
  char string[] = "This is a test";
  fpos_t filepos;
  int c;


  stream = fopen("test.txt","rb");
  if (stream == NULL) {
    printf("fopen error\n");
    return -1;
  }

//  fwrite(string, strlen(string),1,stream);

  c = fgetc(stream);
  printf("c = %c\n",c);

  //printf("len = %d\n",strlen(string));

  //fseek(stream, SEEK_SET, SEEK_END );

  //printf("ftell = %d\n",ftell(stream));

  printf("FileSize = %d\n",GetFileSize(stream));

  fgetpos(stream,&filepos);

  printf("fpos_t.__pos = %ld\n",filepos.__pos);

  c = fgetc(stream);
  printf("c = %c\n",c);


  fgetpos(stream,&filepos);

  printf("fpos_t.__pos = %ld\n",filepos.__pos);

  fclose(stream);


  return 0;
}

#define MAX_ARRAY_SIZE  20

int _spawnvTest()
{
  UINTN i;
  CHAR8 a[100] = "1zZ3**%\tTAbcdEfgH";
  int   Index;
  CHAR8 *ToolArgumentsArray[MAX_ARRAY_SIZE];
  CHAR8 *ToolName = "ls";



  printf("%s\n",a);
  _strlwr(a);
  printf("%s\n",a);

  i = -1;
  printf("%lx\n",i);

  for (Index = 0; Index< MAX_ARRAY_SIZE; Index++) {
    ToolArgumentsArray[Index] = NULL;
  }
  ToolArgumentsArray[0] = ToolName;
  ToolArgumentsArray[1] = "-l";

  if (0 == _spawnv(_P_WAIT, ToolName, ToolArgumentsArray)) {
    printf("Execute successfully\n");
  }
  else
  {
    printf("Execute Fail\n");
  }


  return 0;
}

int main()
{
//  _spawnvTest();
//  file_test();
  FindTest();
  return 0;
}
