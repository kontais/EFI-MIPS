#include <stdio.h>


#define __S_IFMT        0170000 /* These bits determine file type.  */
#define __S_ISTYPE(mode, mask)  (((mode) & __S_IFMT) == (mask))

#define S_ISDIR(mode)    __S_ISTYPE((mode), __S_IFDIR)
#define __S_IFDIR       040000 /* Directory.  */

#define S_ISREG(mode)    __S_ISTYPE((mode), __S_IFREG)
#define __S_IFREG       0100000 /* file.  */


int main()
{
  unsigned int a=0x41ed;

  if (S_ISDIR(a)) {
    printf("is dir\n");
  } 


  a=0x81ed;

  if (S_ISREG(a)) {
    printf("is file\n");
  } 



  a=0x81ed;

  if (S_ISREG(a)) {
    printf("is file\n");
  } 
  return 0;
}

