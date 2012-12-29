#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>  // for stat()
#include <limits.h>    // for PATH_MAX
#include <errno.h>

int main()
{
  char   DirectoryPath[] = "/home/loongson/EFI-MIPS/test/tdir";
  DIR    *Directory;
  struct dirent * DirectoryEntry;
  char   FileName[PATH_MAX];
  struct stat FileStatus;
  //__mode_t mt;

  if ((Directory = opendir(DirectoryPath)) == NULL) {
    printf("Open directory error\n");
    return -1;
  }

  while ((DirectoryEntry = readdir(Directory)) != NULL) {
    printf("%s\n",DirectoryEntry->d_name);
    if ((strcmp(DirectoryEntry->d_name, ".") == 0) ||
        (strcmp(DirectoryEntry->d_name, "..") == 0)) {
      continue;
    }
    memset(FileName,0x00,PATH_MAX);
    strcpy(FileName,DirectoryPath);
    strcat(FileName,"/");
    strcat(FileName, DirectoryEntry->d_name);

    printf("         %s\n",FileName);

    if (stat(FileName, &FileStatus) == -1) {
      printf("Get stat on %s error: %s\n",DirectoryEntry->d_name, strerror(errno));
      closedir(Directory);
      return -1;
    }

    printf("Mode = 0x%x\n", FileStatus.st_mode);
    printf("Mode = %d\n", FileStatus.st_mode);
    printf("Mode&170000 = %d\n", FileStatus.st_mode & 170000);

    if (S_ISDIR(FileStatus.st_mode)) {
      printf("is DIR\n");
    }
    else if (S_ISREG(FileStatus.st_mode)) {
      printf("is File\n");
    } else {
      printf("is special file\n");
    }

  }

  closedir(Directory);

  return 0;
}
