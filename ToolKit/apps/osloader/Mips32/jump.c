#include "efi.h"
#include "efilib.h"
#include "pe.h"

typedef 
BOOLEAN
(* KERNEL_IMAGE_ENTRY_POINT) (
    IN UINTN                 NoEntries,
    IN EFI_MEMORY_DESCRIPTOR *MemoryMap,
    IN UINTN                 MapKey
	);

BOOLEAN
JumpToImage (
    IN UINT8                 *ImageStart,
    IN UINTN                 NoEntries,
    IN EFI_MEMORY_DESCRIPTOR *MemoryMap,
    IN UINTN                 MapKey,
    IN UINTN                 DescriptorSize,
    IN UINT32                DescriptorVersion
    )

//
//
// This code jumps to the Entry point of a PE32+ image. It assumes the image
//  is valid and has been relocated to it's loaded address.
//
// The Entry point in the image is realy a pointer to a plabel. The plabel 
//  contains the entry address of the rountine and it's gp. Any call via a 
//  pointer to a function in C for IA-64 is really an indirect procedure call
//  via a plabel. The C compiler will load the gp for you. I don't know how 
//  to call to an address out side your linked image in C with out going 
//  through the plabel.
//
// It is possible to use linker flags to make a PE32+ Binary == PE32+ image.
//  the key is aligning the image on a 32 byte boundry.
//
{
    IMAGE_DOS_HEADER			*DosHdr;
    IMAGE_NT_HEADERS			*PeHdr;
    KERNEL_IMAGE_ENTRY_POINT	EntryPoint;

    //
    // Assume CheckPEHeader() has already passed on this Image 
    //

    DosHdr = (IMAGE_DOS_HEADER *)ImageStart;
    PeHdr = (IMAGE_NT_HEADERS *)(ImageStart + DosHdr->e_lfanew);

    EntryPoint = (KERNEL_IMAGE_ENTRY_POINT) 
                 (PeHdr->OptionalHeader.ImageBase + 
                  PeHdr->OptionalHeader.AddressOfEntryPoint);

    return EntryPoint(NoEntries, MemoryMap, MapKey);
}

