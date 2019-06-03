#ifndef	__DNTP_TIMESTAMP__
#define	__DNTP_TIMESTAMP__






#include "dntp_type.h"
#include "dntp_tv2ts.h"
#include "dntp_ts2tv.h"





/*
 * Stuff for extracting things from li_vn_mode
 */
#define	PKT_MODE(li_vn_mode)	((u8_t)((li_vn_mode) & 0x7))
#define	PKT_VERSION(li_vn_mode)	((u8_t)(((li_vn_mode) >> 3) & 0x7))
#define	PKT_LEAP(li_vn_mode)	((u8_t)(((li_vn_mode) >> 6) & 0x3))





#define	HTONL_TS(h, n) do {\
	(n)->l_ui = htonl((h)->l_ui);\
	(n)->l_uf = htonl((h)->l_uf);\
	} while(0)





/*
 * Stuff for putting things back into li_vn_mode
 */
#define	PKT_LI_VN_MODE(li, vn, md) \
	((u8_t)((((li) << 6) & 0xc0) | (((vn) << 3) & 0x38) | ((md) & 0x7)))





/*
 * Convert usec to a time stamp fraction.
 */
#define	TVUTOTSF(tvu, tsf) \
	(tsf) = ustotslo[(tvu) & 0xff] \
	    + ustotsmid[((tvu) >> 8) & 0xff] \
	    + ustotshi[((tvu) >> 16) & 0xf]



	
	
/*
 * Convert a struct timeval to a time stamp.
 */
#define TVTOTS(tv, ts) \
	do { \
		(ts)->l_ui = (u32_t)(tv)->tv_sec; \
		TVUTOTSF((tv)->tv_usec, (ts)->l_uf); \
	} while(0)	





/*
 * TV_SHIFT is used to turn the table result into a usec value.  To round,
 * add in TV_ROUNDBIT before shifting
 */
#define	TV_SHIFT	3
#define	TV_ROUNDBIT	0x4	






/*
 * Convert a time stamp fraction to microseconds.  The time stamp
 * fraction is assumed to be unsigned.
 */
#define	TSFTOTVU(tsf, tvu) \
	(tvu) = (tstoushi[((tsf) >> 24) & 0xff] \
	    + tstousmid[((tsf) >> 16) & 0xff] \
	    + tstouslo[((tsf) >> 9) & 0x7f] \
	    + TV_ROUNDBIT) >> TV_SHIFT	





/*
 * Convert a time stamp to a struct timeval.  The time stamp
 * has to be positive.
 */
#define	TSTOTV(ts, tv) \
	do { \
		(tv)->tv_sec = (ts)->l_ui; \
		TSFTOTVU((ts)->l_uf, (tv)->tv_usec); \
		if ((tv)->tv_usec == 1000000) { \
			(tv)->tv_sec++; \
			(tv)->tv_usec = 0; \
		} \
	} while(0)	







#endif//__DNTP_TIMESTAMP__


