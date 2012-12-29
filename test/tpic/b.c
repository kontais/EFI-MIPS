extern int funcc();
extern int OsPrint(char * Str);

int funcb()
{
  funcc();
  OsPrint("funcb no-PIC OK\n");
  return 0;
}
