/************************************************************
*author  Linqing
*Email   linqinga@datang.com
*date:	2014-07-11
*************************************************************/
#include <stdio.h>
#include <unistd.h>
#include "lib_cpu.h"
#include "lib_dbg.h"
#include "alloc.h"

/**
 * @brief use linux3.0.x
 * @details [long description]
 * 
 */
typedef struct jiffy_counts_t {
	unsigned long long usr, nic, sys, idle;
	unsigned long long iowait, irq, softirq, steal;
	unsigned long long total;
	unsigned long long busy;
} jiffy_counts_t;

#define CPU_INFO_PATH 	"/proc/stat"
#define LINE_BUF_NO 	500

/**
 * @brief 	cpu_utilization
 * @details [long description]
 * @return 	CPU utilization percentage example:return 1 -> 1%
 */
int cpu_utilization(void)
{
	static const char fmt[] = "cpu %llu %llu %llu %llu %llu %llu %llu %llu";	
	char line_buf[LINE_BUF_NO];
	jiffy_counts_t jiffy;
	jiffy_counts_t jiffyold;

	int cpu_util;
	int ret;
	unsigned long long total_old;
	unsigned long long total;
	unsigned long long busy_old;
	unsigned long long busy;

	FILE *fp = fopen(CPU_INFO_PATH,"r");

	if (!fgets(line_buf,LINE_BUF_NO,fp)){
		print(DBG_ERROR, "cpu utilization fgets err \n");
	}
	ret = sscanf(line_buf, fmt,
			&jiffyold.usr, &jiffyold.nic, &jiffyold.sys, &jiffyold.idle,
			&jiffyold.iowait, &jiffyold.irq, &jiffyold.softirq,
			&jiffyold.steal);
        busy_old = jiffyold.usr + jiffyold.nic +jiffyold.sys;
        total_old = jiffyold.usr + jiffyold.nic +jiffyold.sys + jiffyold.idle;

	fclose(fp);

	sleep(1);

    fp = fopen(CPU_INFO_PATH,"r");

    if (!fgets(line_buf,LINE_BUF_NO,fp)){
            print(DBG_ERROR, "cpu utilization fgets err \n");
    }
	 ret = sscanf(line_buf, fmt,
                        &jiffy.usr, &jiffy.nic, &jiffy.sys, &jiffy.idle,
                        &jiffy.iowait, &jiffy.irq, &jiffy.softirq,
                        &jiffy.steal);

	
	busy = jiffy.usr + jiffy.nic +jiffy.sys;
	total = jiffy.usr + jiffy.nic +jiffy.sys + jiffy.idle;
	cpu_util = (busy - busy_old)*100.0 /(total- total_old); 
	#if CPU_UTILIZATION_DEBUG	
	printf("busy is %llu  busy_old is %llu total is %llu  total_old is %llu\n",busy,busy_old,total,total_old);
	printf("usr is %llu usr_old is %llu \n",jiffyold.usr,jiffy.usr);
	printf("cpu is %d \n",cpu_util);
	#endif
	fclose(fp);	

	return cpu_util;
} 

#if CPU_UTILIZATION_DEBUG
int main(int argc, char argv[])
{
	 cpu_utilization();
}
#endif
