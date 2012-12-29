/*
 * cs5536_io.h
 * some basic access of msr read/write and gpio read/write. 
 * this access function only suitable before the virtual support module(VSM)
 * working for some simple debugs.
 *
 * Author : jlliu <liujl@lemote.com>
 * Date : 07-07-04
 *
 */
 
/******************************************************************************/

/*
 * rdmsr : read 64bits data from the cs5536 MSR register
 */
extern void _rdmsr(UINT32 msr, UINT32 *hi, UINT32 *lo);

/*
 * wrmsr : write 64bits data to the cs5536 MSR register
 */
extern void _wrmsr(UINT32 msr, UINT32 hi, UINT32 lo);

