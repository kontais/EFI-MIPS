#ifndef __PCI_MACHDEP_H__
#define __PCI_MACHDEP_H__

#include <pcivar.h>

void	 _pci_flush __P((void));
int	     _pci_hwinit __P((int, bus_space_tag_t, bus_space_tag_t));
void	 _pci_hwreinit __P((void));
pcitag_t _pci_make_tag __P((int, int, int));
void	 _pci_break_tag __P((pcitag_t, int *, int *, int *));
pcireg_t _pci_conf_readn __P((pcitag_t, int,int));
void	 _pci_conf_writen __P((pcitag_t, int, pcireg_t,int));
pcireg_t _pci_conf_read __P((pcitag_t, int));
void	 _pci_conf_write __P((pcitag_t, int, pcireg_t));
void	 _pci_businit __P((int));
void	 _pci_devinit __P((int));
vm_offset_t _pci_dmamap __P((vm_offset_t, UINTN));

void	 _pci_bdfprintf (int bus, int device, int function, const char *fmt, ...);
void	 _pci_tagprintf (pcitag_t tag, const char *fmt, ...);
int	     _pci_canscan __P((pcitag_t tag));

UINT8  pci_io_read8(UINTN port);
UINT16 pci_io_read16(UINTN port);
UINT32 pci_io_read32(UINTN port);

void pci_io_write8(UINT8   val,UINTN port);
void pci_io_write16(UINT16 val,UINTN port);
void pci_io_write32(UINT32 val,UINTN port);


UINT8  pci_mem_read8(UINTN addr);
UINT16 pci_mem_read16(UINTN addr);
UINT32 pci_mem_read32(UINTN port);
void pci_mem_write8(UINT8 val,UINTN addr);
void pci_mem_write16(UINT16 val,UINTN addr);
void pci_mem_write32(UINT32 val,UINTN addr);


extern struct pci_device *_pci_bus[];

#endif
