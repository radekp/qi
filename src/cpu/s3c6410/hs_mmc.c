#include <qi.h>
#include "hs_mmc.h"
#include <string.h>
#include <glamo-mmc.h>

#define HCLK_OPERATION
#undef DEBUG_HSMMC
#ifdef DEBUG_HSMMC
#define dbg(x...)       printf(x)
#else
#define dbg(x...)       do { } while (0)
#endif

//#include <linux-mmc.h>
#include <linux-mmc-protocol.h>
#include <s3c6410.h>
//#include <linux/mmc/protocol.h>
//#include <asm/io.h>
//#include <movi.h>

#include "hs_mmc.h"
#include <mmc.h>

#define SDI_Tx_buffer_HSMMC		(0x51000000)
#define SDI_Rx_buffer_HSMMC		(0x51000000+(0x300000))
#define SDI_Compare_buffer_HSMMC	(0x51000000+(0x600000))

#define Card_OneBlockSize_ver1		512

#define MMC_DEFAULT_RCA			(1<<16)

/*  Global variables */

static u32 rd_cnt_HSMMC;
//static u32 wt_cnt_HSMMC;
static u32 BlockNum_HSMMC = 0;

//static u32 WriteBlockCnt_INT = 0;
static u32 ReadBlockCnt_INT = 0;
//static u32 WRITEINT_DONE = 0;
//static u32 READINT_DONE = 0;
//static u32 COMPARE_INT_DONE = 0;
//static u32 CompareCnt_INT = 0;
//static u32 BufferBoundary_INT_Cnt = 0;

static u32 HS_DMA_END = 0;
//static u32 HS_CARD_DETECT = 0;

//static u32 ocr_check = 0;
//static u32 mmc_card = 0;
static u32 rca = 0;

static ulong HCLK;
//static u32 card_mid = 0;

int movi_hc = 1; /* sdhc style block indexing */
enum card_type card_type;

/* extern functions */
extern ulong get_HCLK(void);


#define s3c_hsmmc_readl(x)	*((unsigned int *)(((ELFIN_HSMMC_BASE + (HSMMC_CHANNEL * 0x100000)) + (x))))
#define s3c_hsmmc_readw(x)	*((unsigned short *)(((ELFIN_HSMMC_BASE + (HSMMC_CHANNEL * 0x100000)) + (x))))
#define s3c_hsmmc_readb(x)	*((unsigned char *)(((ELFIN_HSMMC_BASE + (HSMMC_CHANNEL * 0x100000)) + (x))))

#define s3c_hsmmc_writel(v,x)	*((unsigned int *) (((ELFIN_HSMMC_BASE + (HSMMC_CHANNEL * 0x100000)) + (x)))) = v
#define s3c_hsmmc_writew(v,x)	*((unsigned short *)(((ELFIN_HSMMC_BASE + (HSMMC_CHANNEL * 0x100000)) + (x)))) = v
#define s3c_hsmmc_writeb(v,x)	*((unsigned char *)(((ELFIN_HSMMC_BASE + (HSMMC_CHANNEL * 0x100000)) + (x)))) = v

#define readl(x)	*((unsigned int *)(x))
#define writel(v, x)	*((unsigned int *)(x)) = v

#define UNSTUFF_BITS(resp,start,size)                                   \
        ({                                                              \
                const int __size = size;                                \
                const u32 __mask = (__size < 32 ? 1 << __size : 0) - 1; \
                const int __off = 3 - ((start) / 32);                   \
                const int __shft = (start) & 31;                        \
                u32 __res;                                              \
                                                                        \
                __res = resp[__off] >> __shft;                          \
                if (__size + __shft > 32)                               \
                        __res |= resp[__off-1] << ((32 - __shft) & 31); \
                __res & __mask;                                         \
        })

static int wait_for_cmd_done (void)
{
	u32 i;
	ushort n_int, e_int;

	dbg("wait_for_cmd_done\n");
	for (i = 0; i < 0x20000000; i++) {
		n_int = s3c_hsmmc_readw(HM_NORINTSTS);
		dbg("  HM_NORINTSTS: %04x\n", n_int);
		if (n_int & 0x8000)
			/* any error */
			break;
		if (n_int & 0x0001)
			/* command complete */
			return 0;
	}

	e_int = s3c_hsmmc_readw(HM_ERRINTSTS);
	s3c_hsmmc_writew(e_int, HM_ERRINTSTS);
	s3c_hsmmc_writew(n_int, HM_NORINTSTS);
	puts("cmd error1: 0x");
	print32(e_int);
	puts(", HM_NORINTSTS: 0x");
	print32(n_int);
	puts("\n");

	return -1;
}


static void ClearCommandCompleteStatus(void)
{
	s3c_hsmmc_writew(1 << 0, HM_NORINTSTS);
	while (s3c_hsmmc_readw(HM_NORINTSTS) & 0x1) {
		s3c_hsmmc_writew(1 << 0, HM_NORINTSTS);
	}
}

static void card_irq_enable(ushort temp)
{
	s3c_hsmmc_writew((s3c_hsmmc_readw(HM_NORINTSTSEN) & 0xFEFF) | (temp << 8), HM_NORINTSTSEN);
}

void hsmmc_reset (void)
{
	s3c_hsmmc_writeb(0x3, HM_SWRST);
}

void hsmmc_set_gpio (void)
{
	u32 reg;

	reg = readl(GPGCON) & 0xf0000000;
	writel(reg | 0x02222222, GPGCON);

	reg = readl(GPGPUD) & 0xfffff000;
	writel(reg, GPGPUD);
}

static void set_transfer_mode_register (u32 MultiBlk, u32 DataDirection, u32 AutoCmd12En, u32 BlockCntEn, u32 DmaEn)
{
	s3c_hsmmc_writew((s3c_hsmmc_readw(HM_TRNMOD) & ~(0xffff)) | (MultiBlk << 5)
		| (DataDirection << 4) | (AutoCmd12En << 2)
		| (BlockCntEn << 1) | (DmaEn << 0), HM_TRNMOD);
//	dbg("\nHM_TRNMOD = 0x%04x\n", HM_TRNMOD);
}

static void set_arg_register (u32 arg)
{
	s3c_hsmmc_writel(arg, HM_ARGUMENT);
}

static void set_blkcnt_register(ushort uBlkCnt)
{
	s3c_hsmmc_writew(uBlkCnt, HM_BLKCNT);
}

static void SetSystemAddressReg(u32 SysAddr)
{
	s3c_hsmmc_writel(SysAddr, HM_SYSAD);
}

static void set_blksize_register(ushort uDmaBufBoundary, ushort uBlkSize)
{
	s3c_hsmmc_writew((uDmaBufBoundary << 12) | (uBlkSize), HM_BLKSIZE);
}

static void ClearErrInterruptStatus(void)
{
	while (s3c_hsmmc_readw(HM_NORINTSTS) & (0x1 << 15)) {
		s3c_hsmmc_writew(s3c_hsmmc_readw(HM_NORINTSTS), HM_NORINTSTS);
		s3c_hsmmc_writew(s3c_hsmmc_readw(HM_ERRINTSTS), HM_ERRINTSTS);
	}
}

static void InterruptEnable(ushort NormalIntEn, ushort ErrorIntEn)
{
	ClearErrInterruptStatus();
	s3c_hsmmc_writew(NormalIntEn, HM_NORINTSTSEN);
	s3c_hsmmc_writew(ErrorIntEn, HM_ERRINTSTSEN);
}

static void hsmmc_clock_onoff (int on)
{
	u16 reg16;

	if (on == 0) {
		reg16 = s3c_hsmmc_readw(HM_CLKCON) & ~(0x1<<2);
		s3c_hsmmc_writew(reg16, HM_CLKCON);
	} else {
		reg16 = s3c_hsmmc_readw(HM_CLKCON);
		s3c_hsmmc_writew(reg16 | (0x1<<2), HM_CLKCON);

		while (1) {
			reg16 = s3c_hsmmc_readw(HM_CLKCON);
			if (reg16 & (0x1<<3))	/*  SD_CLKSRC is Stable */
				break;
		}
	}
}

static void set_clock (u32 clksrc, u32 div)
{
	u16 reg16;
	u32 i;

	s3c_hsmmc_writel(0xC0004100 | (clksrc << 4), HM_CONTROL2);	// rx feedback control
	s3c_hsmmc_writel(0x00008080, HM_CONTROL3); 			// Low clock: 00008080
	s3c_hsmmc_writel(0x3 << 16, HM_CONTROL4);

	s3c_hsmmc_writew(s3c_hsmmc_readw(HM_CLKCON) & ~(0xff << 8), HM_CLKCON);

	/* SDCLK Value Setting + Internal Clock Enable */
	s3c_hsmmc_writew(((div<<8) | 0x1), HM_CLKCON);

	/* CheckInternalClockStable */
	for (i = 0; i < 0x10000; i++) {
		reg16 = s3c_hsmmc_readw(HM_CLKCON);
		if (reg16 & 0x2)
			break;
	}
	if (i == 0x10000)
		puts("internal clock stabilization failed\n");

	hsmmc_clock_onoff(1);
}

static void set_cmd_register (ushort cmd, u32 data, u32 flags)
{
	ushort val = (cmd << 8);

	if (cmd == 12)
		val |= (3 << 6);

	if (flags & MMC_RSP_136)	/* Long RSP */
		val |= 0x01;
	else if (flags & MMC_RSP_BUSY)	/* R1B */
		val |= 0x03;
	else if (flags & MMC_RSP_PRESENT)	/* Normal RSP */
		val |= 0x02;

	if (flags & MMC_RSP_OPCODE)
		val |= (1<<4);

	if (flags & MMC_RSP_CRC)
		val |= (1<<3);

	if (data)
		val |= (1<<5);

//	puts("cmdreg = 0x");
//	print32(val);
//	puts("\n");
	s3c_hsmmc_writew(val, HM_CMDREG);
}

static int issue_command (ushort cmd, u32 arg, u32 data, u32 flags)
{
	int i;

/*	puts("### issue_command: ");
	printdec(cmd);
	puts(" 0x");
	print32(arg);
	puts(" ");
	printdec(data);
	puts(" 0x");
	print32(flags);
	puts("\n");
*/
	/* Check CommandInhibit_CMD */
	for (i = 0; i < 0x1000000; i++) {
		if (!(s3c_hsmmc_readl(HM_PRNSTS) & 0x1))
			break;
	}
	if (i == 0x1000000) {
		puts("@@@@@@1 rHM_PRNSTS: ");
		printdec(s3c_hsmmc_readl(HM_PRNSTS));
		puts("\n");
	}

	/* Check CommandInhibit_DAT */
	if (flags & MMC_RSP_BUSY) {
		for (i = 0; i < 0x1000000; i++) {
			if (!(s3c_hsmmc_readl(HM_PRNSTS) & 0x2))
				break;
		}
		if (i == 0x1000000) {
			puts("@@@@@@2 rHM_PRNSTS: ");
			print32(s3c_hsmmc_readl(HM_PRNSTS));
			puts("\n");
		}
	}

	s3c_hsmmc_writel(arg, HM_ARGUMENT);

	set_cmd_register(cmd, data, flags);

	if (wait_for_cmd_done())
		return 0;

	ClearCommandCompleteStatus();

	if (!(s3c_hsmmc_readw(HM_NORINTSTS) & 0x8000))
		return 1;

	puts("Command = ");
	printdec((s3c_hsmmc_readw(HM_CMDREG) >> 8));
	puts(", Error Stat = 0x");
	print32(s3c_hsmmc_readw(HM_ERRINTSTS));
	return 0;
}

static int check_card_status(void)
{
	if (!issue_command(MMC_SEND_STATUS, rca<<16, 0, MMC_RSP_R1))
		return 0;

	if (((s3c_hsmmc_readl(HM_RSPREG0) >> 9) & 0xf) == 4) {
//		puts("Card is transfer status\n");
		return 1;
	}

	return 1;
}

static void set_hostctl_speed (u8 mode)
{
	u8 reg8;

	reg8 = s3c_hsmmc_readb(HM_HOSTCTL) & ~(0x1<<2);
	s3c_hsmmc_writeb(reg8 | (mode<<2), HM_HOSTCTL);
}

/* return 0: OK
 * return -1: error
 */
static int set_bus_width (u32 width)
{
	u8 reg = s3c_hsmmc_readb(HM_HOSTCTL);
	u8 bitmode = 0;

	card_irq_enable(0);	// Disable sd card interrupt


	if (!issue_command(MMC_APP_CMD, rca<<16, 0, MMC_RSP_R1))
		return -1;
	else {
		if (width == 1) {	// 1-bits
			bitmode = 0;
			if (!issue_command(MMC_SWITCH, 0, 0, MMC_RSP_R1B))
				return -1;
		} else {	// 4-bits
			bitmode = 1;
			if (!issue_command(MMC_SWITCH, 2, 0, MMC_RSP_R1B))
				return -1;
		}
	}

	if (bitmode == 2)
		reg |= 1 << 5;
	else
		reg |= bitmode << 1;

	s3c_hsmmc_writeb(reg, HM_HOSTCTL);
	card_irq_enable(1);
//	puts(" transfer rHM_HOSTCTL(0x28) = 0x");
//	print32(s3c_hsmmc_readb(HM_HOSTCTL));

	return 0;
}

static void clock_config (u32 Divisior)
{
	if (100000000 / (Divisior * 2) > 25000000) 	// Higher than 25MHz, it is necessary to enable high speed mode of the host controller.
		set_hostctl_speed(HIGH);
	else
		set_hostctl_speed(NORMAL);

	hsmmc_clock_onoff(0);		// when change the sd clock frequency, need to stop sd clock.
	set_clock(SD_EPLL, Divisior);
}

static void check_dma_int (void)
{
	u32 i;

	for (i = 0; i < 0x1000000; i++) {
		if (s3c_hsmmc_readw(HM_NORINTSTS) & 0x0002) {
			HS_DMA_END = 1;
			s3c_hsmmc_writew(s3c_hsmmc_readw(HM_NORINTSTS) | 0x0002, HM_NORINTSTS);
			break;
		}
		if (s3c_hsmmc_readw(HM_NORINTSTS) & 0x8000) {
			puts("error found: ");
			print32(s3c_hsmmc_readw(HM_ERRINTSTS));
			break;
		}
	}
}


static void print_sd_cid(const struct sd_cid *cid)
{
	puts("    Card Type: ");
	switch (card_type) {
	case CARDTYPE_NONE:
		puts("(None) / ");
		break;
	case CARDTYPE_MMC:
		puts("MMC / ");
		break;
	case CARDTYPE_SD:
		puts("SD / ");
		break;
	case CARDTYPE_SD20:
		puts("SD 2.0 / ");
		break;
	case CARDTYPE_SDHC:
		puts("SD 2.0 SDHC / ");
		break;
	}

	puts("Mfr: 0x");
	print8(cid->mid);
	puts(", OEM \"");
	this_board->putc(cid->oid_0);
	this_board->putc(cid->oid_1);
	puts("\" / ");

	this_board->putc(cid->pnm_0);
	this_board->putc(cid->pnm_1);
	this_board->putc(cid->pnm_2);
	this_board->putc(cid->pnm_3);
	this_board->putc(cid->pnm_4);
	puts("\", rev ");
	printdec(cid->prv >> 4);
	puts(".");
	printdec(cid->prv & 15);
	puts(" / s/n: ");
	print32(cid->psn_0 << 24 | cid->psn_1 << 16 | cid->psn_2 << 8 |
	    cid->psn_3);
	puts(" / date: ");
	printdec(cid->mdt_1 & 15);
	puts("/");
	printdec(2000 + ((cid->mdt_0 & 15) << 4)+((cid->mdt_1 & 0xf0) >> 4));
	puts("\n");
}

unsigned int s3c6410_mmc_init (int verbose)
{
	u32 reg;
	u32 width;
	int resp;
	int hcs;
	int retries = 50;
	u8 response[16];
	unsigned int r1[4];
	struct sd_cid *sd_cid = (struct sd_cid *)response;
	struct mmc_csd *csd = (struct mmc_csd *)response;
	u8 *p8 = (u8 *)&r1[0];
	unsigned int sd_sectors = 0;
	/* we need to shift result by 8 bits spread over 4 x 32-bit regs */
	u8 mangle[] = { 7, 0, 1, 2, 11, 4, 5, 6, 15, 8, 9, 10, 0, 12, 13, 14 };
	int n;

	hsmmc_set_gpio();

	hsmmc_reset();

	width = 4;

	HCLK = 33000000; /* FIXME */
	hsmmc_clock_onoff(0);

	reg = readl(SCLK_GATE);
	writel(reg | (1<<27), SCLK_GATE);

	set_clock(SD_EPLL, 0x80);
	s3c_hsmmc_writeb(0xe, HM_TIMEOUTCON);
	set_hostctl_speed(NORMAL);

	InterruptEnable(0xff, 0xff);

//	dbg("HM_NORINTSTS = %x\n", s3c_hsmmc_readw(HM_NORINTSTS));

	/* MMC_GO_IDLE_STATE */
	issue_command(MMC_GO_IDLE_STATE, 0x00, 0, 0);

	udelay(100000);
	udelay(100000);
	udelay(100000);
	udelay(100000);

	/* SDHC card? */

	resp = issue_command(SD_SEND_IF_COND, 0x000001aa,
		0, MMC_CMD_BCR | MMC_RSP_R7);
	if (resp && ((s3c_hsmmc_readl(HM_RSPREG0) & 0xff) == 0xaa)) {
		card_type = CARDTYPE_SD20; /* 2.0 SD, may not be SDHC */
		hcs = 0x40000000;
	}

	/* Well, either way let's say hello in SD card protocol */

	while (retries--) {

		udelay(100000);
		udelay(100000);
		udelay(100000);

		resp = issue_command(MMC_APP_CMD, 0x00000000, 0,
			MMC_RSP_R1);
		if (!resp)
			continue;
		resp = issue_command(SD_APP_OP_COND, hcs | 0x00300000, 0,
			MMC_RSP_R3);
		if (!resp)
			continue;

		if ((s3c_hsmmc_readl(HM_RSPREG0) >> 24) & (1 << 6)) { /* asserts block addressing */
			retries = -2;
			card_type = CARDTYPE_SDHC;
		}

		if ((s3c_hsmmc_readl(HM_RSPREG0) >> 24) & (1 << 7)) { /* not busy */
			retries = -2;
			if (card_type == CARDTYPE_NONE)
				card_type = CARDTYPE_SD;
			break;
		}
	}
	if (retries == -1) {
		puts("no response\n");
		return -2;
	}

	if (!issue_command(MMC_ALL_SEND_CID, 0, 0, MMC_RSP_R2)) {
		puts("CID broken\n");
		return -3;
	}

	r1[0] = s3c_hsmmc_readl(HM_RSPREG3);
	r1[1] = s3c_hsmmc_readl(HM_RSPREG2);
	r1[2] = s3c_hsmmc_readl(HM_RSPREG1);
	r1[3] = s3c_hsmmc_readl(HM_RSPREG0);

	for (n = 0; n < 16; n++)
		response[n] = p8[mangle[n]];

	switch (card_type) {
	case CARDTYPE_SD:
	case CARDTYPE_SD20:
	case CARDTYPE_SDHC:

		if (verbose)
			print_sd_cid(sd_cid);
		resp = issue_command(SD_SEND_RELATIVE_ADDR, MMC_DEFAULT_RCA,
				0, MMC_RSP_R6);
		rca = s3c_hsmmc_readl(HM_RSPREG0) >> 16;
		break;

	default:
		return 1;
	}

	/* grab the CSD */

	resp = issue_command(MMC_SEND_CSD, rca << 16, 0, MMC_RSP_R2);
	if (resp) {

		r1[0] = s3c_hsmmc_readl(HM_RSPREG3);
		r1[1] = s3c_hsmmc_readl(HM_RSPREG2);
		r1[2] = s3c_hsmmc_readl(HM_RSPREG1);
		r1[3] = s3c_hsmmc_readl(HM_RSPREG0);
		for (n = 0; n < 16; n++)
			response[n] = p8[mangle[n]];

		switch (card_type) {
		case CARDTYPE_SDHC:
			puts("    SDHC size: ");
			sd_sectors = (UNSTUFF_BITS(((u32 *)&response[0]), 48, 22)
								    + 1) << 10;
			break;
		default:
			puts("  MMC/SD size: ");
			sd_sectors = ((((unsigned long)1 << csd->c_size_mult1) *
					(unsigned long)(csd->c_size)) >> 9);
		}
		printdec(sd_sectors / 2048);
		puts(" MiB\n");
	} else
		puts("CSD grab broken\n");

	resp = issue_command(MMC_SELECT_CARD, rca<<16, 0, MMC_RSP_R1);
	if (!resp)
		return 1;

	/* Operating Clock setting */
	clock_config(2);	// Divisor 1 = Base clk /2      ,Divisor 2 = Base clk /4, Divisor 4 = Base clk /8 ...

	while (set_bus_width(width));
	while (!check_card_status());

	/* MMC_SET_BLOCKLEN */
	while (!issue_command(MMC_SET_BLOCKLEN, 512, 0, MMC_RSP_R1));

	s3c_hsmmc_writew(0xffff, HM_NORINTSTS);

	return sd_sectors;
}

unsigned long s3c6410_mmc_bread(int dev_num, unsigned long start_blk, unsigned long blknum,
								      void *dst)
{
	u32 blksize; //j, , Addr_temp = start_blk;
	u32 dma = 0, cmd, multi; //, TotalReadByte, read_blk_cnt = 0;

	rd_cnt_HSMMC = 0;
	HS_DMA_END = 0;
	BlockNum_HSMMC = 0;
	rd_cnt_HSMMC = 0;
	ReadBlockCnt_INT = 0;

//	printf("\nHS-MMC block Read test: %d, 0x%x 0x%x\n", test, start_blk, blknum);

	BlockNum_HSMMC = blknum;

	blksize = Card_OneBlockSize_ver1;

#if 0
	Rx_buffer_HSMMC = (u32 *) SDI_Rx_buffer_HSMMC;
	for (i = 0; i < (blksize * BlockNum_HSMMC) / 4; i++)
		*(Rx_buffer_HSMMC + i) = 0x0;
#endif
	while (!check_card_status());

	s3c_hsmmc_writew(s3c_hsmmc_readw(HM_NORINTSTSEN) & ~(DMA_STS_INT_EN | BLOCKGAP_EVENT_STS_INT_EN), HM_NORINTSTSEN);
	s3c_hsmmc_writew((HM_NORINTSIGEN & ~(0xffff)) | TRANSFERCOMPLETE_SIG_INT_EN, HM_NORINTSIGEN);

	SetSystemAddressReg((unsigned long)dst);	// AHB System Address For Write
	dma = 1;

	set_blksize_register(7, 512);	// Maximum DMA Buffer Size, Block Size
	set_blkcnt_register(BlockNum_HSMMC);	// Block Numbers to Write

	if (movi_hc)
		set_arg_register(start_blk);		// Card Start Block Address to Write
	else
		set_arg_register(start_blk * 512);	// Card Start Block Address to Write

	cmd = (blknum > 1) ? 18 : 17;
	multi = (blknum > 1);

	set_transfer_mode_register(multi, 1, multi, 1, dma);
	set_cmd_register(cmd, 1, MMC_RSP_R1);

	if (wait_for_cmd_done()) {
		puts("Command NOT Complete\n");
		return -1;
	} else
		ClearCommandCompleteStatus();


	check_dma_int();
	while (!HS_DMA_END);
//	puts("\nDMA Read End\n");

	HS_DMA_END = 0;
	BlockNum_HSMMC = 0;
	rd_cnt_HSMMC = 0;
	ReadBlockCnt_INT = 0;

	return 0;
}
