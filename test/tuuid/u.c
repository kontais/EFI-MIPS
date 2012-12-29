#include <stdio.h>
#include <stdlib.h>
#include <uuid/uuid.h>

void print_guid(uuid_t gid)
{
  int i;

  for (i = 0; i< 16; i++) {
    printf("%#2x  ",gid[i]);
  }
  printf("\n");
}
int main()
{
  uuid_t gtGUID;

  uuid_generate(gtGUID);
  print_guid(gtGUID);

  uuid_generate_random(gtGUID);
  print_guid(gtGUID);

  uuid_generate_time(gtGUID);
  print_guid(gtGUID);

  return 0;
}

