typedef struct _guid {
  char d0;
  char d1[2];
} GUID;

int main()
{
  GUID gid = {0x01,0x02,0x03};
  GUID gid2 = {0x01,{0x02,0x03}};
  
  gid.d0 = 0x10;

  return 0;
}
