int gvar = 0;
int fd(int a)
{
  return 0;
}


int fb(int a)
{
  fd(2);
  return 0;
}

int
fa (int a)
{

  gvar = a;

  fb(10);
  fb(10);


  return 0;
}


int m()
{
  fa(9);
  fb(3);
  return 0;
}

