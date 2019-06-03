#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "addr.h"
#include "lib_sema.h"
#include "lib_fpga.h"
#include "lib_ip1725.h"

//semaphore proj_id, ALL sema id same
enum {
	SEMA_PROJ_ID	= 'A',
	SEMA_PROJ_ID_IP1725	= 'B'
};

enum {
	SEMA_MEMBER_IP1725,
	SEMA_MEMBER_ARRAY_SIZE
};

static int switch_init_flag = 0;
static int ip1725_sem_id = -1;

#define	SEMA_PATH	"/usr/p350/"

void SwitchRegWrite(int fpga_fd,unsigned char reg, unsigned short val)
{
    u16_t chipReg = reg;
    SwitchInit();
	if(1 != sema_lock(ip1725_sem_id, SEMA_MEMBER_IP1725))
	{
		fprintf(stderr,"--Failed to lock semaphore.\n");
		return ;
	}
	//等待总线空闲
	usleep(5000);
    FpgaWrite(fpga_fd,FPGA_SYS_IP1725_ADDR, chipReg);
    FpgaWrite(fpga_fd,FPGA_SYS_IP1725_VAL, val);
    FpgaWrite(fpga_fd,FPGA_SYS_IP1725_REG_OP, 0x00);
    FpgaWrite(fpga_fd,FPGA_SYS_IP1725_REG_OP, 0x01);
	//等待操作结束
    usleep(5000);
	if(1 != sema_unlock(ip1725_sem_id, SEMA_MEMBER_IP1725)){
			fprintf(stderr,"--Failed to unlock semaphore.\n");
			return ;
	}
}
#if 1
void SwitchRegRead(int fpga_fd,unsigned char reg, unsigned short *val)
{
	int ret = -1;
    u16_t chipReg = reg;

	SwitchInit();

	if(1 != sema_lock(ip1725_sem_id, SEMA_MEMBER_IP1725))
	{
		fprintf(stderr,"--Failed to lock semaphore.\n");
		return ;
	}
	 //等待总线空闲
    usleep(1000);
    FpgaWrite(fpga_fd,FPGA_SYS_IP1725_ADDR, chipReg);
	FpgaWrite(fpga_fd,FPGA_SYS_IP1725_REG_OP, 0x00);
    FpgaWrite(fpga_fd,FPGA_SYS_IP1725_REG_OP, 0x02);
	usleep(5000);
    FpgaRead(fpga_fd,FPGA_SYS_IP1725_READ, val);
	//等待操作结束
    usleep(1000);
	ret = sema_unlock(ip1725_sem_id, SEMA_MEMBER_IP1725);
	if(1 != ret)
	{
		fprintf(stderr,"--Failed to unlock semaphore.\n");
		return ;
	}
}

void SpanningTreeBlock(int fpga_fd)
{
	/*const unsigned char BrcastStormDisableBaseReg = 0x86;
	//0x86      port 1-16    0 : disable, 1 : enable
	//0x87[8:0] port 17-25   0 : disable, 1 : enable
	const unsigned char BrcastStormCtlThreshold=0x87;
	//0x87[14:9] broadcast storm control threshold
	//0x87[15]   broadcast storm counter clear period selection  0 : 100M : 500us; 10M : 5ms    1 : 100M :10ms 10M :100ms
	
	SwitchRegWrite(BrcastStormDisableBaseReg,0xfff1);
	SwitchRegWrite(BrcastStormCtlThreshold,0x9c00);
	//ARP and ICMP storm control 
	SwitchRegWrite(0x88,0xffff);
	//Enable Source MAC address learning 
	SwitchRegWrite(0x8a,0xffff);
	//Bandwidth Control
	SwitchRegWrite(0x01,0x0000);
	unsigned char addr=0x09;
	do{
		SwitchRegWrite(addr,0x0202);
		addr++;
		}while(addr<0x15);
	//Block BroadCast Frames to CPU Port
	
	printf("BroadcastStormDisable\n");*/
	// IP1725 Non-association Port
	//0xED:         port 1-16    0 : non-association port can't be enabled , 1 : non-association port can be enabled 
	//0xEE[8:0]:    port 17-25   0 : non-association port can't be enabled , 1 : non-association port can be enabled 
	SwitchRegWrite(fpga_fd,0xED,0xfffa);
	SwitchRegWrite(fpga_fd,0xEE,0x007f);
}
#endif

void CreateNetWorkInterface()
{
    //创建20端口
    char cmd[50];
    u8_t i = 0;
	/*
	memset(cmd, 0x00, sizeof(cmd));
	sprintf(cmd, "vconfig add eth0 %d", 1);
	system(cmd);
	
	memset(cmd, 0x00, sizeof(cmd));
	sprintf(cmd, "vconfig set_flag eth0.%d 1 1", 1);
	system(cmd);
	*/
    for (i = 5; i < (5+20); i++)
    {
        memset(cmd, 0x00, sizeof(cmd));
        sprintf(cmd, "vconfig add eth0 %d", i);
        system(cmd);
        
        memset(cmd, 0x00, sizeof(cmd));
        sprintf(cmd, "vconfig set_flag eth0.%d 1 1", i);
        system(cmd);
		
	 memset(cmd, 0x00, sizeof(cmd));
        sprintf(cmd, "ifconfig eth0.%d up", i);
        system(cmd);
    }
	
}

void CreateWebNetWorkInterface()
{
    //创建20端口
    char cmd[50];

	memset(cmd, 0x00, sizeof(cmd));
	sprintf(cmd, "vconfig add eth0 %d", 2);
	system(cmd);
	
	memset(cmd, 0x00, sizeof(cmd));
	sprintf(cmd, "vconfig set_flag eth0.%d 1 1", 2);
	system(cmd);

	memset(cmd, 0x00, sizeof(cmd));
    sprintf(cmd, "ifconfig eth0.%d up", 2);
    system(cmd);
	
}

void CreateAllNetWorkInterface()
{
	CreateNetWorkInterface();
	CreateWebNetWorkInterface();
}

void SwitchConfig_enable_port(int fpga_fd)
{
	SwitchRegWrite(fpga_fd,0xEF, 0xFFFF);
	SwitchRegWrite(fpga_fd,0xF0, 0xFF);
	SwitchRegWrite(fpga_fd,0xF1, 0xFFFF);
	SwitchRegWrite(fpga_fd,0xF2, 0xFF);
}

void dt_port_linkstatus_all_get(int fpga_fd, unsigned int *link_status)
{
	int glport               = 3;
	unsigned char addr_base  = 0x66;
	unsigned int port_status = 0;
	unsigned short reg_value   = 0;
	
	for (addr_base = 0x65; addr_base <= 0x6d; addr_base++)
	{
		SwitchRegRead(fpga_fd, addr_base, &reg_value);
		//printf("addr[%02x] val[%04x]\n", addr_base, reg_value);
		
		if (0x65 == addr_base)
		{	
				if (reg_value & (0x1 << 5))
				{
					port_status |= 1 << 21; /* 管理端口号 */
				}
		}
		else if (0x66 == addr_base)
		{
			if (reg_value & (0x1 << 5)) 
			{
				port_status |= 1 << 1;	/* 端口1 */
			}
			
			if (reg_value & (0x1 << 10)) 
			{
				port_status |= 1 << 2;  /* 端口2 */
			}
		}
		else /* 端口3-端口20 */
		{
			if (reg_value & 0x1) 
			{
				port_status |= 1 << glport;	
			}
			
			if (reg_value & (0x1 << 5)) 
			{
				port_status |= 1 << (glport + 1);	
			}
			
			if (reg_value & (0x1 << 10)) 
			{
				port_status |= 1 << (glport + 2);	 
			}
			
			glport += 3; /* 每三个端口对应一个寄存器 */
		}
	}
	
	*link_status = port_status;
	//printf("port_status[%0x]\n", port_status);
	
	return;		
}


void SwitchConfig(int fpga_fd)
{
    u8_t i=0;
    u8_t addr = 0x0;
    u16_t vlanTag = 3; 
    u16_t regVal = 0;
//	u16_t val;
//	unsigned int vid;
//	unsigned short reg;
	//阻止Spanning Tree Protocol风暴

	//SpanningTreeBlock(fpga_fd);
	
    //Set VLAN type 802.1Q based VLAN
    SwitchRegWrite(fpga_fd,0x82, 0x0880);   
    // storm control setting
	SwitchRegWrite(fpga_fd,0x86,0xFFFE);
	SwitchRegWrite(fpga_fd,0x87,0xFFFF);
    //enable arp/icmp storm control setting
	SwitchRegWrite(fpga_fd,0x88,0xFFFF);
	//Aging timer threshold setting :(mg_aging_time + 1) x 27.6 sec. default: 0xA:302s
	//56s
	SwitchRegWrite(fpga_fd,0x89,0x01);

    //Prot1 vlan Group
    //SwitchRegWrite(fpga_fd,0xAD,0xFFF1);    //把1号端口和4到23号口划为一个VLAN
    //SwitchRegWrite(fpga_fd,0xAE,0x00FF); 
	
    //SwitchRegWrite(fpga_fd,0xAF, 0x3);    //把管理端口2和1号口划为一个VLAN
	
	//read
	/*
	for(i = 0; i < 32; i++)
	{

		
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (0<< 14) | (1 << 9) | (1 << 6) | (0<< 5) | (i & 0x1f));	
		SwitchRegRead(fpga_fd, 0xFC, &reg);
		vid = reg;
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (0<< 14) | (1 << 9) | (1 << 6) | (1<< 5) | (i & 0x1f));	
		SwitchRegRead(fpga_fd, 0xFC, &reg);
		vid = (vid << 16) | reg;
		printf("entry %d: %08x\n", i, vid);
	}

	for(i = 0; i < 32; i++)
	{

		
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (0<< 14) | (1 << 9) | (0 << 6) | (0<< 5) | (i & 0x1f));	
		SwitchRegRead(fpga_fd, 0xFC, &reg);
		vid = reg;
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (0<< 14) | (1 << 9) | (0 << 6) | (1<< 5) | (i & 0x1f));	
		SwitchRegRead(fpga_fd, 0xFC, &reg);
		vid = (vid << 16) | reg;
		printf("entry %d: %08x\n", i, vid);
	}
	*/
	/*
	FA:	command register
	BIT 5:	1(low) 0(high)
	BIT 14:	1(write) 0(read)
	FC: data register
	
	VID/VID _TAG data
	
	*/
	
	
	//write VID_TAG table

	for(i = 0; i < 32; i++)
	{
		SwitchRegWrite(fpga_fd,0xFC, 1);
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (1<< 14) | (1 << 9) | (1 << 6) | (0<< 5) | (i & 0x1f));
		SwitchRegWrite(fpga_fd,0xFC, 0);
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (1<< 14) | (1 << 9) | (1 << 6) | (1<< 5) | (i & 0x1f));
	}


	//write VLAN_ID table
	for(i = 0; i < 32; i++)
	{
		SwitchRegWrite(fpga_fd,0xFC, i+1);
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (1<< 14) | (1 << 9) | (0 << 6) | (0<< 5) | (i & 0x1f));
		SwitchRegWrite(fpga_fd,0xFC, i+1);
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (1<< 14) | (1 << 9) | (0 << 6) | (1<< 5) | (i & 0x1f));
	}



	/*

	//read
	for(i = 0; i < 32; i++)
	{

		
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (0<< 14) | (1 << 9) | (1 << 6) | (0<< 5) | (i & 0x1f));	
		SwitchRegRead(fpga_fd, 0xFC, &reg);
		vid = reg;
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (0<< 14) | (1 << 9) | (1 << 6) | (1<< 5) | (i & 0x1f));	
		SwitchRegRead(fpga_fd, 0xFC, &reg);
		vid = (reg << 16) | vid;
		printf("entry %d: %08x\n", i, vid);
	}
	
	for(i = 0; i < 32; i++)
	{

		
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (0<< 14) | (1 << 9) | (0 << 6) | (0<< 5) | (i & 0x1f));	
		SwitchRegRead(fpga_fd, 0xFC, &reg);
		vid = reg;
		SwitchRegWrite(fpga_fd,0xFA, (1<<15) | (0<< 14) | (1 << 9) | (0 << 6) | (1<< 5) | (i & 0x1f));	
		SwitchRegRead(fpga_fd, 0xFC, &reg);
		vid = (vid << 16) | reg;
		printf("entry %d: %08x\n", i, vid);
	}
	*/
    addr = 0xAD;
    //配置Vlan Table，转发表
    for (i = 0; i < 24; i++)
    { 

		if(i<16)
		{
			SwitchRegWrite(fpga_fd,addr, (1<<0)|(1<<i));
			//printf("addr: %x  %x\n",addr, (1<<0)|(1<<i));
			SwitchRegWrite(fpga_fd,addr+1, 0);
		}
		else
		{
			SwitchRegWrite(fpga_fd,addr, 1<<0); 
			SwitchRegWrite(fpga_fd,addr+1, 1<<(i-16));
		}
		
          //把每一个端口和1号端口划为一组
		//SwitchRegWrite(fpga_fd,addr, (1<<0)|(1<<i));
        addr += 2;
    }

	//关闭MAC学习
	SwitchRegWrite(fpga_fd,0x8A, 0x0000);
    SwitchRegWrite(fpga_fd,0x8B, 0xFF00);
	
    //使能ADD VLAN
    //SwitchRegWrite(fpga_fd,0x97, 0xFFFF);
    //SwitchRegWrite(fpga_fd,0x98, 0xFFFF);

    //Remove VLAN
    //SwitchRegWrite(fpga_fd,0x99, 0xFFFE);
    //SwitchRegWrite(fpga_fd,0x9A, 0xFFFF);

    //使能VLAN Table
    //SwitchRegWrite(fpga_fd,0xFA, 0xFFFF);
    //SwitchRegWrite(fpga_fd,0xFC, 0xFFFF);

//配置PVID
	//Config port1-3 Vlan Tag Id,0x9B
	//regVal = (1<<0) | (2<<5) | (1<<10);//把各个端口进来的包隔离开来
	//SwitchRegWrite(fpga_fd, 0x9b, regVal);
	
    //Config Vlan Tag Id,0x9B~0xA3
    addr = 0x9b;
    vlanTag = 0;
    do
    {
			
			regVal = 0x00;
			regVal = (vlanTag<<0) | ((vlanTag+1)<<5) | ((vlanTag+2)<<10);////把各个端口进来的包隔离开来
			vlanTag += 3;
       
        SwitchRegWrite(fpga_fd,addr, regVal);
        addr++;
    }while(addr < 0xA3);
    
    //使能对应端口添加vlan tag功能
   // SwitchRegWrite(fpga_fd,0x47, 0xFFF2);
    //SwitchRegWrite(fpga_fd,0x48, 0x00FF); 
	
	//flush port base addr
	SwitchRegWrite(fpga_fd,0xF3,0xFFFF);
	SwitchRegWrite(fpga_fd,0xF4,0x03FF);
	
    SwitchConfig_enable_port(fpga_fd);
	//recovery port base addr
	SwitchRegWrite(fpga_fd,0xF3,0x0);
	SwitchRegWrite(fpga_fd,0xF4,0x0);

	SwitchRegWrite(fpga_fd,0x6E,0x01);
	
	#if 0
	SwitchRegWrite(fpga_fd,0x86,0xFFFF);
	SwitchRegWrite(fpga_fd,0x87,0x7EFF);
	SwitchRegWrite(fpga_fd,0x88,0x1FFF);
	#endif
	
}

static int initializeSemaphoreSet(int *sem_id)
{
	int id = -1;
	#define SEMA_NUMS 1
	//initialize semaphore set
	id = sema_create(SEMA_PATH, SEMA_PROJ_ID_IP1725, SEMA_NUMS);
	if(id < 0)
	{
		return -1;
	}

	*sem_id = id;

	return 0;
}

/*
  -1	失败
   0	成功
*/
static int cleanSemaphoreSet(int sem_id)
{
	
	if(-1 != sem_id)
	{
		if( 0 == sema_remove(sem_id) )
		{
			return -1;
		}
	}
	return 0;
}

void SwitchInit()
{
	if (switch_init_flag == 1)
		return;

	switch_init_flag = 1;
	if(0 != initializeSemaphoreSet(&ip1725_sem_id)){
		fprintf(stderr,"ip1725:initializeSemaphoreSet failture.\n");
	}
	return;

}

void SwitchClean()
{
	if (switch_init_flag == 0)
		return;

	if(cleanSemaphoreSet(ip1725_sem_id) == 0){
		ip1725_sem_id = -1;
		switch_init_flag = 0;
	}
	return;

}


