typedef void(* ENTRY)(int p1, int p2);

void NewEntry(int p1, int p2)
{
  return ;
}

void switchstack(ENTRY pe, int p2, int p3, void * p4, int p5)
{
  int a;
  void* pa;
  pe(p2,p3);
  pa = p4;
  a = p5;
  a = p3;
  return ;

}


int main()
{
  int b=0;
  int c=0;
  void *d=(void *) 0;
  int f=0;
  ENTRY e = NewEntry;
  switchstack(e,b,c,d,f);
  return 0;
}

