#ifndef _ASM_ADDR_BASE_H
#define _ASM_ADDR_BASE_H

#ifndef _FPGA_ADDR_SLOTBITS
#define _FPGA_ADDR_SLOTBITS	5
#endif
#if 1
#ifndef _FPGA_ADDR_IOTYPEBITS
#define _FPGA_ADDR_IOTYPEBITS	1
#endif
#ifndef _FPGA_ADDR_BDTYPEBITS
#define _FPGA_ADDR_BDTYPEBITS	5
#endif
#else
#ifndef _FPGA_ADDR_BDTYPEBITS
#define _FPGA_ADDR_BDTYPEBITS	6
#endif

#endif
#ifndef _FPGA_ADDR_OFFSETBITS
#define _FPGA_ADDR_OFFSETBITS	5
#endif

#define _FPGA_ADDR_SLOTMASK	((1 << _FPGA_ADDR_SLOTBITS)-1)
#if 1
#define _FPGA_ADDR_IOTYPEMASK	((1 << _FPGA_ADDR_IOTYPEBITS)-1)
#endif
#define _FPGA_ADDR_BDTYPEMASK	((1 << _FPGA_ADDR_BDTYPEBITS)-1)
#define _FPGA_ADDR_OFFSETMASK	((1 << _FPGA_ADDR_OFFSETBITS)-1)

#define _FPGA_ADDR_OFFSETSHIFT	0
#define _FPGA_ADDR_BDTYPESHIFT	(_FPGA_ADDR_OFFSETSHIFT+_FPGA_ADDR_OFFSETBITS)
#if 1
#define _FPGA_ADDR_IOTYPESHIFT	(_FPGA_ADDR_BDTYPESHIFT+_FPGA_ADDR_BDTYPEBITS)
#define _FPGA_ADDR_SLOTSHIFT	(_FPGA_ADDR_IOTYPESHIFT+_FPGA_ADDR_IOTYPEBITS)
#else
#define _FPGA_ADDR_SLOTSHIFT	(_FPGA_ADDR_BDTYPESHIFT+_FPGA_ADDR_BDTYPEBITS)
#endif

/*
 * Direction bits, which any architecture can choose to override
 * before including this file.
 */

//#ifndef _FPGA_ADDR_NONE
//# define _FPGA_ADDR_NONE	0U
//#endif

#if 1
#define _FPGA_ADDR_INTYPE	1
#define _FPGA_ADDR_OUTTYPE	0

/*
*	_FPGA_ADDR(slot,io_type,bdtype,offset)
*	slot: 0-31
*	iotype: 1-input 0-output
*	bdtype:0-63
*	offset:0-31
*  high --------------------- low
*   slot   iotype bdtype  offset
*   5      1     5           5
*/
#define _FPGA_ADDR(slot,iotype,bdtype,offset) \
	(((slot)  << _FPGA_ADDR_SLOTSHIFT) | \
	 ((iotype) << _FPGA_ADDR_IOTYPESHIFT) | \
	 ((bdtype)   << _FPGA_ADDR_BDTYPESHIFT) | \
	 ((offset) << _FPGA_ADDR_OFFSETSHIFT))

#define _FPGA_IN_ADDR(slot,bdtype,offset) _FPGA_ADDR((slot),_FPGA_ADDR_INTYPE,(bdtype),(offset))
#define _FPGA_OUT_ADDR(slot,bdtype,offset) _FPGA_ADDR((slot),_FPGA_ADDR_OUTTYPE,(bdtype),(offset))

#else
/*
*	_FPGA_ADDR(slot,bdtype,offset)
*	slot: 0-31
*	bdtype:0-63
*	offset:0-31
*/
#define _FPGA_ADDR(slot,bdtype,offset) \
	(((slot)  << _FPGA_ADDR_SLOTSHIFT) | \
	 ((bdtype)   << _FPGA_ADDR_BDTYPESHIFT) | \
	 ((offset) << _FPGA_ADDR_OFFSETSHIFT))

#endif

#endif
