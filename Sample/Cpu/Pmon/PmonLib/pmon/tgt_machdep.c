#include <endian.h>
#include <pcivar.h>
#include <types.h>
#include "libc.h"
#include "pci_machdep.h"
#include "cmdtable.h"
#include "tgt_machdep.h"
#include "mips_machdep.h"
#include "cfb_console.h"
#include "kbd.h"

void PmonEntry(void)
{
    init_cmd();

	_pci_businit(1);
	_pci_devinit(1);

	vga_init();	
    
	KbdInit();

  //InitShell();	
}



#define	HIGH_PORT	0x0381
#define	LOW_PORT	0x0382
#define	DATA_PORT	0x0383

void
tgt_reboot(void)
{
	pci_io_write8(0xf4, HIGH_PORT);
	pci_io_write8(0xec, LOW_PORT);
	pci_io_write8(0x01, DATA_PORT);

	while(1);
}

void
tgt_poweroff(void)
{
	UINTN temp;

	temp = pci_mem_read32(0xbfe0011c);
	temp &= ~0x00000001;
	pci_mem_write32(temp, 0xbfe0011c);

	temp = pci_mem_read32(0xbfe00120);
	temp &= ~0x00000001;
	pci_mem_write32(temp, 0xbfe00120);

	while(1);

}








