
int
main ()
{
  unsigned char *Iterator = "\x6\x8";

    switch (*(Iterator++)) {
      
      case (0x03):    
      case (0x04):     
        return 0x1;
        
      case (0x08):
        return 0x2;

      case (0x06):
      case (0x07):
        return 0x3;

      default:
        return 0x4;
        break;
    }
}

