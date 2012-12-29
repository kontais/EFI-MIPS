/*
 * Copyright (c) 1999, 2000
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
#include <efilib.h>
#include <shell.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <efi_interface.h>
#include <wchar.h>

/*void dumpit(int *ch,unsigned n)
{
    unsigned i,c;

    Print(L" ");

    for(i=0;i<n;i++)
    {
        c=ch[i]%128;
        if(c>=32 && c<=127)
            Print(L"%c",c);
        else
            Print(L".");
    }
    Print(L"\n");
}

void hexdump(void* t,unsigned len)
{
    unsigned char* x = (unsigned char*)t;
    unsigned i=0;
    unsigned c;
    unsigned ch[16];


    while(i != len)
    {
        c = ch[i%16] = *x++;
        if(!(i%16))
            Print(L"%04x: ",i);

        Print(L"%02x",c);

        if(!(++i%4))
            Print(L" ");

        if(!(i%16))
            dumpit(ch,16);
    }

    if(i%16)
    {
        c=2*(16-i%16);
        c+=4-(i%16)/4;
        for(;c;c--)
            Print(L" ");
        dumpit(ch,i%16);
    }
}*/

#pragma comment(exestr,"version_number=1.00.0.0001 ")

int main(int argc,wchar_t* argv[])
{
	EFI_HANDLE              ImageHandle;
	EFI_STATUS              Status;
	EFI_LOADED_IMAGE*       ImageInfo;
	CHAR16                  *LoadOptions,*ExitData;
	UINTN                   LoadOptionsSize = 0,ExitDataSize;
	CHAR16                  *Cwd;
	int                     i;
	int                     Hpath = 0;
	CHAR16                  *ptr;

	InitializeShellApplication(_GetImageHandle(),_LIBC_EFISystemTable);
	
	argc--;
	argv++;

         
        ptr=argv[0];
        while(*ptr !='\0'){
         if (*ptr == '\\')
            Hpath = 1;
         ptr++;   
        }    
        if( Hpath == 0)
          { 
            LoadOptionsSize = wcslen(argv[0])*sizeof(CHAR16) + 6 ;	
            ptr = AllocateZeroPool(LoadOptionsSize);
            wcscpy(ptr,L".\\");
            wcscat(ptr,argv[0]);
          }  
        
        Status = LoadImage(ptr,&ImageHandle); 

    	if(EFI_ERROR(Status))
	{
		Print(L"load: LoadImage error %s - %r\n",argv[0],Status);
		goto Done;
	}

       if ( Hpath == 0 )
           FreePool(ptr);
   	
	// Verify the image is a driver ?
	_LIBC_EFISystemTable->BootServices->HandleProtocol(ImageHandle,&LoadedImageProtocol,(VOID*)&ImageInfo);
	
	if(ImageInfo->ImageCodeType != EfiBootServicesCode &&
		ImageInfo->ImageCodeType != EfiRuntimeServicesCode)
	{
		Print(L"load: image %s is not a driver\n",argv[0]);
		UnloadImage(ImageHandle);
		Status = EFI_LOAD_ERROR;
		goto Done;
	}

	// Construct a load options buffer containing the command line and
	// current working directory.
	//
	// NOTE: To prevent memory leaks, the protocol is responsible for
	// freeing the memory associated with the load options.
	//
	Cwd = ShellCurDir(NULL);

	for(i = 0; i < argc; i++)
		LoadOptionsSize += (StrLen(argv[i]) + 3) * sizeof(CHAR16);  /* 3 extra for "" and space/null */
	
	LoadOptionsSize += (StrLen(Cwd) + 1) * sizeof(CHAR16);
	LoadOptions = AllocatePool(LoadOptionsSize);
	
	StrCpy(LoadOptions,L"\"");
	StrCat(LoadOptions,argv[0]);
	StrCat(LoadOptions,L"\"");

	for(i = 1; i < argc; i++)
	{
		StrCat(LoadOptions,L" \"");
		StrCat(LoadOptions,argv[i]);
		StrCat(LoadOptions,L"\"");
	}

	StrCpy(&LoadOptions[StrLen(LoadOptions) + 1],Cwd);
	FreePool(Cwd);


        Print(L"LoadOptions %s \n",LoadOptions);
	  
	ImageInfo->LoadOptionsSize = (UINT32)LoadOptionsSize;
	ImageInfo->LoadOptions = LoadOptions;



	//
	// Start the image
	//
	Status = StartImage(ImageHandle,(UINT32)LoadOptionsSize,LoadOptions,_LIBC_EFISystemTable,&ExitDataSize,&ExitData);
	if(!EFI_ERROR(Status))
		wprintf(L"load: image %s loaded. returned %x\n",argv[0],Status);
	else
		wprintf(L"load: image %s returned %x\n",argv[0],Status);

Done:
	return Status;
}
