extern int SecMain(void);

int TestMain()
{
  int ReturnValue;

  ReturnValue = SecMain();

  return ReturnValue;
}
void interrupt(void)
{
  return ;
}

int  cpu_info_primary;
