/*
 * Copyright (c) 1999, 2004
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software must
 *    display the following acknowledgement:
 * 
 *    This product includes software developed by Intel Corporation and its
 *    contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include <atk_libc.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <string.h>
#include "efi_interface.h"
#include "efiapi.h"

#include "efi.h"
#include "efilib.h"

/*
 *  Forward reference
 */

static void  usage();

#define HANDLE_NUM 64
#define MAX_LINESIZE 512
//static  EFI_GUID LoadedImageProtocol  = LOADED_IMAGE_PROTOCOL;

int
main(   int Argc, wchar_t **Argv )
{
	
	EFI_HANDLE              *handlebuf = NULL;
	UINTN                   bufsize = 0;
	UINTN                   NoHandles;
	INTN                    index=0;
	int                     unloaded = 0;
	EFI_STATUS              Status;
	EFI_BOOT_SERVICES       *BS = NULL;
	EFI_LOADED_IMAGE        *ImageInfo = NULL;
	 

	/*
	 *  Find all files listed on the command line
	 */
	
	if ( Argc != 2 ){
	    usage();
	}
	Argc-- ;
	Argv++;
	
	BS = _GetBootServices();
	
	if( BS == NULL ) {
	   printf( "nunload: no boot service available \n" );
	   exit(1);
	}                 
	       
  bufsize = 10 * sizeof(EFI_HANDLE);
  handlebuf =( EFI_HANDLE *) malloc(bufsize);
        
  if( handlebuf == NULL ){
	   printf(" nunload: no memory to allocate \n");
	   exit(1);
	}                
        
 again:
  Status = BS->LocateHandle (
                  ByProtocol,
                  &LoadedImageProtocol,
                  NULL,
                  &bufsize,
                  handlebuf
                  );
        
  if (EFI_ERROR(Status)){
    if ( Status == EFI_BUFFER_TOO_SMALL ){
     	free(handlebuf); 
      handlebuf =( EFI_HANDLE *) malloc(bufsize);
      if( handlebuf == NULL ){
         printf(" nunload: no memory to allocate \n");
         exit(1);
   	  }  
      goto again;	
    } 
    else {
  	  printf(" nunload: locate handle error %s \n",Status);
      exit(1);
    }
  }
  NoHandles = bufsize / sizeof (EFI_HANDLE);
   
  for( index = NoHandles -2 ; index >=0 ; index-- ) {
	  Status = BS->HandleProtocol (handlebuf[index], 
                                 &LoadedImageProtocol, 
                                 (VOID**)&ImageInfo);  
    if (EFI_ERROR(Status)) {
	    printf(" nunload: get image info failed %p \n",Status);
	    exit(1);
	  }
	  if( ImageInfo->LoadOptions!=NULL && wcsstr(ImageInfo->LoadOptions,Argv[0])!= NULL ) {
	    Status = BS->UnloadImage( handlebuf[index]);
	    if (EFI_ERROR(Status)) {
	      printf(" nunload: unload image failed %p \n",Status);
	      exit(1);
	    }
	    unloaded++;
	  }
	}
	
  if( handlebuf != NULL )
    free(handlebuf);	
   
  printf( "nunload: %x images unload successfully\n",unloaded );
  exit(0); 
}


static void
usage()
{
	fprintf( stderr, "%s\n", "usage: nunload imagename.efi" );
	exit(1);
}
