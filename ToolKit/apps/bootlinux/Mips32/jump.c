#include "efi.h"
#include "efilib.h"
#include "elf.h"

#define MAX_AC  100

VOID
go (
  int argc,
  char **argv,
  char **envp,
  int pc
  );

void strcpy(char*, char*);


/*
 *  Take the string pointed by 's', break it up in words and
 *  make a pointer list in 'av'. Return number of 'args'.
 */
int 
Argvize(char *av[], char *s)
{
	char **pav = av, c;
	int ac;

	for (ac = 0; ac < MAX_AC; ac++) {
		/* step over cntrls and spaces */
		while (*s && *s <= ' ')
			s++;

		if (!*s)
			break;

		c = *s;
		/* if it's a quote skip forward */
		if (c == '\'' || c == '"') {
			if (pav)
				*pav++ = ++s;
			while (*s && *s != c)
				s++;
			if (*s)
				*s++ = 0;
		} else {		/* find end of word */
			if (pav)
				*pav++ = s;
			while (' ' < *s)
				s++;
		}

		if (*s)
			*s++ = 0;
	}
	return (ac);
}


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
  Elf64_Ehdr *e64;
  UINT32     ep;
  char       **Argv;
  char       **Envp;
  char       *CommandLine;
  UINTN      ArgC;

  e64 = (Elf64_Ehdr *) ImageStart;
  ep  = (UINT32) e64->e_entry;

  BS->AllocatePool(EfiLoaderData, 200, (VOID**)&CommandLine);
  BS->AllocatePool(EfiLoaderData, MAX_AC, (VOID**)&Argv);
  BS->AllocatePool(EfiLoaderData, MAX_AC, (VOID**)&Envp);

  strcpy (CommandLine, "xxx root=/dev/sda2 PMON_VER=LM8089-1.4.9a");
  ArgC = Argvize (Argv, CommandLine);
  Envp[0] = 0;
	
	go (ArgC, Argv, Envp, ep);
  
  return TRUE;
    
}

