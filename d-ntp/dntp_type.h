#ifndef	__DNTP_TYPE__
#define	__DNTP_TYPE__






#include "lib_type.h"






/*
 * l_fp is 64 bits long with the decimal between bits 31 and 32.
 * This is used for time stamps in the NTP packet header (in network
 * byte order) and for internal computations of offsets (in local host
 * byte order). We use the same structure for both signed and unsigned
 * values, which is a big hack but saves rewriting all the operators
 * twice. Just to confuse this, we also sometimes just carry the
 * fractional part in calculations, in both signed and unsigned forms.
 *
 */
typedef struct {
	union {
		u32_t Xl_ui;
		int Xl_i;
	} Ul_i;
	union {
		u32_t Xl_uf;
		int Xl_f;
	} Ul_f;
} l_fp;






#define	l_ui	Ul_i.Xl_ui		/* unsigned integral part */
#define	l_i		Ul_i.Xl_i		/* signed integral part */
#define	l_uf	Ul_f.Xl_uf		/* unsigned fractional part */
#define	l_f		Ul_f.Xl_f		/* signed fractional part */






struct ntpkt {
	u8_t	LVM;
	u8_t	Stratum;
	u8_t	Poll;
	char	Precision;
	int		RootDelay;
	u32_t	RootDispersion;
	u8_t	RefId[4];
	l_fp 	RefStamp;
	l_fp 	OrgStamp;
	l_fp	RecStamp;
	l_fp 	TranStamp;
	
	u32_t	KeyId;
	u8_t	Digest[16];
};







#endif//__DNTP_TYPE__



