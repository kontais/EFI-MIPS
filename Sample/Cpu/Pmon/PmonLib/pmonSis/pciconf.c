/*	$Id: pciconf.c,v 1.2 2006/07/20 09:37:06 cpu Exp $ */

/*
 * Copyright (c) 2000 Opsycon AB  (www.opsycon.se)
 * Copyright (c) 2000 Rtmx, Inc   (www.rtmx.com)
 * Copyright (c) 2001 IP Unplugged AB  (www.ipunplugged.com)
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
 *	This product includes software developed for Rtmx, Inc by
 *	Opsycon Open System Consulting AB, Sweden.
 *      This product includes software developed for IP Inplugged AB, by
 *      Patrik Lindergren.
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

/*
 *  This work is derivate work created from code written at Algorithmics UK.
 *  It was part of PMON which is not copyrighted. After modifications and
 *  extensions it is now released under standard BSD copyright.
 */

/*
 * pciconf.c: generic PCI bus configuration
 */

#include "libc.h"
#include "pcivar.h"
#include "pci_machdep.h"

#define PCIVERBOSE 5

#define PCI_PRINTF printf
#define PCI_VPRINTF vprintf


static int _pci_roundup(int , int );
static int _pci_getIntRouting(struct pci_device *);
static int _pci_setupIntRouting(struct pci_device *);
static void _pci_scan_dev(struct pci_device *dev, int bus, int device, int initialise);
static void _insertsort_window(struct pci_win **, struct pci_win *);
static void _pci_device_insert(struct pci_device *parent, struct pci_device *device);
static pcireg_t _pci_allocate_mem __P((struct pci_device *, vm_size_t));
static pcireg_t _pci_allocate_io __P((struct pci_device *, vm_size_t));
static void _setup_pcibuses(int );
static void _pci_bus_insert(struct pci_bus *);



pcitag_t have_vga = 0;			/* Have tag if VGA board found */
int monarch_mode = 1;			/* Default as master on the bus! */
int pci_roots;				/* How many pci roots to init */
int _pciverbose = PCIVERBOSE;
static int _pci_nbus = 8;		/* Allow for eight roots */
struct tgt_bus_space def_bus_iot;		/* Default bus tags */
struct tgt_bus_space def_bus_memt;		/* Default bus tags */
struct pci_device *_pci_head;
struct pci_bus *_pci_bushead;
struct pci_intline_routing *_pci_inthead;
struct pci_device *vga_dev;


static void
print_bdf (int bus, int device, int function)
{
    PCI_PRINTF ("PCI");
    PCI_PRINTF (" bus %d", bus);
    if (device >= 0)
	PCI_PRINTF (" device %d", device);
    if (function >= 0)
	PCI_PRINTF (" function %d", function);
    PCI_PRINTF (": ");
}

void
_pci_bdfprintf (int bus, int device, int function, const char *fmt, ...)
{
    VA_LIST arg;

    print_bdf (bus, device, function);
    VA_START(arg, fmt);
    PCI_VPRINTF (fmt, arg);
    VA_END(arg);
}

void
_pci_tagprintf (pcitag_t tag, const char *fmt, ...)
{
    VA_LIST arg;
    int bus, device, function;

    _pci_break_tag (tag, &bus, &device, &function); 
    print_bdf (bus, device, function);

    VA_START(arg, fmt);
    PCI_VPRINTF (fmt, arg);
    VA_END(arg);
}

/***********************************************
 *
 * 查询设备功能
 *
 **********************************************/

static void
_pci_query_dev_func (struct pci_device *dev, pcitag_t tag, int initialise)
{
	pcireg_t id, class;
	pcireg_t old, mask;
	pcireg_t stat;
	pcireg_t bparam;
	int reg;
	struct pci_bus *pb;
	struct pci_device *pd;
	UINTN x;
	int bus, device, function;
    
	class = _pci_conf_read(tag, PCI_CLASS_REG);  // 设备功能类别 CLASS
	id = _pci_conf_read(tag, PCI_ID_REG);        // 再读一次VID/DID


	//
	// 添加一个PCI设备
	//
	pd = pmalloc(sizeof(struct pci_device));
	if(pd == NULL) {
		PCI_PRINTF ("pci: can't alloc memory for device\n");
		return;
	}
        
	_pci_break_tag (tag, &bus, &device, &function);

	pd->pa.pa_bus = bus;
	pd->pa.pa_device = device;
	pd->pa.pa_function = function;
	pd->pa.pa_tag = tag;
	pd->pa.pa_id = id;
	pd->pa.pa_class = class;
	pd->pa.pa_flags = PCI_FLAGS_IO_ENABLED | PCI_FLAGS_MEM_ENABLED;
	pd->pa.pa_iot = dev->pa.pa_iot;
	pd->pa.pa_memt = dev->pa.pa_memt;
	pd->pa.pa_dmat = dev->pa.pa_dmat;
	pd->parent = dev;
	pd->pcibus = dev->bridge.secbus;
	pb = pd->pcibus;
	_pci_device_insert(dev, pd);
	PCI_PRINTF("Insert to bus 0x%x\n",dev->bridge.secbus);


	

	//
	//  Calculated Interrupt routing
	// 
	_pci_setupIntRouting(pd);
	PCI_PRINTF("Setup Routing\n");

	//
	//  Shut off device if we initialize from non reset.
	//
	stat = _pci_conf_read(tag, PCI_COMMAND_STATUS_REG);
	stat &= ~(PCI_COMMAND_MASTER_ENABLE |
		      PCI_COMMAND_IO_ENABLE |
		      PCI_COMMAND_MEM_ENABLE);
	_pci_conf_write(tag, PCI_COMMAND_STATUS_REG, stat);
	pd->stat = stat;


	
    //
	// do all devices support fast back-to-back
	//
	if ((stat & PCI_STATUS_BACKTOBACK_SUPPORT) == 0) {
		pb->fast_b2b = 0;		/* no, sorry */
	}

	/* do all devices run at 66 MHz */
	if ((stat & PCI_STATUS_66MHZ_SUPPORT) == 0) {
		pb->freq66 = 0; 		/* no, sorry */
	}

	/* find slowest devsel */
	x = stat & PCI_STATUS_DEVSEL_MASK;
	if (x > pb->devsel) {
		pb->devsel = x;
	}

	/* Funny looking code which deals with any 32bit read only cfg... */
	bparam = _pci_conf_read(tag, (PCI_MINGNT & ~0x3));
	pd->min_gnt = 0xff & (bparam >> ((PCI_MINGNT & 3) * 8));
	bparam = _pci_conf_read(tag, (PCI_MAXLAT & ~0x3));
	pd->max_lat = 0xff & (bparam >> ((PCI_MAXLAT & 3) * 8));

	if (pd->min_gnt != 0 || pd->max_lat != 0) {
		/* find largest minimum grant time of all devices */
		if (pd->min_gnt != 0 && pd->min_gnt > pb->min_gnt) {
			pb->min_gnt = pd->min_gnt;
		}
	
		/* find smallest maximum latency time of all devices */
		if (pd->max_lat != 0 && pd->max_lat < pb->max_lat) {
			    pb->max_lat = pd->max_lat;
		}
	
		/* subtract our min on-bus time per second from bus bandwidth */
		if (initialise) {
			pb->bandwidth -= pd->min_gnt * 4000000 / (pd->min_gnt + pd->max_lat);
		}
	}

	//
	// Map interrupt to interrupt line (software function only)
	//
	bparam = _pci_conf_read(tag, PCI_INTERRUPT_REG);
	bparam &= ~(PCI_INTERRUPT_LINE_MASK << PCI_INTERRUPT_LINE_SHIFT);
	bparam |= ((_pci_getIntRouting(pd) & 0xff) << PCI_INTERRUPT_LINE_SHIFT);
	_pci_conf_write(tag, PCI_INTERRUPT_REG, bparam);

	//
	//  Check to see if device is a PCI Bridge
	// 
	if (PCI_ISCLASS(class, PCI_CLASS_BRIDGE, PCI_SUBCLASS_BRIDGE_PCI)) {
		struct pci_device *pcidev;
		struct pci_win *pm_mem = NULL;
		struct pci_win *pm_io = NULL;
		struct pci_win *pm;
		pcireg_t tmp;

		PCI_PRINTF("Find a pci bridge\n");

		pd->bridge.pribus_num = bus;
		pd->bridge.secbus_num =  ++_pci_nbus;
		/* Set it temperary to same as secondary bus number */
		pd->bridge.subbus_num =  pd->bridge.secbus_num;

		//
		// Set sub bus number
		//
		tmp = _pci_conf_read(tag, PCI_PRIBUS_1);
		tmp &= 0xff000000;
		tmp |= pd->bridge.pribus_num;
		tmp |= pd->bridge.secbus_num << 8;
		tmp |= pd->bridge.subbus_num << 16;
		_pci_conf_write(tag, PCI_PRIBUS_1, tmp);

		/* Update sub bus number */
		for(pcidev = dev; pcidev != NULL; pcidev = pcidev->parent) {
			pcidev->bridge.subbus_num = pd->bridge.secbus_num;
			tmp = _pci_conf_read(pcidev->pa.pa_tag, PCI_PRIBUS_1);
			tmp &= 0xff00ffff;
			tmp |= pd->bridge.secbus_num << 16;
			_pci_conf_write(pcidev->pa.pa_tag, PCI_PRIBUS_1, tmp);
		}

		pd->bridge.secbus = pmalloc(sizeof(struct pci_bus));
		if(pd->bridge.secbus == NULL) {
			PCI_PRINTF ("pci: can't alloc memory for new pci bus\n");
			return;
		}
        
		pd->bridge.secbus->max_lat = 255;
		pd->bridge.secbus->fast_b2b = 1;
		pd->bridge.secbus->prefetch = 1;
		pd->bridge.secbus->freq66 = 1;
		pd->bridge.secbus->bandwidth = 4000000;
		pd->bridge.secbus->ndev = 1;
		pd->bridge.secbus->bus = pd->bridge.secbus_num;

		_pci_bus_insert(pd->bridge.secbus);

		/* Scan secondary bus of the bridge */
		_pci_scan_dev(pd, pd->bridge.secbus_num, 0, initialise);

		/*
		 * Sum up the address space needed by secondary side of bridge
		 */

		/* Sum up I/O Space needed */
		for(pm = pd->bridge.iospace; pm != NULL; pm = pm->next) {
			if(pm_io == NULL) {
				pm_io = pmalloc(sizeof(struct pci_win));
				if(pm_io == NULL) {
					PCI_PRINTF ("pci: can't alloc memory for pci memory window\n");
					return;
				}
				pm_io->device = pd;
				pm_io->reg = PCI_IOBASEL_1;
				pm_io->flags = PCI_MAPREG_TYPE_IO;
			}
			pm_io->size += pm->size;
		}

		/* Sum up Memory Space needed */
		for(pm = pd->bridge.memspace; pm != NULL; pm = pm->next) {
			if(pm_mem == NULL) {
				pm_mem = pmalloc(sizeof(struct pci_win));
				if(pm_mem == NULL) {
					PCI_PRINTF ("pci: can't alloc memory for pci memory window\n");
					return;
				}
            
				pm_mem->device = pd;
				pm_mem->reg = PCI_MEMBASE_1; 
				pm_mem->flags = PCI_MAPREG_MEM_TYPE_32BIT;
			}
			pm_mem->size += pm->size;
		}
        
		/* Round to minimum granularity requierd for a bridge */
		pm_io->size = _pci_roundup(pm_io->size, 0x1000);
		pm_mem->size = _pci_roundup(pm_mem->size, 0x100000);

		if(pm_io) {
			_insertsort_window(&pd->parent->bridge.iospace, pm_io);
		}
		if(pm_mem) {
			_insertsort_window(&pd->parent->bridge.memspace,pm_mem);
		}
	}
	else if (PCI_ISCLASS(class, PCI_CLASS_MASS_STORAGE,
		    PCI_SUBCLASS_MASS_STORAGE_IDE) &&
		dev->bridge.secbus->minpciioaddr == 0) {
        /*
	 * There is no need to setup memory regions for IDE storage devices
	 * but only if PCI/ISA I/O space is accessables
	 */
	 	PCI_PRINTF("MASS STORE skip\n");
		return;

	} else {
		int skipnext = 0;

		for (reg = PCI_MAPREG_START; reg < PCI_MAPREG_END; reg += 4) {
			struct pci_win *pm;

			if (skipnext) {
				skipnext = 0;
				continue;
			}

			old = _pci_conf_read(tag, reg);
			_pci_conf_write(tag, reg, 0xffffffff);  // 写全FF
			mask = _pci_conf_read(tag, reg);
			_pci_conf_write(tag, reg, old);
			if (mask == 0 || mask == 0xffffffff) {
				//break;
				continue;
			}

			if (_pciverbose >= 3) {
				_pci_tagprintf (tag, "reg 0x%x = 0x%x\n",
							reg, mask);
			}

			if (PCI_MAPREG_TYPE(mask) == PCI_MAPREG_TYPE_IO) {
				mask |= 0xffff0000; /* must be ones */
				pm = pmalloc(sizeof(struct pci_win));
				if(pm == NULL) {
					PCI_PRINTF ("pci: can't alloc memory for pci memory window\n");
					return;
				}
            
				pm->device = pd;
				pm->reg = reg;
				pm->flags = PCI_MAPREG_TYPE_IO;
				pm->size = -(PCI_MAPREG_IO_ADDR(mask));
				_insertsort_window(&pd->parent->bridge.iospace, pm);
			}
			else {
				switch (PCI_MAPREG_MEM_TYPE(mask)) {
				case PCI_MAPREG_MEM_TYPE_32BIT:
				case PCI_MAPREG_MEM_TYPE_32BIT_1M:
					break;
				case PCI_MAPREG_MEM_TYPE_64BIT:
					_pci_conf_write(tag, reg + 4, 0x0);
					skipnext = 1;
					break;
				default:
					_pci_tagprintf (tag, "reserved mapping type 0x%x\n",
					      PCI_MAPREG_MEM_TYPE(mask));
					continue;
				}

				if  (!PCI_MAPREG_MEM_PREFETCHABLE(mask)) {
					pb->prefetch = 0;
				}

				pm = pmalloc(sizeof(struct pci_win));
				if(pm == NULL) {
					PCI_PRINTF ("pci: can't alloc memory for pci memory window\n");
					return;
				}

				pm->device = pd;
				pm->reg = reg;
				pm->flags = PCI_MAPREG_MEM_TYPE_32BIT;
				pm->size = -(PCI_MAPREG_MEM_ADDR(mask));
				_insertsort_window(&pd->parent->bridge.memspace, pm);
			}
		}

		//
		// Finally check for Expansion ROM
		//
		reg = PCI_MAPREG_ROM;
		old = _pci_conf_read(tag, reg);
		_pci_conf_write(tag, reg, 0xfffffffe);
		mask = _pci_conf_read(tag, reg);
		_pci_conf_write(tag, reg, old);

		if (mask != 0 && mask != 0xffffffff) {
			struct pci_win *pm;

			PCI_PRINTF("Have Option ROM\n");
			if (_pciverbose >= 3) {
				_pci_tagprintf (tag, "reg 0x%x = 0x%x\n", reg, mask);
			}

			pm = pmalloc(sizeof(struct pci_win));
			if(pm == NULL) {
				PCI_PRINTF ("pci: can't alloc memory for pci memory window\n");
				return;
			}
			pm->device = pd;
			pm->reg = reg;
			pm->size = -(PCI_MAPREG_ROM_ADDR(mask));
			_insertsort_window(&pd->parent->bridge.memspace, pm);
		}
	}
}

static int
_pci_roundup(value, round)
    int value;
    int round;
{
    int result = (value / round) * round;

    if(value % round)
        result += round;

    return(result);
}

static void
_pci_bus_insert(bus)
    struct pci_bus *bus;
{
    struct pci_bus *pb;

    for(pb = _pci_bushead; pb->next != NULL; pb = pb->next)
        ;;

    pb->next = bus;
}

static void
_pci_device_insert(parent, device)
    struct pci_device *parent;
    struct pci_device *device;
{
    struct pci_device *pd = parent->bridge.child;

    if(pd == NULL) {
        parent->bridge.child = device;
    } else {
    
        for(; pd->next != NULL; pd = pd->next)
            ;;

        pd->next = device;
    }
}

/***********************************************
 *
 * 对一个设备扫描
 *
 **********************************************/

static void
_pci_query_dev (struct pci_device *dev, int bus, int device, int initialise)
{
	pcitag_t tag;
	pcireg_t id;
	pcireg_t misc;
	
	tag = _pci_make_tag(bus, device, 0);
	if (!_pci_canscan (tag))
		return;

	if (_pciverbose >= 2)
		_pci_bdfprintf (bus, device, -1, "probe...");  // 打印BUS DEV FUNC

	id = _pci_conf_read(tag, PCI_ID_REG); // 读Vendor ID/Device ID

	if (_pciverbose >= 2) {
		PCI_PRINTF ("completed\n");
	}

	if (id == 0 || id == 0xffffffff) {
		return;
	}

	misc = _pci_conf_read(tag, PCI_BHLC_REG); // Head Tye,是否有多功能设备
	
	if (PCI_HDRTYPE_MULTIFN(misc)) {
		int function;
		for (function = 0; function < 8; function++) {
			tag = _pci_make_tag(bus, device, function);
			id = _pci_conf_read(tag, PCI_ID_REG);
			if (id == 0 || id == 0xffffffff) {
				//return;
				continue;
			}
			_pci_query_dev_func (dev, tag, initialise); // 递归调用
		}
	}
	else {
		_pci_query_dev_func (dev, tag, initialise); //分析功能
	}
}


static pcireg_t
_pci_allocate_mem(dev, size)
	struct pci_device *dev;
	vm_size_t size;
{
	pcireg_t address,address1;

	/* allocate upwards, then round to size boundary */
	address = (dev->bridge.secbus->minpcimemaddr + size - 1) & ~(size - 1); 
	address1 = address + size;
	if (address1 > dev->bridge.secbus->nextpcimemaddr ||
	    address1 < dev->bridge.secbus->minpcimemaddr) {
			return(-1);
	}
	dev->bridge.secbus->minpcimemaddr = address1;

	return(address);
}

static pcireg_t
_pci_allocate_io(dev, size)
    struct pci_device *dev;
    vm_size_t size;
{
	pcireg_t address,address1;

	/* allocate downwards, then round to size boundary */
	address=dev->bridge.secbus->minpciioaddr;
	address1 = (dev->bridge.secbus->minpciioaddr + size) & ~(size - 1);
	if (address1 > dev->bridge.secbus->nextpciioaddr ||
            address1 < dev->bridge.secbus->minpciioaddr) {
		return -1;
	}
	dev->bridge.secbus->minpciioaddr = address1;

	return(address);
}

static void
_insertsort_window(pm_list, pm)
    struct pci_win **pm_list;
    struct pci_win *pm;
{
	struct pci_win *pm1, *pm2;

	PCI_PRINTF("Need Space Type = 0x%x, Size = 0x%x\n",pm->flags,pm->size);
	pm1 = (struct pci_win *)pm_list;
	while((pm2 = pm1->next)) {
		if(pm->size >= pm2->size) {
			break;
		}
		pm1 = pm2;
	}

	pm1->next = pm;
	pm->next = pm2;
}


static void
_pci_setup_windows (struct pci_device *dev)
{
    struct pci_win *pm;
    struct pci_win *next;
    struct pci_device *pd;

    for(pm = dev->bridge.memspace; pm != NULL; pm = next) {

        pd = pm->device;
        next = pm->next;
        pm->address = _pci_allocate_mem (dev, pm->size);
        if (pm->address == -1) {
            _pci_tagprintf (pd->pa.pa_tag, 
                            "not enough PCI mem space (%d requested)\n", 
                            pm->size);
            continue;
        }
        if (_pciverbose >= 2)
            _pci_tagprintf (pd->pa.pa_tag, "mem @%p, %d bytes\n", pm->address, pm->size);

	if (PCI_ISCLASS(pd->pa.pa_class,
	    PCI_CLASS_BRIDGE, PCI_SUBCLASS_BRIDGE_PCI) &&
           (pm->reg == PCI_MEMBASE_1)) {

            pcireg_t memory;

	    PCI_PRINTF ("bridge class=%x\n",pd->pa.pa_class);

            pd->bridge.secbus->minpcimemaddr = pm->address;
            pd->bridge.secbus->nextpcimemaddr = pm->address + pm->size;

            memory = (((pm->address+pm->size) >> 16) << 16) | (pm->address >> 16);
            _pci_conf_write(pd->pa.pa_tag, pm->reg, memory);

        } else if (pm->reg != PCI_MAPREG_ROM) {
            /* normal memory - expansion rom done below */
            pcireg_t base = _pci_conf_read(pd->pa.pa_tag, pm->reg);
            base = pm->address | (base & ~PCI_MAPREG_MEM_ADDR_MASK);
            _pci_conf_write(pd->pa.pa_tag, pm->reg, base);
			if(_pciverbose >=2)
				_pci_tagprintf(pd->pa.pa_tag, "mem space : 0x%x\n", base);
        }
    }
    
    /* Program expansion rom address base after normal memory base, 
       to keep DEC ethernet chip happy */
    for (pm = dev->bridge.memspace; pm != NULL; pm = next) {

	pd = pm->device;
    PCI_PRINTF("_pci_setup_windows pd:%x pd->pa.pa_class:%x\n",pd,pd->pa.pa_class);
	if (PCI_ISCLASS(((pd->pa.pa_class)&0xff00ffff), PCI_CLASS_DISPLAY, PCI_SUBCLASS_DISPLAY_VGA)) 
		vga_dev = pd;
	if (pm->reg == PCI_MAPREG_ROM) {
	    /* expansion rom */
		if (_pciverbose >= 2)
		    _pci_tagprintf (pd->pa.pa_tag, "exp @%p, %d bytes\n",
			pm->address, pm->size);
	    _pci_conf_write(pd->pa.pa_tag, pm->reg, pm->address | PCI_MAPREG_TYPE_ROM);
        }
        next = pm->next;
        dev->bridge.memspace = next;
        pfree(pm);
    }

    
    for(pm = dev->bridge.iospace; pm != NULL; pm = next) {

        pd = pm->device;
        next = pm->next;
        pm->address = _pci_allocate_io (dev, pm->size);
        if (pm->address == -1) {
            _pci_tagprintf (pd->pa.pa_tag, 
                            "not enough PCI io space (%d requested)\n", 
                            pm->size);
            pfree(pm);
            continue;
        }
        if (_pciverbose >= 2)
		    _pci_tagprintf (pd->pa.pa_tag, "i/o @%p, %d bytes\n", pm->address, pm->size);

	if (PCI_ISCLASS(pd->pa.pa_class,
	    PCI_CLASS_BRIDGE, PCI_SUBCLASS_BRIDGE_PCI) &&
           (pm->reg == PCI_IOBASEL_1)) {
	    pcireg_t tmp;

            pd->bridge.secbus->minpciioaddr = pm->address;
            pd->bridge.secbus->nextpciioaddr = pm->address + pm->size;

	    tmp = _pci_conf_read(pd->pa.pa_tag,PCI_IOBASEL_1);
	    tmp &= 0xffff0000;
	    tmp |= (pm->address >> 8) & 0xf0;
	    tmp |= ((pm->address + pm->size) & 0xf000);
	    _pci_conf_write(pd->pa.pa_tag,PCI_IOBASEL_1, tmp);

	    tmp = (pm->address >> 16) & 0xffff;
	    tmp |= ((pm->address + pm->size) & 0xffff0000);
	    _pci_conf_write(pd->pa.pa_tag,PCI_IOBASEH_1, tmp);

        }
        else {
            _pci_conf_write(pd->pa.pa_tag, pm->reg, pm->address | PCI_MAPREG_TYPE_IO);
	    if(_pciverbose)
			_pci_tagprintf(pd->pa.pa_tag, "bar : 0x%x\n",  _pci_conf_read(pd->pa.pa_tag, pm->reg));
        }
        dev->bridge.iospace = next;
        pfree(pm);
    }

    /* Recursive allocate memory for secondary buses */
    for(pd = dev->bridge.child; pd != NULL; pd = pd->next) {
	if (PCI_ISCLASS(pd->pa.pa_class,
	    PCI_CLASS_BRIDGE, PCI_SUBCLASS_BRIDGE_PCI)) {
            _pci_setup_windows(pd);
        }
    }
}

/*
 * Calculate interrupt routing for a device
 */
static int
_pci_getIntRouting(struct pci_device *dev)
{
	return(dev->intr_routing[PCI_INTLINE_A]);
}

static int
_pci_setupIntRouting(struct pci_device *dev)
{
	int error = -1;
	struct pci_intline_routing *pi;
    
	for(pi = _pci_inthead; pi != NULL; pi = pi->next) {
		if(dev->parent->pa.pa_tag == _pci_make_tag(pi->bus, pi->device, pi->function)) {
			dev->intr_routing[PCI_INTLINE_A] = pi->intline[dev->pa.pa_device][PCI_INTLINE_A];
			dev->intr_routing[PCI_INTLINE_B] = pi->intline[dev->pa.pa_device][PCI_INTLINE_B];
			dev->intr_routing[PCI_INTLINE_C] = pi->intline[dev->pa.pa_device][PCI_INTLINE_C];
			dev->intr_routing[PCI_INTLINE_D] = pi->intline[dev->pa.pa_device][PCI_INTLINE_D];
			error = 0;
			break;
		}
	}
	
	/*
	 * If there where no predefines routing calculate it.
	 */
	if(error == -1) {
		switch(dev->pa.pa_device % 4) {
			case 0:
				dev->intr_routing[PCI_INTLINE_A] = dev->parent->intr_routing[PCI_INTLINE_A];
				dev->intr_routing[PCI_INTLINE_B] = dev->parent->intr_routing[PCI_INTLINE_B];
				dev->intr_routing[PCI_INTLINE_C] = dev->parent->intr_routing[PCI_INTLINE_C];
				dev->intr_routing[PCI_INTLINE_D] = dev->parent->intr_routing[PCI_INTLINE_D];
				error = 0;
				break;
			case 1:
				dev->intr_routing[PCI_INTLINE_A] = dev->parent->intr_routing[PCI_INTLINE_B];
				dev->intr_routing[PCI_INTLINE_B] = dev->parent->intr_routing[PCI_INTLINE_C];
				dev->intr_routing[PCI_INTLINE_C] = dev->parent->intr_routing[PCI_INTLINE_D];
				dev->intr_routing[PCI_INTLINE_D] = dev->parent->intr_routing[PCI_INTLINE_A];
				error = 0;
				break;
			case 2:
				dev->intr_routing[PCI_INTLINE_A] = dev->parent->intr_routing[PCI_INTLINE_C];
				dev->intr_routing[PCI_INTLINE_B] = dev->parent->intr_routing[PCI_INTLINE_D];
				dev->intr_routing[PCI_INTLINE_C] = dev->parent->intr_routing[PCI_INTLINE_A];
				dev->intr_routing[PCI_INTLINE_D] = dev->parent->intr_routing[PCI_INTLINE_B];
				error = 0;
				break;
			case 3:
				dev->intr_routing[PCI_INTLINE_A] = dev->parent->intr_routing[PCI_INTLINE_D];
				dev->intr_routing[PCI_INTLINE_B] = dev->parent->intr_routing[PCI_INTLINE_A];
				dev->intr_routing[PCI_INTLINE_C] = dev->parent->intr_routing[PCI_INTLINE_B];
				dev->intr_routing[PCI_INTLINE_D] = dev->parent->intr_routing[PCI_INTLINE_C];
				error = 0;
				break;
			default:
				break;
		}
	}

	return(error);
}

static void
_pci_setup_devices (struct pci_device *parent, int initialise)
{
    struct pci_device *pd;

    for (pd = parent->bridge.child; pd ; pd = pd->next) {
	/* set device parameters */
	struct pci_bus *pb = pd->pcibus;
	pcitag_t tag = pd->pa.pa_tag;
	pcireg_t cmd, misc, class;
	UINTN ltim;

	cmd = _pci_conf_read(tag, PCI_COMMAND_STATUS_REG);

	if (initialise) {
	    class = _pci_conf_read(tag, PCI_CLASS_REG);
	    cmd |= PCI_COMMAND_MASTER_ENABLE | 
			   PCI_COMMAND_SERR_ENABLE   | 
			   PCI_COMMAND_PARITY_ENABLE;
	    /* always enable i/o & memory space, in case this card is
	       just snarfing space from the fixed ISA block and doesn't
	       declare separate PCI space. Exception from this is if
	       it is a bridge chip which we will initialize later */
            cmd |= PCI_COMMAND_IO_ENABLE | PCI_COMMAND_MEM_ENABLE;

	    if (pb->fast_b2b)
		cmd |= PCI_COMMAND_BACKTOBACK_ENABLE;
            _pci_conf_write(tag, PCI_COMMAND_STATUS_REG, cmd);

	    ltim = 64;

	    misc = _pci_conf_read (tag, PCI_BHLC_REG);
	    misc = (misc & ~(PCI_LATTIMER_MASK << PCI_LATTIMER_SHIFT))
		   | ((ltim & 0xff) << PCI_LATTIMER_SHIFT);
	    misc = (misc & ~(PCI_CACHELINE_MASK << PCI_CACHELINE_SHIFT))
		   | ((PCI_CACHE_LINE_SIZE & 0xff) << PCI_CACHELINE_SHIFT);
	    _pci_conf_write (tag, PCI_BHLC_REG, misc);
	    
	    if(PCI_CLASS(class) == PCI_CLASS_BRIDGE ||
	       PCI_SUBCLASS(class) == PCI_SUBCLASS_BRIDGE_PCI || pd->bridge.child != NULL) {
		    _pci_setup_devices (pd, initialise); 
	    }
        }
    }
}

/***********************************************
 *
 * PCI BUS初始化
 *
 **********************************************/
void _pci_businit (int init)
{

	/* intialise the PCI bridge */
	if (/*init*/ 1) {
		init = _pci_hwinit (init, &def_bus_iot, &def_bus_memt);
		pci_roots = init;
		if (init < 1)
			return;
	}
	if(monarch_mode) {
		int i;
		struct pci_device *pb;

                if (_pciverbose) {
			PCI_PRINTF("setting up %d bus\n", init);
		}
		for(i = 0, pb = _pci_head; i < pci_roots; i++, pb = pb->next) {
			_pci_scan_dev(pb, i, 0, init);
		}
			PCI_PRINTF("----_setup_pcibuses------\n");
        	_setup_pcibuses(init);
	}
}

/***********************************************
 *
 * 每个总线的设备枚举，第一个参数是bus device
 *
 **********************************************/
static void
_pci_scan_dev(struct pci_device *dev, int bus, int device, int initialise)
{
	for(; device < 32; device++) {
		_pci_query_dev (dev, bus, device, initialise);
	}
}

static void
_setup_pcibuses(int initialise)
{
	struct pci_bus *pb;
	struct pci_device *pd;
	UINTN def_ltim, max_ltim;
	int i;


	for(pb = _pci_bushead; pb != NULL; pb = pb->next) {
    
		if (pb->ndev == 0)
			return;

		if (initialise) {
			/* convert largest minimum grant time to cycle count */
/*XXX 66/33 Mhz */	max_ltim = pb->min_gnt * 33 / 4;
        
			/* now see how much bandwidth is left to distribute */
			if (pb->bandwidth <= 0) {
				if (_pciverbose) {
					_pci_bdfprintf (pb->bus, -1, -1,
					    "WARN: total bandwidth exceeded\n");
				}
				def_ltim = 1;
			}
			else {
				/* calculate a fair share for each device */
				def_ltim = pb->bandwidth / pb->ndev;
				if (def_ltim > pb->max_lat) {
				/* would exceed critical time for some device */
					def_ltim = pb->max_lat;
				}
				/* convert to cycle count */
				def_ltim = def_ltim * 33 / 4;
			}
			/* most devices don't implement bottom three bits */
			def_ltim = (def_ltim + 7) & ~7;
			max_ltim = (max_ltim + 7) & ~7;
        
			pb->def_ltim = MIN (def_ltim, 255);
			pb->max_ltim = MIN (MAX (max_ltim, def_ltim), 255);
		}
	}

	_pci_hwreinit ();

	/* setup the individual device windows */
	for(i = 0, pd = _pci_head; i < pci_roots; i++, pd = pd->next) {
		_pci_setup_windows (pd);
	}

}


/*
 *  Scan list of configured devices, probe and attach.
 */
void
_pci_devinit (int initialise)
{
	if(monarch_mode) {
		int i;
		struct pci_device *pd;

		for(i = 0, pd = _pci_head; i < pci_roots; i++, pd = pd->next) {
			_pci_setup_devices (pd, initialise);
		}
	}
}

