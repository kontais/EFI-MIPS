#include "Tiano.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/mman.h"
#include "EfiCommonLib.h"
#include "Print.h"
#include "Math.h"
#include "sys/cachectl.h"

#define MM_SIZE 0x100000
#define MM_TEST

UINT8  SrcStr[100] = "memory map test";
UINT8  DstStr[100];

int main()
{
  void *res;
  UINTN resAlinged;
  EFI_PHYSICAL_ADDRESS    MemoryBuffer;
  EFI_PEI_ELF_LOADER_READ_FILE   ImageRead;
  UINTN  RSize = 20;
  EFI_STATUS  Status;

#ifdef MM_TEST
  res = MapMemory(0, MM_SIZE,
		  PROT_READ | PROT_WRITE | PROT_EXEC,
		  MAP_PRIVATE | MAP_ANONYMOUS);
  if (res == MAP_FAILED)
    printf("mmap error\n");
  printf("Addr = 0x%x\n",(UINTN) res);
  printf("Size = 0x%x\n",MM_SIZE);

  MemoryBuffer = (EFI_PHYSICAL_ADDRESS) res;
#else
  res = malloc (MM_SIZE + 16); // 16 for alignment
  if (res == NULL) 
    perror("malloc error");

  resAlinged = (UINTN) ((UINTN)res + 16) & (~(16-1));
  printf("Addr = 0x%x\n",(UINTN) resAlinged);
  MemoryBuffer = (EFI_PHYSICAL_ADDRESS) resAlinged;
#endif

  
  memcpy((void*)MemoryBuffer,(void*)PeiImageRead,0x400);

  ImageRead = (EFI_PEI_ELF_LOADER_READ_FILE) (UINTN)MemoryBuffer;

  _flush_cache((char*) MemoryBuffer, 0x400, BCACHE);
  
  printf("MB = 0x%x\n",(UINTN) MemoryBuffer);
  
  Status = ImageRead((VOID*) SrcStr, 0, &RSize, (VOID*) DstStr);

  printf("Status = %d\n",Status);

  return 0;
}

