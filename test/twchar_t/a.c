#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <string.h>

wchar_t ts[4] = L"abc";
wchar_t tt[4] = L"abe";
char tc[4] = "abc";
wchar_t dest[25] = L"";
wchar_t src[10]  = L" 张a";




int _wcslen(wchar_t *s)
{
  int i = 0;
  while (*s) {
    s++;
    i++;
  }
  return i;
}

int _wcsncmp (wchar_t *s1, wchar_t *s2, size_t n)
{
  if (n == 0) {
    return n;
  }
  while ( *s1 && *s2 && (n != 0)) {
    if ( *s1 != *s2 ) {
      break;
    }
    if (--n == 0) {
      break;
    }
    s1++;
    s2++;
  }
  return *s1 - *s2;
}

int _wcscmp(wchar_t *s1, wchar_t *s2)
{
  while( *s1 && *s2) {
    if ( *s1 != *s2) {
      break;
    }
    s1++;
    s2++;
  }
  return *s1 - *s2;
}

wchar_t *_wcscpy(wchar_t *dest, wchar_t *src)
{
  wchar_t *const d = dest;
  while (*src) {
    *dest++ = *src++;
  }
  *dest = L'\0';

  return d;
}
int _wcscpy_test()
{
  _wcscpy(dest,src);
  printf("_wcscpy dest len = %d\n",_wcslen(dest));
  return 0;
}
int wcsncmp_test()
{
  printf("wcsncmp=%d\n",wcsncmp(ts,tt,2));
  printf("_wcsncmp=%d\n",_wcsncmp(ts,tt,2));
  return 0;
}

wchar_t *_wcsncpy(wchar_t *dest, const wchar_t *src, size_t n)
{
  wchar_t *const d = dest;
  if (n == 0) {
    return d;
  }
  while (*src && (n != 0)) {
    *dest++ = *src++;
    if (--n == 0) {
      break;
    }
  }
  *dest++ = L'\0';
  while (n-- != 0 ) {
    *dest++ = L'\0';
  }

  return d;
}

int _wcsncpy_test()
{
  _wcsncpy(dest,src,8);
  printf("_wcsncpy dest len = %d\n",_wcslen(dest));
  return 0;
}

wchar_t *_wcscat(wchar_t *dest, const wchar_t *src)
{
  wchar_t *const d = dest;
  while (*dest) {
    dest++;
  }
  while(*src) {
    *dest++ = *src++;
  }
  *dest = L'\0';

  return d;
}

int wcscat_test()
{
  printf("wcscat len = %d\n",_wcslen(_wcscat(dest,src)));
  return 0;
}

int main()
{

  printf("sizeof(wchar_t) = %d\n",sizeof(wchar_t));
#if 0
  printf("0x%x\n",ts[0]);
  printf("0x%x\n",ts[1]);
  printf("0x%x\n",ts[2]);
  printf("0x%x\n",ts[3]);
  printf("0x%x\n",ts[4]);
  printf("0x%x\n",ts[5]);
  printf("0x%x\n",ts[6]);
  printf("0x%x\n",ts[7]);
  printf("0x%x\n",ts[8]);
  printf("0x%x\n",ts[9]);
  printf("_wcscmp=0x%x\n",_wcscmp(ts,tt));
  if (_wcscmp(ts,tt) == 0) {
    printf("equl\n");
  }
  else {
    printf("no equl\n");
  }
  printf("strlen=%d\n",strlen(tc));
  printf("_wcslen=%d\n",_wcslen(tt));
#endif
  _wcscpy_test();
//  wcsncmp_test();
//  _wcsncpy_test();
//  wcscat_test();
  return 0;
}

