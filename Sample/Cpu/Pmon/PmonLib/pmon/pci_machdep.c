/*	$Id: pci_machdep.c,v 1.3 2006/07/20 09:37:06 cpu Exp $ */

/*
 * Copyright (c) 2001 Opsycon AB  (www.opsycon.se)
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Opsycon AB, Sweden.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */



#include <pcivar.h>
#include "pci_machdep.h"
#include "libc.h"
#include <bonitopmon.h>
#include "ArchDefs.h"

/*****************************************************************/

extern int _pciverbose;

extern unsigned long long Load64(unsigned long long Address);

struct pci_device *_pci_bus[16];
int _max_pci_bus = 0;


/* PCI mem regions in PCI space */

/* soft versions of above */
static pcireg_t pci_local_mem_pci_base;


int pciinfoshow()
{
  unsigned int Data;
  unsigned int Index;
  unsigned long long Data64;
  unsigned long long Addr64;


  Data = BONITO_PCIMAP;
  printf("PCIMap = 0x%08x\n", Data);
  printf("trans lo0 = 0x%08x\n", (Data & (0x3f <<  0)) << 26);
  printf("trans lo1 = 0x%08x\n", (Data & (0x3f <<  6)) << 20);
  printf("trans lo2 = 0x%08x\n", (Data & (0x3f << 12)) << 14);

  for (Index = 0; Index < 12; Index+=4) {
    printf("Hit%d = 0x%08x_%08x\n", Index/4, BONITO_PCI_REG(0x14 + Index), BONITO_PCI_REG(0x10 + Index));
    printf("Bar%d = 0x%08x_%08x\n", Index/4, BONITO(BONITO_REGBASE + 0x54 + Index), BONITO(BONITO_REGBASE + 0x50 + Index));
    printf("\n");
  }



  Addr64 = 0x90000000;
  Addr64 = (Addr64 << 32) | 0x3ff00000;
  

  for (Index = 0; Index < 4; Index ++) {
    Data64 = Load64(Addr64 + Index * 8);
    printf("CPU Win%d Base = 0x%08x_%08x\n", Index, (UINT32)(Data64>>32), (UINT32)Data64);
    
    Data64 = Load64(Addr64 + Index * 8 + 0x20);
    printf("CPU Win%d Mask = 0x%08x_%08x\n", Index, (UINT32)(Data64>>32), (UINT32)Data64);

    Data64 = Load64(Addr64 + Index * 8 + 0x40);
    printf("CPU Win%d MMap = 0x%08x_%08x\n", Index, (UINT32)(Data64>>32), (UINT32)Data64);
    printf("\n");
  }

  Addr64 = 0x90000000;
  Addr64 = (Addr64 << 32) | 0x3ff00060;


  for (Index = 0; Index < 4; Index ++) {
    Data64 = Load64(Addr64 + Index * 8);
    printf("PCI Win%d Base = 0x%08x_%08x\n", Index, (UINT32)(Data64>>32), (UINT32)Data64);
    
    Data64 = Load64(Addr64 + Index * 8 + 0x20);
    printf("PCI Win%d Mask = 0x%08x_%08x\n", Index, (UINT32)(Data64>>32), (UINT32)Data64);

    Data64 = Load64(Addr64 + Index * 8 + 0x40);
    printf("PCI Win%d MMap = 0x%08x_%08x\n", Index, (UINT32)(Data64>>32), (UINT32)Data64);
    printf("\n");
  }
  
	return 0;
}






/***********************************************
 *
 * 硬件相关资源配置
 *
 **********************************************/
int _pci_hwinit(int initialise, bus_space_tag_t iot,bus_space_tag_t memt)
{
	struct pci_device *pd;
	struct pci_bus *pb;



	if (!initialise) {
		return(0);
	}

	pci_local_mem_pci_base = PCI_LOCAL_MEM_PCI_BASE;
	/*
	 *  Allocate and initialize PCI bus heads.
	 */

	/*
	 * PCI Bus 0
	 */
	pd = pmalloc(sizeof(struct pci_device));
	pb = pmalloc(sizeof(struct pci_bus));
	if(pd == NULL || pb == NULL) {
		printf("pci: can't alloc memory. pci not initialized\n");
		return(-1);
	}

	pd->pa.pa_flags = PCI_FLAGS_IO_ENABLED | PCI_FLAGS_MEM_ENABLED;
	
	//
	// PCI_IO_BASE(ISA_IO_BASE)
	//
	pd->pa.pa_iot = pmalloc(sizeof(bus_space_tag_t));
	pd->pa.pa_iot->bus_reverse = 1;
	pd->pa.pa_iot->bus_base = BONITO_PCIIO_BASE_VA;
	printf("PCI_IO_BASE=0x%x\n",pd->pa.pa_iot->bus_base);

	//
	// PCI_MEM_BASE
	//
	pd->pa.pa_memt = pmalloc(sizeof(bus_space_tag_t));
	pd->pa.pa_memt->bus_reverse = 1;
	pd->pa.pa_memt->bus_base = BONITO_PCILO_BASE_VA; // 3*64M
	printf("PCI_MEM_BASE=0x%x\n",pd->pa.pa_memt->bus_base);

	pd->pa.pa_dmat = NULL;
	pd->bridge.secbus = pb;
	_pci_head = pd;


	//
	// CPU空间 PCI MEM资源分配
	//
	/* first window(PCI MEM Lo0) is reserved for vga mem */
	pb->minpcimemaddr  = PCI_MEM_SPACE_PCI_BASE+0x04000000;
	pb->nextpcimemaddr = PCI_MEM_SPACE_PCI_BASE+0x08000000; // PCI MEM Lo1 + Lo2 = 128MByte


	//
	// Bonito MAP设置
	//
    /*set Bonito register; first windows for 0-0x4000000 pci mem */
	BONITO_PCIMAP =
	    BONITO_PCIMAP_WIN(0, PCI_MEM_SPACE_PCI_BASE+0x00000000) |	
	    BONITO_PCIMAP_WIN(1, PCI_MEM_SPACE_PCI_BASE+0x04000000) |
	    BONITO_PCIMAP_WIN(2, PCI_MEM_SPACE_PCI_BASE+0x08000000) |
	    BONITO_PCIMAP_PCIMAP_2;


	//
	// CPU空间，PCI IO 资源分配
	//
	pb->minpciioaddr  = PCI_IO_SPACE_BASE+0x000b000;
	pb->nextpciioaddr = PCI_IO_SPACE_BASE+ BONITO_PCIIO_SIZE;

	
	pb->pci_mem_base   = BONITO_PCILO_BASE_VA;
	pb->pci_io_base    = BONITO_PCIIO_BASE_VA;
	pb->max_lat = 255;
	pb->fast_b2b = 1;
	pb->prefetch = 1;
	pb->bandwidth = 0x4000000;
	pb->ndev = 1;
	_pci_bushead = pb;
	_pci_bus[_max_pci_bus++] = pd;

	
//	bus_dmamap_tag._dmamap_offs = 0;

	//
	//  Bonito PCI 访问CPU空间设置
	//
	BONITO_PCIBASE0 = 0x80000000;
	BONITO_PCIBASE1 = 0x0;
	BONITO_PCIBASE2 = 0x70000000;
	BONITO_PCI_REG(0x40) = 0x80000000;
	BONITO_PCI_REG(0x44) =  0xf0000000;
	/* pci base0/1 can access 256M sdram */
	BONITO_PCIMEMBASECFG = 0;


  //pciinfoshow();
  
	return(1);
}


/*
 * Called to reinitialise the bridge after we've scanned each PCI device
 * and know what is possible. We also set up the interrupt controller
 * routing and level control registers.
 */
void
_pci_hwreinit (void)
{
}

void
_pci_flush (void)
{
}

/*
 *  Map the CPU virtual address of an area of local memory to a PCI
 *  address that can be used by a PCI bus master to access it.
 */
vm_offset_t
_pci_dmamap(va, len)
	vm_offset_t va;
	UINTN len;
{
	return(pci_local_mem_pci_base + VA_TO_PA (va));
}




/*
 *  Make pci tag from bus, device and function data.
 */
pcitag_t
_pci_make_tag(bus, device, function)
	int bus;
	int device;
	int function;
{
	pcitag_t tag;

	tag = (bus << 16) | (device << 11) | (function << 8);
	return(tag);
}

/*
 *  Break up a pci tag to bus, device function components.
 */
void
_pci_break_tag(tag, busp, devicep, functionp)
	pcitag_t tag;
	int *busp;
	int *devicep;
	int *functionp;
{
	if (busp) {
		*busp = (tag >> 16) & 255;
	}
	if (devicep) {
		*devicep = (tag >> 11) & 31;
	}
	if (functionp) {
		*functionp = (tag >> 8) & 7;
	}
}

/***********************************************
 *
 * 总线上第一个设备是自身
 *
 **********************************************/
int _pci_canscan (pcitag_t tag)
{
	int bus, device, function;

	_pci_break_tag (tag, &bus, &device, &function); 
	if((bus == 0 || bus == 1) && device == 0) {
		return(0);		/* Ignore the Discovery itself */
	}
	return (1);
}


pcireg_t
_pci_conf_read(pcitag_t tag,int reg)
{
	return _pci_conf_readn(tag,reg,4);
}

pcireg_t _pci_conf_readn(pcitag_t tag, int reg, int width)
{
    UINT32 addr, type;
    pcireg_t data;
    int bus, device, function;

    if ((reg & (width-1)) || reg < 0 || reg >= 0x100) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_read: bad reg 0x%x\n", reg);
	return ~0;
    }

    _pci_break_tag (tag, &bus, &device, &function); 
    if (bus == 0) {
	/* Type 0 configuration on onboard PCI bus */
	if (device > 20 || function > 7)
	    return ~0;		/* device out of range */
	addr = (1 << (device+11)) | (function << 8) | reg;
	type = 0x00000;   // PCI 配置类型
    }
    else {
	/* Type 1 configuration on offboard PCI bus */
	if (bus > 255 || device > 31 || function > 7)
	    return ~0;	/* device out of range */
	addr = (bus << 16) | (device << 11) | (function << 8) | reg;
	type = 0x10000;
    }


 
    /* clear aborts */
    BONITO_PCICMD |= PCI_STATUS_MASTER_ABORT | PCI_STATUS_MASTER_TARGET_ABORT;

    BONITO_PCIMAP_CFG = (addr >> 16) | type;

    data = *(volatile pcireg_t *)PHYS_TO_UNCACHED(BONITO_PCICFG_BASE | (addr & 0xfffc));

    /* move data to correct position */
    data = data >> ((addr & 3) << 3);

    if (BONITO_PCICMD & PCI_STATUS_MASTER_ABORT) {
	BONITO_PCICMD |= PCI_STATUS_MASTER_ABORT;
	return ~0;
    }

    if (BONITO_PCICMD & PCI_STATUS_MASTER_TARGET_ABORT) {
	BONITO_PCICMD |= PCI_STATUS_MASTER_TARGET_ABORT;
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_read: target abort\n");
	return ~0;
    }

    return data;
}


extern UINT32 GetMemorySize(void);

unsigned int pci_type0(void)
{
  UINT32 addr, type;
  unsigned int data;
  int bus, device, function, reg;
  pcitag_t  tag;

  bus =0;
  device = 14;
  function = 0;
  reg = 0;

  tag = _pci_make_tag(bus, device, function);


	addr = (1 << (device+11)) | (function << 8) | reg;
	type = 0x00000;   // PCI 配置类型

  printf("addr = 0x%x\n", addr);

  /* clear aborts */
  BONITO_PCICMD |= PCI_STATUS_MASTER_ABORT | PCI_STATUS_MASTER_TARGET_ABORT;

  BONITO_PCIMAP_CFG = (addr >> 16) | type;

  data = *(volatile unsigned int *)PHYS_TO_UNCACHED(BONITO_PCICFG_BASE | (addr & 0xfffc));

  /* move data to correct position */
  data = data >> ((addr & 3) << 3);

  if (BONITO_PCICMD & PCI_STATUS_MASTER_ABORT) {
    BONITO_PCICMD |= PCI_STATUS_MASTER_ABORT;
    _pci_tagprintf (tag, "_pci_conf_read: abort\n");
    return ~0;
  }

  if (BONITO_PCICMD & PCI_STATUS_MASTER_TARGET_ABORT) {
    BONITO_PCICMD |= PCI_STATUS_MASTER_TARGET_ABORT;
    _pci_tagprintf (tag, "_pci_conf_read: target abort\n");
    return ~0;
  }

  _pci_tagprintf (tag, "type 0 reg 0 data = 0x%x\n", data);

  printf("MemSize = %x\n",GetMemorySize());
  return data;
}




unsigned int pci_type1(void)
{
  UINT32 addr, type;
  unsigned int data;
  int bus, device, function, reg;
  pcitag_t  tag;



  bus =0;
  device = 14;
  function = 0;
  reg = 0;

  tag = _pci_make_tag(bus, device, function);

	addr = (bus << 16) | (device << 11) | (function << 8) | reg;
	type = 0x10000;

  /* clear aborts */
  BONITO_PCICMD |= PCI_STATUS_MASTER_ABORT | PCI_STATUS_MASTER_TARGET_ABORT;

  BONITO_PCIMAP_CFG = (addr >> 16) | type;

  data = *(volatile unsigned int *)PHYS_TO_UNCACHED(BONITO_PCICFG_BASE | (addr & 0xfffc));

  /* move data to correct position */
  data = data >> ((addr & 3) << 3);

  if (BONITO_PCICMD & PCI_STATUS_MASTER_ABORT) {
    BONITO_PCICMD |= PCI_STATUS_MASTER_ABORT;
    _pci_tagprintf (tag, "_pci_conf_read: abort\n");
    return ~0;
  }

  if (BONITO_PCICMD & PCI_STATUS_MASTER_TARGET_ABORT) {
    BONITO_PCICMD |= PCI_STATUS_MASTER_TARGET_ABORT;
    _pci_tagprintf (tag, "_pci_conf_read: target abort\n");
    return ~0;
  }

  _pci_tagprintf (tag, "type 0 reg 0 data = 0x%x\n", data);
  
  return data;
}





void
_pci_conf_write(pcitag_t tag, int reg, pcireg_t data)
{
	return _pci_conf_writen(tag,reg,data,4);
}

void _pci_conf_writen(pcitag_t tag, int reg, pcireg_t data,int width)
{
    UINT32 addr, type;
    int bus, device, function;

    if ((reg &(width-1)) || reg < 0 || reg >= 0x100) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_write: bad reg %x\n", reg);
	return;
    }

    _pci_break_tag (tag, &bus, &device, &function);

    if (bus == 0) {
	/* Type 0 configuration on onboard PCI bus */
	if (device > 20 || function > 7)
	    return;		/* device out of range */
	addr = (1 << (device+11)) | (function << 8) | reg;
	type = 0x00000;
    }
    else {
	/* Type 1 configuration on offboard PCI bus */
	if (bus > 255 || device > 31 || function > 7)
	    return;	/* device out of range */
	addr = (bus << 16) | (device << 11) | (function << 8) | reg;
	type = 0x10000;
    }



    /* clear aborts */
    BONITO_PCICMD |= PCI_STATUS_MASTER_ABORT | PCI_STATUS_MASTER_TARGET_ABORT;

    BONITO_PCIMAP_CFG = (addr >> 16);

    {
      pcireg_t ori = *(volatile pcireg_t *)PHYS_TO_UNCACHED(BONITO_PCICFG_BASE | (addr & 0xfffc));
      pcireg_t mask = 0x0;

      if (width == 2) {
	if (addr & 3) mask = 0xffff; 
	else mask = 0xffff0000;
      }else if (width == 1) {
	if ((addr & 3) == 1) {
	  mask = 0xffff00ff;
	}else if ((addr & 3) == 2) {
	  mask = 0xff00ffff;
	}else if ((addr & 3) == 3) {
	  mask = 0x00ffffff;
	}else{
	  mask = 0xffffff00;
	}
      }

      data = data << ((addr & 3) << 3);
      data = (ori & mask) | data;
      *(volatile pcireg_t *)PHYS_TO_UNCACHED(BONITO_PCICFG_BASE | (addr & 0xfffc)) = data;
    }


    if (BONITO_PCICMD & PCI_STATUS_MASTER_ABORT) {
	BONITO_PCICMD |= PCI_STATUS_MASTER_ABORT;
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_write: master abort\n");
    }

    if (BONITO_PCICMD & PCI_STATUS_MASTER_TARGET_ABORT) {
	BONITO_PCICMD |= PCI_STATUS_MASTER_TARGET_ABORT;
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_write: target abort\n");
    }
}


UINT8 pci_io_read8(UINTN port)
{
        return (*(volatile UINT8 *)(BONITO_PCIIO_BASE_VA + port));
}

UINT16 pci_io_read16(UINTN port)
{
        return (*(volatile UINT16 *)(BONITO_PCIIO_BASE_VA + port));
}
UINT32 pci_io_read32(UINTN port)
{
        return (*(volatile UINT32 *)(BONITO_PCIIO_BASE_VA + port));
}

void pci_io_write8(UINT8 val,UINTN port)
{
	*(volatile UINT8 *)(BONITO_PCIIO_BASE_VA + (port)) = (val);
}
void pci_io_write16(UINT16 val,UINTN port)
{
	*(volatile UINT16 *)(BONITO_PCIIO_BASE_VA + (port)) = (val);
}

void pci_io_write32(UINT32 val,UINTN port)
{
	*(volatile UINT32 *)(BONITO_PCIIO_BASE_VA + (port)) = (val);
}


UINT8 pci_mem_read8(UINTN addr)
{
        return (*(volatile UINT8 *)(addr));
}

UINT16 pci_mem_read16(UINTN addr)
{
        return (*(volatile UINT16 *)(addr));
}
UINT32 pci_mem_read32(UINTN addr)
{
        return (*(volatile UINT32 *)(addr));
}

void pci_mem_write8(UINT8 val,UINTN addr)
{
	*(volatile UINT8 *)(addr) = (val);
}
void pci_mem_write16(UINT16 val,UINTN addr)
{
	*(volatile UINT16 *)(addr) = (val);
}

void pci_mem_write32(UINT32 val,UINTN addr)
{
	*(volatile UINT32 *)(addr) = (val);
}

