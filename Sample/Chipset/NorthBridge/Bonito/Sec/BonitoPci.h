#ifndef _BONITO_PCI_H_
#define _BONITO_PCI_H_

#define  BONITO_PCI_CFG_ADDR(idsel, func, reg)   ( (1 << (11 + (idsel))) + ((func) << 8) + (reg) )

#define PCI_READ32(idsel, func, reg) \
  li    a0, BONITO_PCI_CFG_ADDR(idsel, func, reg); \
  jal   PciRead32; \
  nop;

#define PCI_WRITE32(idsel, func, reg, data) \
  li    a0, BONITO_PCI_CFG_ADDR(idsel, func, reg); \
  li    a1, data;   \
  jal   PciWrite32; \
  nop;
  
#endif /* _BONITO_PCI_H_ */
