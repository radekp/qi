#include <qi.h>
#include <neo_smdk6410.h>
#include <serial-s3c64xx.h>

static const struct board_variant board_variants[] = {
	[0] = {
		.name = "SMDK",
		.machine_revision = 0,
	},
};

/**
 * returns PCB revision information in b0, d8, d9
 * SMDK6410 EVB returns 0x000
 * SMDK6410 returns 0x001
 */

int smdk6410_get_pcb_revision(void)
{
	return 0;
}

const struct board_variant const * get_board_variant_smdk6410(void)
{
	return &board_variants[smdk6410_get_pcb_revision()];
}

