typedef int (*FUNCPTR)();
int FuncMain();

int gVar1 = 12345678;
int gVar2;
int gVar3 = 2;

FUNCPTR pFunc = FuncMain;

int Func1()
{
  return 0;
}
int FuncMain()
{
  gVar2 = 0xabcdef;
  gVar1 = 0;
  if (gVar1 = 3) {
    Func1();
    return 3;
  }
  return 0xbb;
}
