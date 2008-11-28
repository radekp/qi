#ifndef __PORTS_S3C24XX_H__
#define __PORTS_S3C24XX_H__


// I/O PORT
#define rGPACON			(*(volatile unsigned *)0x56000000)
#define rGPADAT			(*(volatile unsigned *)0x56000004)

#define rGPBCON			(*(volatile unsigned *)0x56000010)
#define rGPBDAT			(*(volatile unsigned *)0x56000014)
#define rGPBUP			(*(volatile unsigned *)0x56000018)

#define rGPCCON			(*(volatile unsigned *)0x56000020)
#define rGPCDAT			(*(volatile unsigned *)0x56000024)
#define rGPCUP			(*(volatile unsigned *)0x56000028)

#define rGPDCON			(*(volatile unsigned *)0x56000030)
#define rGPDDAT			(*(volatile unsigned *)0x56000034)
#define rGPDUP			(*(volatile unsigned *)0x56000038)

#define rGPECON			(*(volatile unsigned *)0x56000040)
#define rGPEDAT			(*(volatile unsigned *)0x56000044)
#define rGPEUP			(*(volatile unsigned *)0x56000048)

#define rGPFCON			(*(volatile unsigned *)0x56000050)
#define rGPFDAT			(*(volatile unsigned *)0x56000054)
#define rGPFUP			(*(volatile unsigned *)0x56000058)

#define rGPGCON			(*(volatile unsigned *)0x56000060)
#define rGPGDAT			(*(volatile unsigned *)0x56000064)
#define rGPGUP			(*(volatile unsigned *)0x56000068)

#define rGPHCON			(*(volatile unsigned *)0x56000070)
#define rGPHDAT			(*(volatile unsigned *)0x56000074)
#define rGPHUP			(*(volatile unsigned *)0x56000078)

#define rGPJCON    			(*(volatile unsigned *)0x560000d0)	//Port J control
#define rGPJDAT   			(*(volatile unsigned *)0x560000d4)	//Port J data
#define rGPJUP			(*(volatile unsigned *)0x560000d8)	//Port J data

#endif
