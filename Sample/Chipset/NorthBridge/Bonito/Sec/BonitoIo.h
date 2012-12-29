#ifndef _BONITO_IO_H_
#define _BONITO_IO_H_


#define IO_READ8(port) \
  li    a0, port;      \
  jal   IoRead8;       \
  nop;
 
#define IO_READ16(port) \
  li    a0, port;      \
  jal   IoRead16;       \
  nop;
 
#define IO_READ32(port) \
  li    a0, port;      \
  jal   IoRead32;       \
  nop;
 
#define IO_WRITE8(port, data) \
  li    a0, port;   \
  li    a1, data;   \
  jal   IoWrite8; \
  nop;

#define IO_WRITE82(port) \
  li    a0, port;   \
  jal   IoWrite8; \
  nop;
  
#define IO_WRITE16(port, data) \
  li    a0, port;   \
  li    a1, data;   \
  jal   IoWrite16; \
  nop;
  
#define IO_WRITE32(port, data) \
  li    a0, port;   \
  li    a1, data;   \
  jal   IoWrite32; \
  nop;

#define IO_MASK_SET8(port, data) \
  li    a0, port;   \
  li    a1, data;   \
  jal   IoMaskSet8; \
  nop;

#define IO_MASK_SET16(port, data) \
  li    a0, port;   \
  li    a1, data;   \
  jal   IoMaskSet16; \
  nop;

#define IO_MASK_SET32(port, data) \
  li    a0, port;   \
  li    a1, data;   \
  jal   IoMaskSet32; \
  nop;

#define IO_MASK_CLR8(port, data) \
  li    a0, port;   \
  li    a1, data;   \
  jal   IoMaskClr8; \
  nop;

#define IO_MASK_CLR16(port, data) \
  li    a0, port;   \
  li    a1, data;   \
  jal   IoMaskClr16; \
  nop;

#define IO_MASK_CLR32(port, data) \
  li    a0, port;   \
  li    a1, data;   \
  jal   IoMaskClr32; \
  nop;

  
#endif /* _BONITO_IO_H_ */
