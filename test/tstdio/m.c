#include "stdio.h"
#include "stdlib.h"
#include "string.h"
typedef unsigned int   UINT32;
typedef unsigned int   UINTN;
typedef          int   INT32;
typedef unsigned short UINT16;
typedef unsigned char  UINT8;
typedef unsigned char  CHAR8;

typedef struct {
  UINT32  Data1;
  UINT16  Data2;
  UINT16  Data3;
  UINT8   Data4[8];
} EFI_GUID;


int main()
{
  EFI_GUID gid = {0x123456,0xabcd,0xef12,{0xa,0xb,0xc,0xd,0xe,0xf,0x1,0x2}};
  EFI_GUID *GuidBuffer;
  CHAR8 str[256]="12345678-abcd-efab-1234-1234567890ab";
  INT32 Index;
  UINTN Data1;
  UINTN Data2;
  UINTN Data3;
  UINTN Data4[8];

  for ( Index = 0; Index < 8; Index++) {
   Data4[Index] = 0;
  }

  Data4[7] = 0xabcdef12;
  printf("Data4[7] = 0x%x\n",Data4[7]);

  Index = sscanf (
            str,
            "%08x-%04x-%04x-%02x%02x-%02hx%02hx%02hx%02hx%02hx%02hx",
            &Data1,
            &Data2,
            &Data3,
            &Data4[0],
            &Data4[1],
            &Data4[2],
            &Data4[3],
            &Data4[4],
            &Data4[5],
            &Data4[6],
            &Data4[7]
      );

  printf("%s\n",str);
  printf("Data1=0x%x\n",Data1);
  printf("Data2=0x%x\n",Data2);
  printf("Data3=0x%x\n",Data3);
  for ( Index = 0; Index < 8; Index++) {
    printf("Data4[%d]=0x%x\n",Index,Data4[Index]);
  }

  GuidBuffer = &gid;
  GuidBuffer->Data1     = (UINT32) Data1;
  GuidBuffer->Data2     = (UINT16) Data2;
  GuidBuffer->Data3     = (UINT16) Data3;
  GuidBuffer->Data4[0]  = (UINT8) Data4[0];
  GuidBuffer->Data4[1]  = (UINT8) Data4[1];
  GuidBuffer->Data4[2]  = (UINT8) Data4[2];
  GuidBuffer->Data4[3]  = (UINT8) Data4[3];
  GuidBuffer->Data4[4]  = (UINT8) Data4[4];
  GuidBuffer->Data4[5]  = (UINT8) Data4[5];
  GuidBuffer->Data4[6]  = (UINT8) Data4[6];
  GuidBuffer->Data4[7]  = (UINT8) Data4[7];


  printf("GuidBuffer->Data4[7] = 0x%x\n",GuidBuffer->Data4[7]);




  return 0;
}

