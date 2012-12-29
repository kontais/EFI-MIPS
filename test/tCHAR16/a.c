#include <stdio.h>
typedef unsigned short CHAR16;
typedef int            INTN;

INTN
EfiStrCmp (
  CHAR16   *String,
  CHAR16   *String2
)
/*++
 *
 * Routine Description:
 * Compare the Unicode string pointed by String to the string pointed by String2.
 *
 * Arguments:
 * String - String to process
 *
 * String2 - The other string to process
 *
 * Returns:
 * Return a positive integer if String is lexicall greater than String2; Zero if 
 * the two strings are identical; and a negative interger if String is lexically 
 * less than String2.
 *
 * --*/
{
  while (*String) {
    if (*String != *String2) {
      break;
    }

    String += 1;
    String2 += 1;
  }

  return *String - *String2;
}



int main()
{
  CHAR16 *s1 = "\0\0";
  CHAR16 *s2 = "\0\0";

  printf("EfiStrCmp(s1,s2) = %d\n",EfiStrCmp(s1,"\0\0"));
  printf("%c",'\2');

  return 0;
}
