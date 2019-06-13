data:20190613  modify content
1. modify in din_issue.c.
   >> ptpsendconfig function:
   add ptp2 stop; usleep(100000);
   ipconfig -a <ipaddress>; usleep(100000); 
   ipconfig -m <netmask>; usleep(100000);
   ipconfig -g <gateway>; usleep(800000);
   ptp2 start 0; usleep(100000);
   ...........
   unitl layer3 and unicast
2. modify in dout_issue.c.


   see the patch_1.diff.
   ----------------------------------------------
