#ifndef	__LIB_TYPE__
#define	__LIB_TYPE__


#define get_marco_name(x)   #x
#define get_marco_value(x)		get_marco_name(x)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))


typedef unsigned char		u8_t;
typedef signed char			s8_t;

typedef unsigned short		u16_t;
typedef signed short		s16_t;

typedef unsigned int		u32_t;
typedef signed int			s32_t;

typedef unsigned long long	u64_t;
typedef signed long long	s64_t;

typedef enum {false = 0, true = 1} bool_t;

#define MALLOC(a) malloc(a)
#define REALLOC(a, b) realloc(a, b)
#define CALLOC(a, b) calloc(a, b)
#define FREE(a) free((void *)(a)),(a)=NULL


#endif//__LIB_TYPE__



