/*******************************************************************
 *
 * Copyright (C) SAP AG, 2003
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Contact: SAP AG, Neurottstrasse 16, D-69190 Walldorf, GERMANY
 *
 ********************************************************************
 *
 * test source for reference implementation of unicode literals
 * JTC1/SC22/WG14 
 * PDTR 19769
 * suggested C standard extension for unicode character types
 *
 ******************************************************************/

#ifdef UGCC_NO_INCLUDES
  int printf( const char*, ... );
  void abort(void);
  typedef unsigned short char16_t;
  typedef unsigned int char32_t;
  #ifndef __cplusplus
    typedef long wchar_t;
  #endif
#else
  #include <uchar.h>
  #include <stdio.h>
  #include <stdlib.h>
#endif


int g_nFailure = 0;


void verifychar( char c, char c_cmp, int n, int n_cmp ) {
  if( n!=n_cmp || c!=c_cmp ) {
    printf("Verify failed. Expected: %c Size: %d Got: %c Size: %d\n", c_cmp, n_cmp, c, n );
  }   
}

#define VERIFY_CHAR( CHAR_CONST, TYPE, CHAR_EXPECTED ) \
  verifychar( CHAR_CONST, CHAR_EXPECTED, sizeof(CHAR_CONST), sizeof(TYPE) )


void verifycharw( int c, int w_cmp ) {
  if( c!=w_cmp ) {
    printf("Verify failed. Expected: %4x Got: %4x\n", w_cmp, c );
    g_nFailure++;
  }   
}

void printcstr(const char* s, int n) {
  int i=0;
  for( i=0; i<n; i++ ) {
    printf("%c",s[i]);
  }
}

void verifycstr( const char* s, const char* s_cmp, int n ) {
  int i=0;
  for( i=0; i<n; i++ ) {
    if( s[i]!=s_cmp[i] ) {
      printf("Verify failed. Expected: %*s Got: %*s\n", n, s_cmp, n, s);
      g_nFailure++;
      return;          
    }
  }
}

void printwstr(const wchar_t* w, int n) {
  int i=0;
  for( i=0; i<n; i++ ) {
    printf("%4x",(int)w[i]);
  }
}

void verifywstr( const wchar_t* w, const char* s_cmp, int n ) {
  int i=0;
  for( i=0; i<n; i++ ) {
    if( w[i]!=s_cmp[i] ) {
      printf("Verify failed. Expected: %*s Got: ", n, s_cmp);
      printwstr(w,n);
      printf("\n");
      g_nFailure++;          
      return;          
    }
  }
}

void printu16str(const char16_t* u16, int n) {
  int i=0;
  for( i=0; i<n; i++ ) {
    printf("%2x ",(int)u16[i]);
  }
}

void verifyu16str( const char16_t* u16, const char* s_cmp, int n ) {
  int i=0;
  for( i=0; i<n; i++ ) {
    if( u16[i]!=s_cmp[i] ) {
      printf("Verify failed. Expected: %*s Got: ", n, s_cmp);
      printu16str(u16,n);
      printf("\n");
      g_nFailure++;          
      return;          
    }
  }
}

void verifyu16strw( const char16_t* u16, const wchar_t* w_cmp, int n ) {
  int i=0;
  for( i=0; i<n; i++ ) {
    if( u16[i]!=w_cmp[i] ) {
      printf("Verify failed. Expected: ");
      printwstr(w_cmp, n);
      printf(" Got: ", n, w_cmp);
      printu16str(u16,n);
      printf("\n");
      g_nFailure++;          
      return;          
    }
  }
}

void printu32str(const char32_t* u32, int n) {
  int i=0;
  for( i=0; i<n; i++ ) {
    printf("%4x",(int)u32[i]);
  }
}

void verifyu32str( const char32_t* u32, const char* s_cmp, int n ) {
  int i=0;
  for( i=0; i<n; i++ ) {
    if( u32[i]!=s_cmp[i] ) {
      printf("Verify failed. Expected: %*s Got: ", n, s_cmp);
      printu32str(u32,n);
      printf("\n");
      g_nFailure++;          
      return;          
    }
  }
}

void verifyu32strw( const char32_t* u32, const wchar_t* w_cmp, int n ) {
  int i=0;
  for( i=0; i<n; i++ ) {
    if( u32[i]!=w_cmp[i] ) {
      printf("Verify failed. Expected: ");
      printwstr(w_cmp, n);
      printf(" Got: ", n, w_cmp);
      printu32str(u32,n);
      printf("\n");
      g_nFailure++;          
      return;          
    }
  }
}

#define ARR_STR_LEN(arr) (sizeof(arr)/sizeof(arr[0])-1)


void TestChar() {
  VERIFY_CHAR(L'L',wchar_t,'L');
  VERIFY_CHAR(u'u',char16_t,'u');
  VERIFY_CHAR(U'U',char32_t,'U');
}

#if 'B' < 'A'
  #error 'B' < 'A'
#endif

#if L'B' < L'A'
  #error L'B' < L'A'
#endif

#if u'B' < u'A'
  #error u'B' < u'A'
#endif

#if U'B' < U'A'
  #error char U'B' < U'A'
#endif


void TestSingleString() {
  char s[] = "Hello World";
  wchar_t w[] = L"Hello World L";
  unsigned short u16[] = u"Hello World u";
  unsigned int u32[] = U"Hello World U";

  verifycstr( s, "Hello World", ARR_STR_LEN(s) );
  verifywstr( w, "Hello World L", ARR_STR_LEN(w) );
  verifyu16str( u16, "Hello World u", ARR_STR_LEN(u16) );
  verifyu32str( u32, "Hello World U", ARR_STR_LEN(u32) );
}

void TestCombinedString() {
{
  wchar_t w_sw[] = "Hello" L"World s L";
  char16_t u16_su16[] = "Hello" u"World s u";
  char32_t u32_su32[] = "Hello" U"World s U";

  verifywstr( w_sw, "HelloWorld s L", ARR_STR_LEN(w_sw) );
  verifyu16str( u16_su16, "HelloWorld s u", ARR_STR_LEN(u16_su16) );
  verifyu32str( u32_su32, "HelloWorld s U", ARR_STR_LEN(u32_su32) );
}

{
  wchar_t w_ws[] = L"Hello" "World L s";
  char16_t u16_u16s[] = u"Hello" "World u s";
  char32_t u32_u32s[] = U"Hello" "World U s";

  verifywstr( w_ws, "HelloWorld L s", ARR_STR_LEN(w_ws) );
  verifyu16str( u16_u16s, "HelloWorld u s", ARR_STR_LEN(u16_u16s) );
  verifyu32str( u32_u32s, "HelloWorld U s", ARR_STR_LEN(u32_u32s) );
}

{
  wchar_t u32_su16w[] = "Hello" u"World" L"s u " U"L U" ;
  verifywstr( u32_su16w, "HelloWorlds u L U", ARR_STR_LEN(u32_su16w) );
}
}


void TestPtrString() {
  char* s_ptr = "Hello World s ptr";
  wchar_t* w_ptr = L"Hello World L ptr";
  char16_t* u16_ptr = u"Hello World u ptr";
  char32_t* u32_ptr = U"Hello World U ptr";
  int nLen = sizeof("Hello World s ptr");

  verifycstr( s_ptr, "Hello World s ptr", nLen );
  verifywstr( w_ptr, "Hello World L ptr", nLen );
  verifyu16str( u16_ptr, "Hello World u ptr", nLen );
  verifyu32str( u32_ptr, "Hello World U ptr", nLen );
}


void TestArrString() {
  char16_t*  u16_aa[]      = { u"Hello", u"World" };
  char16_t   u16_alim[12] = u"Hello World";
  char16_t   u16_alimsize[sizeof(u"Hello World")/sizeof(unsigned short)] = u"Hello World";

  verifyu16str( u16_aa[0], "Hello", sizeof("Hello") );
  verifyu16str( u16_aa[1], "World", sizeof("World") );
  verifyu16str( u16_alim, "Hello World", sizeof("Hello World")-1 );
  verifyu16str( u16_alimsize, "Hello World", sizeof("Hello World") );
}


typedef struct {
  const char*             m_s;
  const wchar_t*          m_w;
  const unsigned short*   m_u16;
  const unsigned int*     m_u32;
} struct_t;


void TestStructString() {

  struct_t o = { "Hello", L"Hello", u"Hello", U"Hello" };
  int nLen = sizeof("Hello");

  verifycstr( o.m_s, "Hello", nLen );
  verifywstr( o.m_w, "Hello", nLen );
  verifyu16str( o.m_u16, "Hello", nLen );
  verifyu16str( o.m_u16, "Hello", nLen );
}

void TestLocaleChar() {
  #ifdef UGCC_LANG_UTF8
    wchar_t w_cmp = 0x20AC;
    verifycharw( u'鈧?, w_cmp);
    verifycharw( U'鈧?, w_cmp);
  #endif

  #ifdef UGCC_LANG_SJIS
    wchar_t w_cmp = 0xFF42;
    verifycharw( u'倐', w_cmp);
    verifycharw( U'倐', w_cmp);
  #endif

  #ifdef UGCC_LANG_LATIN1
    wchar_t w_cmp = 0x00D1;
    verifycharw( u'?, w_cmp);
    verifycharw( U'?, w_cmp);
  #endif
    
  #ifdef UGCC_LANG_LATIN2
    wchar_t w_cmp= 0x0143;
    verifycharw( u'?, w_cmp);
    verifycharw( U'?, w_cmp);
  #endif
}

void TestLocaleString() {
  #ifdef UGCC_LANG_UTF8
    wchar_t w_cmp[] = { 0x20AC };
    verifyu16strw(u"鈧?, w_cmp, 1 );
    verifyu32strw(U"鈧?, w_cmp, 1 );
  #endif

  #ifdef UGCC_LANG_SJIS
    wchar_t w_cmp[] = { 0xFF42 };
    verifyu16strw(u"倐", w_cmp, 1 );
    verifyu32strw(U"倐", w_cmp, 1 );
  #endif

  #ifdef UGCC_LANG_LATIN1
    wchar_t w_cmp[] = { 0x00D1, 0x00D2 };
    verifyu16strw(u"岩", w_cmp, 2 );
    verifyu32strw(U"岩", w_cmp, 2 );
  #endif
    
  #ifdef UGCC_LANG_LATIN2
    wchar_t w_cmp[] = { 0x0143, 0x0147 };
    verifyu16strw(u"岩", w_cmp, 2 );
    verifyu32strw(U"岩", w_cmp, 2 );
  #endif
}

int main(void) {

  TestChar();
  TestSingleString();
  TestCombinedString();
  TestPtrString();
  TestArrString();
  TestStructString();
  TestLocaleString();
  TestLocaleChar();

  if( g_nFailure==0 ) {
    printf( "Test OK\n" );
  } else {
    printf( "Test FAILED. %d failures occured\n", g_nFailure );
    abort();
  }
}

