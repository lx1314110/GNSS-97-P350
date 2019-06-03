#ifndef	__DNTP_IP1725__
#define	__DNTP_IP1725__

void SwitchRegWrite(int fpga_fd,unsigned char reg, unsigned short val);
void SwitchRegRead(int fpga_fd, unsigned char reg, unsigned short * val);


void SpanningTreeBlock(int fpga_fd);

void CreateWebNetWorkInterface();

void CreateNetWorkInterface();
void CreateAllNetWorkInterface();
void dt_port_linkstatus_all_get(int fpga_fd, unsigned int *link_status);

void SwitchConfig(int fpga_fd);

void SwitchInit();
void SwitchClean();


#endif
