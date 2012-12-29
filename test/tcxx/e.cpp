#include <iostream>
using namespace std;


//typedef char int8_t;
//typedef int8_t INT8;
typedef char INT8;


class CRectangle {
  int x, y;
  public:
  int area(void) {return (x*y);}

  void 
    set_values ( 
      int a,
      int b) 
  {  
    x = a;  
    y = b;  
    return ;
  }
void CRPrint(INT8 *s1, INT8 *s2)
{
  return ;
}
};

int main()
{
  char ss[] = "abcd";
  CRectangle rect;


  rect.set_values(3,4);
  cout << "area: " << rect.area() << endl;
  rect.CRPrint(ss,"ced");
  return 0;
}


