#include <signal.h>
#include <sys/time.h>

#include "lib_timer.h"



volatile unsigned int elapsed = 0;



/*
  signal handler
*/
void catch_sigalrm(int sig)
{
	if(SIGALRM == sig)
	{
		elapsed++;
	}
}



/*
  initialize inner timer

  @us_timer_period	inner timer period measured in microsecond(us)

  on success, 1 will be returned.
  on error, 0 will be returned.
*/
unsigned char timer_initialize(unsigned int us_timer_period)
{
	struct itimerval itimer;

	if( SIG_ERR == signal(SIGALRM, SIG_IGN) )
	{
		return 0;
	}

	elapsed = 0;
	itimer.it_value.tv_sec = itimer.it_interval.tv_sec = 0;
	itimer.it_value.tv_usec = itimer.it_interval.tv_usec = us_timer_period;

	if( SIG_ERR == signal(SIGALRM, catch_sigalrm) )
	{
		return 0;
	}
	if( -1 == setitimer(ITIMER_REAL, &itimer, 0) )
	{
		return 0;
	}

	return 1;
}



/*
  update all timers in timer set

  @itimer	the pointer point to timer set
  @ntimer	the number of timers in timer set
*/
void timer_update( 
	struct interval_timer *itimer, 
	unsigned int ntimer )
{
	int i, delta;
	
	delta = elapsed;
	elapsed = 0;

	if(0 == delta)
		return;

	for(i = 0; i < ntimer; ++i)
	{
		if((itimer[i].interval) > 0 && ((itimer[i].left) -= delta) <= 0)
		{
			itimer[i].left = itimer[i].interval;
			itimer[i].expire = 1;
		}
	}

}



/*
  stop a specific timer in timer set

  @index	index of a specific timer
  @itimer	the pointer which point to timer set
  @ntimer	the number of timers in timer set
*/
void timer_stop( 
	unsigned short index, 
	struct interval_timer *itimer, 
	unsigned int ntimer )
{
	if(index >= ntimer)
		return;

	itimer[index].interval = 0;
}


/*
  start a specific timer in timer set

  @index	index of a specific timer
  @interval	timer interval assign to a specific timer
  @itimer	the pointer which point to timer set
  @ntimer	the number of timers in timer set
  @us_timer_period	inner timer period measured in microsecond(us)
*/
void timer_start( 
	unsigned short index, 
	float interval, 
	struct interval_timer *itimer, 
	unsigned int ntimer, 
	unsigned int us_timer_period )
{
	if(index >= ntimer)
		return;

	itimer[index].expire = 0;

	itimer[index].left = (int)((interval * 1E6) / us_timer_period);
	if(itimer[index].left == 0)
	{
		//the interval is too small, raise it to 1 to make sure it expires as soon as possible
		itimer[index].left = 1;
	}
	itimer[index].interval = itimer[index].left;
}


/*
  check state of a specific timer
  
  @index	index of a specific timer
  @itimer	the pointer which point to timer set
  @ntimer	the number of timers in timer set
*/
unsigned char timer_expired( 
	unsigned short index, 
	struct interval_timer *itimer,
	unsigned int ntimer )
{
	timer_update(itimer, ntimer);

	if (index >= ntimer)
		return 0;

	if (0 == itimer[index].expire)
		return 0;

	itimer[index].expire = 0;
	
	return 1;
}


