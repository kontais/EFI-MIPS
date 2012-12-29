/*
 *  Copyright (C) 2001-2009 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
 *	Contributed by Jason Fleischli <jason.fleischli@hp.com>
 *  Copyright (C) 2006-2009 Intel Corporation
 *	Contributed by Fenghua Yu <fenghua.yu@intel.com>
 *	Contributed by Bibo Mao <bibo.mao@intel.com>
 *	Contributed by Chandramouli Narayanan <mouli@linux.intel.com>
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
 * Please check out the elilo.txt for complete documentation on how
 * to use this program.
 */

#include <efi.h>
#include <efilib.h>

#include "fs/netfs.h"

#include "elilo.h"

#define FS_NAME L"netfs"

#define NETFS_DEFAULT_BUFSIZE		16*MB
#define NETFS_DEFAULT_BUFSIZE_INC	 8*MB
#define NETFS_DEFAULT_BLOCKSIZE		1024	/* setting to zero is supposed to default the underlying */
						/* pxe implementation to largest blocksize supported,... */
						/* in reality on original older efi implementations its */
						/* never set causing the pxe transfer to timeout. */
						/* the spec defines the minimum supported blocksize default */
						/* to be 512 bytes... a bit extreme, 1024 should work for */
						/* everything  */

#define NETFS_DEFAULT_SERVER_TYPE	EFI_PXE_BASE_CODE_BOOT_TYPE_BOOTSTRAP
#define NETFS_FD_MAX	2

typedef struct _netfs_fd {
	struct _netfs_fd *next;
	
	CHAR8  *netbuf;
	UINT64 netbuf_maxsize;	/* currently allocated buffer */
	UINTN  netbuf_size;	/* number of bytes currently used in the buffer */
	UINT64 netbuf_pos;	/* current position in the buffer */
	BOOLEAN is_valid;	/* avoid conflicting opens */
	BOOLEAN netbuf_reuse;

	CHAR16 last_file[FILENAME_MAXLEN];
} netfs_fd_t;


typedef struct {
	EFI_PXE_BASE_CODE *pxe;
	EFI_HANDLE	  dev;		/* handle to device we're attached to */
	BOOLEAN 	  using_pxe;	/* true if downloaded using the PXE protocol vs. regular DHCP */

	EFI_IP_ADDRESS	srv_ip;
	EFI_IP_ADDRESS	cln_ip;
	EFI_IP_ADDRESS	gw_ip;
	EFI_IP_ADDRESS	netmask;
	UINT8		hw_addr[16];

	netfs_fd_t	fd_tab[NETFS_FD_MAX];
	netfs_fd_t	*free_fd;
	UINTN		free_fd_count;

} netfs_priv_state_t;

#define NETFS_F2FD(l,f)		(UINTN)((f)-(l)->fd_tab)
#define NETFS_FD2F(l,fd)	((l)->fd_tab+fd)
#define NETFS_F_INVALID(f)	((f)->is_valid == FALSE)


typedef union {
	netfs_interface_t pub_intf;
	struct {
		netfs_interface_t  pub_intf;
		netfs_priv_state_t priv_data;
	} netfs_priv;
} netfs_t;

#define FS_PRIVATE(n)	(&(((netfs_t *)n)->netfs_priv.priv_data))

typedef union {
	EFI_HANDLE *dev;
	netfs_t  *intf;
} dev_tab_t;

static dev_tab_t *dev_tab;	/* holds all devices we found */
static UINTN ndev;		/* how many entries in dev_tab */

static EFI_GUID NetFsProtocol = NETFS_PROTOCOL;


#if 0
static EFI_PXE_BASE_CODE_CALLBACK_STATUS
netfs_callback_func(
    IN EFI_PXE_BASE_CODE_CALLBACK *this,
    IN EFI_PXE_BASE_CODE_FUNCTION function,
    IN BOOLEAN received,
    IN UINT32 packet_len,
    IN EFI_PXE_BASE_CODE_PACKET *packet OPTIONAL
)
{
	Print(L"netfs_callback called received=%d packet_len=%d\n", received, packet_len);
	return EFI_ABORTED;
}

static EFI_PXE_BASE_CODE_CALLBACK netfs_callback = {
    EFI_PXE_BASE_CODE_CALLBACK_INTERFACE_REVISION,
    &netfs_callback_func
};
#endif

static netfs_fd_t *
netfs_fd_alloc(netfs_priv_state_t *nfs, CHAR16 *name)
{
	netfs_fd_t *tmp = NULL, *prev = NULL, *match;
	UINT8 netbuf_reuse = 0;

	if (nfs->free_fd == NULL) {
		ERR_PRT((L"out of file descriptor"));
		return NULL;
	}
	match = nfs->free_fd; 
	for (tmp = nfs->free_fd; tmp; tmp = tmp->next) {
		if (!StrCmp(name, tmp->last_file)) {
			DBG_PRT((L"Using cached file %s netbuf_size=%d", tmp->last_file, tmp->netbuf_size));
			netbuf_reuse = 1;
			match = tmp;
			break;
		}
		prev = tmp;
	}
	/* indicate whether or not we got a match in caching */
	match->netbuf_reuse = netbuf_reuse;

	if (match == nfs->free_fd) 
		nfs->free_fd = match->next;	
	else 
		prev->next  = match->next;

	nfs->free_fd_count--;

	return match;
}

static VOID
netfs_fd_free(netfs_priv_state_t *nfs, netfs_fd_t *f)
{
	if (f == NULL) {
		ERR_PRT((L"invalid fd"));
		return;
	}
	f->next    = nfs->free_fd;

	/* we keep the netbuf, in case we can reuse it */
	f->is_valid = FALSE;

	nfs->free_fd = f;
	nfs->free_fd_count++;

	if (nfs->free_fd_count > NETFS_FD_MAX) {
		ERR_PRT((L"too many free descriptors %d", nfs->free_fd_count));
	}
}


static INTN
netbuf_alloc(netfs_fd_t *f)
{
	/* we will try to reuse the existing buffer first */
	if (f->netbuf != 0) return 0;

	f->netbuf_pos     = 0;

	f->netbuf = (CHAR8 *)alloc_pages(EFI_SIZE_TO_PAGES(f->netbuf_maxsize), EfiLoaderData, AllocateAnyPages, 0);
		
	return f->netbuf == 0 ? -1 : 0;
}

static EFI_STATUS
netfs_name(netfs_interface_t *this, CHAR16 *name, UINTN maxlen)
{
	if (name == NULL || maxlen < 1) return EFI_INVALID_PARAMETER;

	StrnCpy(name, FS_NAME, maxlen-1);

	name[maxlen-1] = CHAR_NULL;

	return EFI_SUCCESS;
}

static  VOID
netfs_extract_ip(netfs_priv_state_t *nfs)
{
	EFI_PXE_BASE_CODE *pxe = nfs->pxe;

	if (pxe->Mode->PxeDiscoverValid) {
		nfs->using_pxe = TRUE;
		Memcpy(&nfs->srv_ip, pxe->Mode->PxeReply.Dhcpv4.BootpSiAddr, sizeof(EFI_IP_ADDRESS)); 
		Memcpy(&nfs->hw_addr, pxe->Mode->PxeReply.Dhcpv4.BootpHwAddr, 16*sizeof(UINT8));
	} else {
		Memcpy(&nfs->srv_ip, pxe->Mode->DhcpAck.Dhcpv4.BootpSiAddr, sizeof(EFI_IP_ADDRESS)); 
		Memcpy(&nfs->hw_addr, pxe->Mode->DhcpAck.Dhcpv4.BootpHwAddr, sizeof(nfs->hw_addr));
	}

	Memcpy(&nfs->cln_ip, &pxe->Mode->StationIp, sizeof(EFI_IP_ADDRESS)); 
	Memcpy(&nfs->netmask, &pxe->Mode->SubnetMask, sizeof(EFI_IP_ADDRESS)); 

	/*
	 * the fact that we use index 0, is just a guess
	 */
	if (pxe->Mode->RouteTableEntries>0) 
		Memcpy(&nfs->gw_ip, &pxe->Mode->RouteTable[0].GwAddr, sizeof(EFI_IP_ADDRESS)); 

	VERB_PRT(1, Print(L"PXE PxeDiscoverValid: %s\n", pxe->Mode->PxeDiscoverValid?  L"Yes (PXE-aware DHCPD)\n" : L"No (Regular DHCPD)\n"));
#if 0
	status = BS->HandleProtocol(dev, &PxeCallbackProtocol, (VOID **)&netfs_callback);
	status = LibInstallProtocolInterfaces(&dev, &PxeCallbackProtocol, &netfs_callback, NULL);
	Print(L"PXE Callback support : %r\n", status);
	if (status == EFI_SUCCESS) {
		BOOLEAN doit = TRUE;
		status = pxe->SetParameters(pxe, NULL, NULL, NULL, NULL, &doit);
		Print(L"PXE Callback SetParameters: %r\n", status);
	}
#endif
		/*
		 * XXX: TFTPD server not quite right when using PXE, need to extract bootservers...
		 */
	VERB_PRT(1, Print(L"Local IP: %d.%d.%d.%d\n",
		pxe->Mode->StationIp.v4.Addr[0] & 0xff,
		pxe->Mode->StationIp.v4.Addr[1] & 0xff,
		pxe->Mode->StationIp.v4.Addr[2] & 0xff,
		pxe->Mode->StationIp.v4.Addr[3] & 0xff));

	VERB_PRT(1, Print(L"SM: %d.%d.%d.%d\n",
		pxe->Mode->SubnetMask.v4.Addr[0] & 0xff,
		pxe->Mode->SubnetMask.v4.Addr[1] & 0xff,
		pxe->Mode->SubnetMask.v4.Addr[2] & 0xff,
		pxe->Mode->SubnetMask.v4.Addr[3] & 0xff));

	VERB_PRT(1, Print(L"TFTPD IP: %d.%d.%d.%d\n", 
		nfs->srv_ip.v4.Addr[0] & 0xff, 
		nfs->srv_ip.v4.Addr[1] & 0xff, 
		nfs->srv_ip.v4.Addr[2] & 0xff, 
		nfs->srv_ip.v4.Addr[3] & 0xff));

	VERB_PRT(1, Print(L"Gateway IP: %d.%d.%d.%d\n",
			nfs->gw_ip.v4.Addr[0] & 0xff, 
			nfs->gw_ip.v4.Addr[1] & 0xff, 
			nfs->gw_ip.v4.Addr[2] & 0xff, 
			nfs->gw_ip.v4.Addr[3] & 0xff));

}

static EFI_STATUS
netfs_start(EFI_PXE_BASE_CODE *pxe)
{
	EFI_STATUS status;

	status = pxe->Start (pxe, FALSE);
	if (EFI_ERROR(status)) return status;

	return pxe->Dhcp(pxe, FALSE);
}

static EFI_STATUS
netfs_open(netfs_interface_t *this, CHAR16 *name, UINTN *fd)
{
	netfs_priv_state_t *nfs;
	netfs_fd_t	   *f;
	EFI_STATUS	   status;
	CHAR8		   ascii_name[FILENAME_MAXLEN];
	UINTN 		   blocksize = NETFS_DEFAULT_BLOCKSIZE;
	UINTN		   prev_netbufsize, retries = 0;
	BOOLEAN		   server_provided_filesize = FALSE;

	if (this == NULL || name == NULL || fd == NULL) return EFI_INVALID_PARAMETER;

	nfs = FS_PRIVATE(this);

	if (nfs->pxe == NULL) return EFI_INVALID_PARAMETER;

	/*
	 * Try to start protocol if not already active
	 */
	if (nfs->pxe->Mode->Started == FALSE) {
		status = netfs_start(nfs->pxe);
		if (EFI_ERROR(status)) return  status;
		netfs_extract_ip(nfs);
	}

	if ((f=netfs_fd_alloc(nfs, name)) == NULL) return EFI_OUT_OF_RESOURCES;

	if (f->netbuf_reuse) {
		f->netbuf_pos = 0;
		f->is_valid = TRUE;
		*fd = NETFS_F2FD(nfs, f);
		return EFI_SUCCESS;
	}
	f->netbuf_maxsize = NETFS_DEFAULT_BUFSIZE;
	f->netbuf_size = 0;
	
	if (f->netbuf == NULL && netbuf_alloc(f) == -1) {
		netfs_fd_free(nfs, f);
		return EFI_OUT_OF_RESOURCES;
	}

	/* well, we need to download ! */

	U2ascii(name, ascii_name, FILENAME_MAXLEN);

	VERB_PRT(2, Print(L"downloading %a from %d.%d.%d.%d...\n", ascii_name, 
				nfs->srv_ip.v4.Addr[0], 
				nfs->srv_ip.v4.Addr[1], 
				nfs->srv_ip.v4.Addr[2], 
				nfs->srv_ip.v4.Addr[3]));
retry:
	if (retries == 2) {
		netfs_fd_free(nfs, f);
		VERB_PRT(2, Print(L"Failed: %r\n", status));
		return status;
	}

/*
 * netboot bugfix SF tracker 2874380
 * EFI 1.10 spec
 * For read operations, the return data will be placed in the buffer specified by BufferPtr. If
 * BufferSize is too small to contain the entire downloaded file, then
 * EFI_BUFFER_TOO_SMALL will be returned and BufferSize will be set to zero or the size of
 * the requested file (the size of the requested file is only returned if the TFTP server supports TFTP
 * options). If BufferSize is large enough for the read operation, then BufferSize will be set to
 * the size of the downloaded file, and EFI_SUCCESS will be returned. Applications using the
 * PxeBc.Mtftp() services should use the get-file-size operations to determine the size of the
 * downloaded file prior to using the read-file operations—especially when downloading large
 * (greater than 64 MB) files—instead of making two calls to the read-file operation. Following this
 * recommendation will save time if the file is larger than expected and the TFTP server does not
 * support TFTP option extensions. Without TFTP option extension support, the client has to
 * download the entire file, counting and discarding the received packets, to determine the file size.
 * ...
 * For TFTP “get file size” operations, the size of the requested file or directory is returned in
 * BufferSize, and EFI_SUCCESS will be returned. If the TFTP server does not support options,
 * the file will be downloaded into a bit bucket and the length of the downloaded file will be returned.
 */
	status = nfs->pxe->Mtftp ( 
				   nfs->pxe, 
				   EFI_PXE_BASE_CODE_TFTP_GET_FILE_SIZE, 
			  	   f->netbuf, 
			   	   FALSE,
			   	   &(f->netbuf_size), 	// PXE writes size of file from server here
			   	   &blocksize, 
			   	   &nfs->srv_ip, 
			   	   ascii_name, 
			   	   NULL, 
			   	   FALSE);
	/*
	 * If options are not supported by this tftp server, according to the spec the file will be
 	 * downloaded into a bit bucket, the size calculated by efi fw and returned in the status
	 * field of this call. YUK!!... in this case we will default to currently allocated max
	 * if thats still not big enough it will be caught and increased following the read file attempt
	 * then retried. 
	 * XXX need to research how this is handled or changed in the latest UEFI spec.
	 */
	if (status != EFI_SUCCESS) {
		f->netbuf_size = f->netbuf_maxsize;
		VERB_PRT(2, Print(L"setting default buffer size of %d for %a, no filesize recd from tftp server\n",
				  f->netbuf_size, ascii_name));
	}

	if (status == EFI_SUCCESS) {
		server_provided_filesize = TRUE;
		VERB_PRT(2, Print(L"received file size of %d for %a from tftp server.\n",
				  f->netbuf_size, ascii_name));
	}

	if (f->netbuf_size > f->netbuf_maxsize) {	// we need a bigger buffer
		VERB_PRT(2, Print(L"allocated buffer too small, attempting to increase\n"));
		f->netbuf_maxsize += f->netbuf_size;
		free(f->netbuf);
		f->netbuf = NULL;
		if (netbuf_alloc(f) == -1) return EFI_OUT_OF_RESOURCES;
	}

	/* paranoid catch any corner case missed */
	if (f->netbuf_size == 0) f->netbuf_size = f->netbuf_maxsize;

	DBG_PRT((L"\nbefore read: netbuf:" PTR_FMT " netbuf_size=%d blocksize=%d\n", 
		f->netbuf, 
		f->netbuf_size,
		blocksize));

	prev_netbufsize = f->netbuf_size;

	/* now try and download this file from the tftp server */
	status = nfs->pxe->Mtftp (
				   nfs->pxe, 
				   EFI_PXE_BASE_CODE_TFTP_READ_FILE, 
				   f->netbuf, 
				   FALSE,
			   	   &(f->netbuf_size), 
			   	   &blocksize, 
			   	   &nfs->srv_ip, 
			   	   ascii_name, 
			   	   NULL, 
			   	   FALSE);

	DBG_PRT((L"after: status=%r netbuf:" PTR_FMT " netbuf_size=%d blocksize=%d\n", 
		status, 
		f->netbuf, 
		f->netbuf_size, 
		blocksize));

	if ((status == EFI_TIMEOUT || status == EFI_BUFFER_TOO_SMALL) && !server_provided_filesize) {
		Print(L"buffer too small, need netbuf_size=%d\n", f->netbuf_size);
		/*
		 * if the TFTP server supports TFTP options, then we should
		 * get the required size. So we test to see if the size
		 * we set has changed. If so, we got the required size.
		 * If not, we increase the buffer size and retry.
		 */
		if (f->netbuf_size == prev_netbufsize) {
			f->netbuf_maxsize += NETFS_DEFAULT_BUFSIZE_INC;
		} else {
			/* we got an answer from the TFTP server, let's try it */
			f->netbuf_maxsize = f->netbuf_size;
			server_provided_filesize = TRUE;
		}
		free(f->netbuf);

		f->netbuf = NULL; /* will force reallocation */

		if (netbuf_alloc(f) == 0) {
			retries++;
			goto retry;
		}

	} else if (status == EFI_TIMEOUT) {	//if just a simple timeout, buffers are good just retry
		VERB_PRT(2, Print(L"TFTP returned EFI_TIMEOUT ERROR... %d retries left.\n", (2 - retries)));
		retries++;
		goto retry;
	}
	if (status == EFI_SUCCESS) {
		/* start at the beginning of the file */
		f->netbuf_pos = 0;

		/* cache file name */
		StrCpy(f->last_file, name);

		f->is_valid = 1;

		*fd = NETFS_F2FD(nfs, f);
		VERB_PRT(2, Print(L"Done\n"));
	} else {
		netfs_fd_free(nfs, f);
		VERB_PRT(2, Print(L"Failed: %r\n", status));
	}
	DBG_PRT((L"File %s netbuf_size=%d: %r", name, f->netbuf_size, status));
#if 0
	Print(L"\n---\n");
	{ INTN i; 
		for(i=0; i < netbuf_size; i++) {
			Print(L"%c", (CHAR16)netbuf[i]);
		}
	}
	Print(L"\n---\n");
#endif
	return status;
}


static EFI_STATUS
netfs_read(netfs_interface_t *this, UINTN fd, VOID *buf, UINTN *size)
{
	netfs_priv_state_t *nfs;
	netfs_fd_t	   *f;
	UINTN		   count;

	if (this == NULL || fd >= NETFS_FD_MAX || buf == NULL || size == NULL) return EFI_INVALID_PARAMETER;

	nfs = FS_PRIVATE(this);
	f   = NETFS_FD2F(nfs, fd);

	if (NETFS_F_INVALID(f)) return EFI_INVALID_PARAMETER;

	count = MIN(*size, f->netbuf_size - f->netbuf_pos);

	if (count) Memcpy(buf, f->netbuf+f->netbuf_pos, count);

	*size = count;
	f->netbuf_pos += count;

	return EFI_SUCCESS;
}

static EFI_STATUS
netfs_close(netfs_interface_t *this, UINTN fd)
{
	netfs_priv_state_t *nfs;
	netfs_fd_t	     *f;

	if (this == NULL || fd >= NETFS_FD_MAX) return EFI_INVALID_PARAMETER;

	nfs = FS_PRIVATE(this);
	f   = NETFS_FD2F(nfs, fd);

	if (NETFS_F_INVALID(f)) return EFI_INVALID_PARAMETER;

	netfs_fd_free(nfs, f);

	return EFI_SUCCESS;
}

static EFI_STATUS
netfs_seek(netfs_interface_t *this, UINTN fd, UINT64 newpos)
{
	netfs_priv_state_t *nfs;
	netfs_fd_t	   *f;

	if (this == NULL || fd >= NETFS_FD_MAX) return EFI_INVALID_PARAMETER;

	nfs = FS_PRIVATE(this);
	f   = NETFS_FD2F(nfs, fd);

	if (NETFS_F_INVALID(f)) return EFI_INVALID_PARAMETER;

	if (newpos > f->netbuf_size) return EFI_INVALID_PARAMETER;

	f->netbuf_pos = newpos;

	return EFI_SUCCESS;
}

static EFI_STATUS
netfs_infosize(netfs_interface_t *this, UINTN fd, UINT64 *sz)
{
	netfs_priv_state_t *nfs;
	netfs_fd_t	   *f;

	if (this == NULL || fd >= NETFS_FD_MAX || sz == NULL) return EFI_INVALID_PARAMETER;

	nfs = FS_PRIVATE(this);
	f   = NETFS_FD2F(nfs, fd);

	if (NETFS_F_INVALID(f)) return EFI_INVALID_PARAMETER;

	*sz = f->netbuf_size;

	return EFI_SUCCESS;
}

static INTN
find_dhcp_option(EFI_PXE_BASE_CODE_PACKET *packet, UINT8 use_ipv6, UINT8 option, CHAR8 *str, INTN *len)
{
	INTN i = 0;
	UINT8 tag, length;
	UINT8 *opts = packet->Dhcpv4.DhcpOptions;

	*len = 0;

	for(;;) {
		if (i >= 56) {
			DBG_PRT((L"reach end of options (no marker)\n"));
			break;
		}
		tag = opts[i++];

		if (tag == 0) continue;
		if (tag == 255) break;

		length = opts[i++];

#if 0
		{ UINT8 l = length, k = 0;
			Print(L"found option %d len=%d: ", tag, length);
			while (l--) { Print(L"%c(%d)\n", (CHAR16)opts[k], opts[k]); k++; }
			Print(L"\n");
		}
#endif
		if (tag == option) {
			*len = length;
			while (length--) { *str++ = opts[i++]; }
			return 0;
		}
		i += length;
	}
	return -1;
}

static EFI_STATUS
netfs_getinfo(netfs_interface_t *this, netfs_info_t *info)
{
	netfs_priv_state_t *nfs;
	CHAR8 str[256];
	INTN len, r;

	if (this == NULL || info == NULL) return EFI_INVALID_PARAMETER;

	nfs = FS_PRIVATE(this);

	Memcpy(&info->cln_ipaddr, &nfs->cln_ip, sizeof(EFI_IP_ADDRESS));
	Memcpy(&info->srv_ipaddr, &nfs->srv_ip, sizeof(EFI_IP_ADDRESS));
	Memcpy(&info->netmask, &nfs->netmask, sizeof(EFI_IP_ADDRESS));
	Memcpy(&info->gw_ipaddr, &nfs->gw_ip, sizeof(EFI_IP_ADDRESS));
	Memcpy(&info->hw_addr, &nfs->hw_addr, sizeof(info->hw_addr));

	info->using_pxe  = nfs->using_pxe;
	info->started    = nfs->pxe->Mode->Started;
	info->using_ipv6 = nfs->pxe->Mode->UsingIpv6;

	if (nfs->pxe->Mode->UsingIpv6) goto skip_options;

	r = find_dhcp_option(&nfs->pxe->Mode->DhcpAck,nfs->pxe->Mode->UsingIpv6, 15, str,  &len);
	str[len] = '\0';
	ascii2U(str, info->domainame, 255);

	VERB_PRT(3, Print(L"domain(15): %a\n", str));

	r = find_dhcp_option(&nfs->pxe->Mode->DhcpAck,nfs->pxe->Mode->UsingIpv6, 12, str,  &len);
	str[len] = '\0';
	ascii2U(str, info->hostname, 255);

	VERB_PRT(3, Print(L"hostname(12): %a\n", str));

	/*
	 * extract bootfile name from DHCP exchanges
	 */
	if (nfs->using_pxe == 0) {
 		ascii2U(nfs->pxe->Mode->DhcpAck.Dhcpv4.BootpBootFile, info->bootfile, NETFS_BOOTFILE_MAXLEN);
		VERB_PRT(3, Print(L"bootfile: %s\n", info->bootfile));
	}

skip_options:
	return EFI_SUCCESS;
}

static UINT16
find_pxe_server_type(EFI_PXE_BASE_CODE *pxe)
{
	INTN i = 0, max;
	UINT8 tag, length;
	UINT8 *opts = pxe->Mode->PxeReply.Dhcpv4.DhcpOptions;
	UINT16 server_type;

	while(i < 55) {
		tag    = opts[i];
		length = opts[i+1];

		DBG_PRT((L"Tag #%d Length %d\n",tag, length));

		if (tag == 43) goto found;

		i += 2 + length;
	}
	return NETFS_DEFAULT_SERVER_TYPE;
found:
	max = i+2+length;
	i  += 2;
	while (i < max) {
		tag    = opts[i];
		length = opts[i+1];
		if (tag == 71) {
			server_type =(opts[i+2]<<8) | opts[i+3];
			DBG_PRT((L"ServerType: %d\n", server_type));
			return server_type;
		}
		i+= 2 + length;
	}
	return NETFS_DEFAULT_SERVER_TYPE;
}

static EFI_STATUS
netfs_query_layer(netfs_interface_t *this, UINT16 server_type, UINT16 layer, UINTN maxlen, CHAR16 *str)
{
	netfs_priv_state_t *nfs;
	EFI_STATUS	   status;

	if (this == NULL || str == NULL) return EFI_INVALID_PARAMETER;

	nfs = FS_PRIVATE(this);
	
	if (nfs->using_pxe == FALSE) return EFI_UNSUPPORTED;

	if (server_type == 0) server_type = find_pxe_server_type(nfs->pxe);

	status = nfs->pxe->Discover (nfs->pxe, server_type, &layer, FALSE, 0);
	if(status == EFI_SUCCESS) {
		ascii2U(nfs->pxe->Mode->PxeReply.Dhcpv4.BootpBootFile, str, maxlen);
	} 
	return status;
}

static VOID
netfs_init_state(netfs_t *netfs, EFI_HANDLE dev, EFI_PXE_BASE_CODE *pxe)
{
	netfs_priv_state_t *nfs = FS_PRIVATE(netfs);
	UINTN i;

	/* need to do some init here on netfs_intf */
	Memset(netfs, 0, sizeof(*netfs));


	netfs->pub_intf.netfs_name        = netfs_name;
	netfs->pub_intf.netfs_open        = netfs_open;
	netfs->pub_intf.netfs_read        = netfs_read;
	netfs->pub_intf.netfs_close       = netfs_close;
	netfs->pub_intf.netfs_infosize    = netfs_infosize;
	netfs->pub_intf.netfs_seek        = netfs_seek;
	netfs->pub_intf.netfs_query_layer = netfs_query_layer;
	netfs->pub_intf.netfs_getinfo     = netfs_getinfo;

	nfs->dev = dev;
	nfs->pxe = pxe;

	/*
	 * we defer DHCP request until it is really necessary (netfs_open)
	 */
	if (pxe->Mode->Started == TRUE) netfs_extract_ip(nfs);

	Memset(nfs->fd_tab, 0, sizeof(nfs->fd_tab));

	for (i=0; i < NETFS_FD_MAX-1; i++) {
		nfs->fd_tab[i].next = &nfs->fd_tab[i+1];
	}
	/* null on last element is done by memset */

	nfs->free_fd	    = nfs->fd_tab;
	nfs->free_fd_count  = NETFS_FD_MAX;
}

static EFI_STATUS
netfs_install_one(EFI_HANDLE dev, VOID **intf)
{

	EFI_STATUS status;
	netfs_t *netfs;
	EFI_PXE_BASE_CODE *pxe;

	status = BS->HandleProtocol (dev, &NetFsProtocol, (VOID **)&netfs);
	if (status == EFI_SUCCESS) {
		ERR_PRT((L"Warning: found existing %s protocol on device", FS_NAME));
		goto found;
	}
	
	status = BS->HandleProtocol (dev, &PxeBaseCodeProtocol, (VOID **)&pxe);
	if (EFI_ERROR(status)) return EFI_INVALID_PARAMETER;


	netfs = (netfs_t *)alloc(sizeof(*netfs), EfiLoaderData);
	if (netfs == NULL) {
		ERR_PRT((L"failed to allocate %s", FS_NAME));
		return EFI_OUT_OF_RESOURCES;
	}

	netfs_init_state(netfs, dev, pxe);

	status = LibInstallProtocolInterfaces(&dev, &NetFsProtocol, netfs, NULL);
	if (EFI_ERROR(status)) {
		ERR_PRT((L"Cannot install %s protocol: %r", FS_NAME, status));
		free(netfs);
		return status;
	}

found:
	if (intf) *intf = (VOID *)netfs;

	VERB_PRT(3,
		{ EFI_DEVICE_PATH *dp; CHAR16 *str;
		  dp  = DevicePathFromHandle(dev);
		  str = DevicePathToStr(dp);
		  Print(L"attached %s to %s\n", FS_NAME, str);
		  FreePool(str);
		});

	return EFI_SUCCESS;
}
	
EFI_STATUS
netfs_install(VOID)
{
	UINTN size = 0;
	UINTN i;
	EFI_STATUS status;
	VOID *intf;

	BS->LocateHandle (ByProtocol, &PxeBaseCodeProtocol, NULL, &size, NULL);
	if (size == 0) return EFI_UNSUPPORTED; /* no device found, oh well */

	DBG_PRT((L"size=%d", size));

	dev_tab = (dev_tab_t *)alloc(size, EfiLoaderData);
	if (dev_tab == NULL) {
		ERR_PRT((L"failed to allocate handle table"));
		return EFI_OUT_OF_RESOURCES;
	}
	
	status = BS->LocateHandle (ByProtocol, &PxeBaseCodeProtocol, NULL, &size, (VOID **)dev_tab);
	if (status != EFI_SUCCESS) {
		ERR_PRT((L"failed to get handles: %r", status));
		free(dev_tab);
		return status;
	}
	ndev = size / sizeof(EFI_HANDLE);

	for(i=0; i < ndev; i++) {
		intf = NULL;
		netfs_install_one(dev_tab[i].dev, &intf);
		/* override device handle with interface pointer */
		dev_tab[i].intf = intf;
	}

	return EFI_SUCCESS;
}
	
EFI_STATUS
netfs_uninstall(VOID)
{
	
	netfs_priv_state_t *nfs;
	EFI_STATUS status;
	UINTN i;

	for(i=0; i < ndev; i++) {
		if (dev_tab[i].intf == NULL) continue;
		nfs = FS_PRIVATE(dev_tab[i].intf);
		status = BS->UninstallProtocolInterface (nfs->dev, &NetFsProtocol, dev_tab[i].intf);
		if (EFI_ERROR(status)) {
			ERR_PRT((L"Uninstall %s error: %r", FS_NAME, status));
			continue;
		}
		VERB_PRT(3,
			{ EFI_DEVICE_PATH *dp; CHAR16 *str;
		  	dp  = DevicePathFromHandle(nfs->dev);
		  	str = DevicePathToStr(dp);
		  	Print(L"uninstalled %s on %s\n", FS_NAME, str);
		  	FreePool(str);
			});

		if (nfs->pxe->Mode->Started == TRUE) 
			nfs->pxe->Stop (nfs->pxe);

		free(dev_tab[i].intf);
	}
	if (dev_tab) free(dev_tab);

	return EFI_SUCCESS;
}
