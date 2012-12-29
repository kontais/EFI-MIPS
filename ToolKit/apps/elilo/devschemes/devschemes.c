/*
 *  Copyright (C) 2001-2003 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
 *
 * This file is part of the ELILO, the EFI Linux boot loader.
 *
 *  ELILO is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  ELILO is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with ELILO; see the file COPYING.  If not, write to the Free
 *  Software Foundation, 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * Look at the README.devschemes for more explanations on how
 * to use devschemes.
 */

#include <efi.h>
#include <efilib.h>

#include "elilo.h"
#include "fileops.h"

#define NAMING_SCHEME	L"simple"

typedef struct {
	INT8 type;
	INT8 subtype;
	INTN (*device_func)(device_t *dev, EFI_DEVICE_PATH *dp);
} devices_types_t;


static UINT32 atapi_count, scsi_count, net_count;

static INTN
atapi_device(device_t *dev, EFI_DEVICE_PATH *dp)
{
	//ATAPI_DEVICE_PATH *atapi = (ATAPI_DEVICE_PATH *)dp;

	dev->name[0] = L'a';
	dev->name[1] = L't';
	dev->name[2] = L'a';
	dev->name[3] = L'p';
	dev->name[4] = L'i';

	SPrint(dev->name+5,FILEOPS_DEVNAME_MAXLEN-5-1,  L"%d", atapi_count);
	atapi_count++;

	return 0;
}

static INTN
scsi_device(device_t *dev, EFI_DEVICE_PATH *dp)
{
	//SCSI_DEVICE_PATH *scsi = (SCSI_DEVICE_PATH *)dp;

	dev->name[0]   = L's';
	dev->name[1]   = L'c';
	dev->name[2]   = L's';
	dev->name[3]   = L'i';

	SPrint(dev->name+4, FILEOPS_DEVNAME_MAXLEN-4-1, L"%d", scsi_count);
	scsi_count++;

	return 0;
}

static INTN
network_device(device_t *dev, EFI_DEVICE_PATH *dp)
{
	//MAC_ADDR_DEVICE_PATH *mac = (MAC_ADDR_DEVICE_PATH *)dp;

	dev->name[0] = L'n';
	dev->name[1] = L'e';
	dev->name[2] = L't';

	SPrint(dev->name+3, FILEOPS_DEVNAME_MAXLEN-3-1, L"%d", net_count);
	net_count++;

	return 0;
}

/*
 * what we are looking for in the device path
 */
static devices_types_t dev_types[]={
	{ MESSAGING_DEVICE_PATH, MSG_ATAPI_DP, atapi_device},
	{ MESSAGING_DEVICE_PATH, MSG_SCSI_DP, scsi_device},
	{ MESSAGING_DEVICE_PATH, MSG_MAC_ADDR_DP, network_device},
	{ 0, 0 , NULL}
};

static INTN
simple_scheme(device_t *tab, UINTN n)
{
	EFI_DEVICE_PATH *dp1, *dp;
	devices_types_t *p;
	UINTN i;

	/*
	 * note that this test is necessary but not sufficient to guarantee that this scheme
	 * will work because, we have no way of detecting that the machine got actually
	 * rebooted if the EDD30 variable was forced. this comes from the fact, that elilo
	 * can be invoked once, aborted and then restarted with no machine reboot.
	 *
	 * XXX: there may be a way to detect this with the another variable which would
	 * be in volatile memory only
	 */
	if (elilo_opt.edd30_on == 0) {
		VERB_PRT(4, Print(L"%s device naming scheme only works with EDD3.0 enabled\n", NAMING_SCHEME));
		return -1;
	}

	for(i=0; i < n; i++) {
		dp = DevicePathFromHandle(tab[i].dev);
		if (dp == NULL) {
			ERR_PRT((L"cannot get device path for device %d", i));
			continue;
		}
		dp1 = dp = UnpackDevicePath(dp);

		while (!IsDevicePathEnd(dp)) {
			p = dev_types;
			while (p->type) {
				if (   p->type == DevicePathType(dp) 
				    && p->subtype == DevicePathSubType(dp)) {
					(*p->device_func)(tab+i, dp);
				        goto done;
				}

				p++;
			}
        		dp = NextDevicePathNode(dp);
		}
done:
       		FreePool(dp1); 
	}
	return 0;
}

devname_scheme_t simple_devname_scheme={
	NAMING_SCHEME,
	simple_scheme
};
