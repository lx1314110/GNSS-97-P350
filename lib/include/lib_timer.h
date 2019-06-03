#ifndef	__LIB_TIMER__
#define	__LIB_TIMER__



struct interval_timer {
	unsigned char expire;
	int	left;
	int	interval;
};



unsigned char timer_initialize(unsigned int us_timer_period);

void timer_update( 
	struct interval_timer *itimer, 
	unsigned int ntimer );

void timer_stop( 
	unsigned short index, 
	struct interval_timer *itimer, 
	unsigned int ntimer );

void timer_start( 
	unsigned short index, 
	float interval, 
	struct interval_timer *itimer, 
	unsigned int ntimer, 
	unsigned int us_timer_period );

unsigned char timer_expired( 
	unsigned short index, 
	struct interval_timer *itimer,
	unsigned int ntimer );





#endif//__LIB_TIMER__


