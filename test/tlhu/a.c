typedef char          int8_t;
typedef unsigned char uint8_t;

typedef int8_t INT8;
typedef uint8_t UINT8;

int t()
{
  unsigned a;
  a = *(UINT8 *)0;
  a = *(INT8 *)0;
}

