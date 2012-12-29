#ifndef _SYS_TYPES_H_
#define	_SYS_TYPES_H_

#define	UINT_MAX	0xffffffff
#define LINESZ      200

#define	__P(protos)	protos		/* full-blown ANSI C */

#ifndef	NULL
#define	NULL	0
#endif




typedef unsigned char  UINT8;
typedef unsigned int   UINTN;
typedef int            INTN;
typedef unsigned int   UINT32;
typedef int            INT32;
typedef unsigned short UINT16;

typedef	UINTN	size_t;

typedef	UINTN	vm_offset_t;
typedef	UINTN	vm_size_t;
typedef UINTN	vaddr_t;
typedef UINTN	paddr_t;


#define _EFI_INT_SIZE_OF(n) ((sizeof (n) + sizeof (UINTN) - 1) &~(sizeof (UINTN) - 1))

typedef UINT8 *VA_LIST;
#define VA_START(ap, v) (ap = (VA_LIST) & (v) + _EFI_INT_SIZE_OF (v))
#define VA_ARG(ap, t)   (*(t *) ((ap += _EFI_INT_SIZE_OF (t)) - _EFI_INT_SIZE_OF (t)))
#define VA_END(ap)      (ap = (VA_LIST) 0)

#define	_U	0x01
#define	_L	0x02
#define	_N	0x04
#define	_S	0x08
#define	_P	0x10
#define	_C	0x20
#define	_X	0x40
#define	_B	0x80

#endif /* !_SYS_TYPES_H_ */
