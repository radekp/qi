/*
 * (C) Copyright 2007 OpenMoko, Inc.
 * Author: Andy Green <andy@openmoko.com>
 *
 * Generic i2c bitbang state machine
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#include <qi.h>
#include <i2c-bitbang.h>

void i2c_read(struct i2c_bitbang * bb, unsigned char ads7, unsigned char reg)
{
	bb->data[0] = (ads7 << 1); /* write the register address */
	bb->data[1] = reg;
	bb->data[2] = IBCONTROL_DO_START;
	bb->data[3] = (ads7 << 1) | 1; /* then issue read cycle to device */
	bb->data[4] = IBCONTROL_DO_READ;
	bb->data[5] = IBCONTROL_DO_STOP;
	bb->data[6] = IBCONTROL_COMPLETE;
	bb->state = IBS_INIT;
}

void i2c_write(struct i2c_bitbang * bb, unsigned char ads7, unsigned char reg,
								unsigned char b)
{
	bb->data[0] = (ads7 << 1);
	bb->data[1] = reg;
	bb->data[2] = b;
	bb->data[3] = IBCONTROL_DO_STOP;
	bb->data[4] = IBCONTROL_COMPLETE;
	bb->state = IBS_INIT;
}

int i2c_next_state(struct i2c_bitbang * bb)
{
	switch (bb->state) {
	case IBS_INIT:
		bb->index = 0;
		bb->index_read = 0;
		(bb->set)(1, 1);
		bb->state = IBS_START1;
		break;

	case IBS_START1:
		(bb->set)(1, 0);
		bb->state = IBS_START2;
		break;

	case IBS_START2:
		(bb->set)(0, 0); /* start */
		bb->count = 8;
		bb->state = IBS_ADS_TX_S;
		break;

	/* transmit address or data */
	case IBS_ADS_TX_S:
		(bb->set)(0, !!(bb->data[bb->index] & 0x80));
		bb->state = IBS_ADS_TX_H;
		break;
	case IBS_ADS_TX_H:
		(bb->set)(1, !!(bb->data[bb->index] & 0x80));
		bb->state = IBS_ADS_TX_L;
		break;
	case IBS_ADS_TX_L:
		(bb->set)(0, !!(bb->data[bb->index] & 0x80));
		bb->data[bb->index] <<= 1;
		bb->count--;
		if (bb->count) {
			bb->state = IBS_ADS_TX_S;
			break;
		}

		(bb->set)(0, 1);
		bb->state = IBS_ADS_TX_ACK_H;
		break;

	case IBS_ADS_TX_ACK_H:
		/* we finished... we expect an ack now */
		if ((bb->read_sda)())
			return -1;

		(bb->set)(1, 1);
		bb->state = IBS_ADS_TX_ACK_L;
		break;

	case IBS_ADS_TX_ACK_L:
		(bb->set)(0, 1);

		bb->count = 8;
		bb->index++;
		switch (bb->data[bb->index]) {
		case IBCONTROL_DO_START:
			bb->state = IBS_START1;
			bb->index++;
			break;
		case IBCONTROL_DO_STOP:
			bb->state = IBS_STOP1;
			bb->index++;
			break;
		case IBCONTROL_DO_READ:
			bb->data[bb->index_read] = 0;
			bb->state = IBS_DATA_RX_S;
			break;
		case IBCONTROL_COMPLETE:
			return 1;
		default:
			bb->state = IBS_ADS_TX_S; /* write it out */
			break;
		}
		break;


	/* receive data */
	case IBS_DATA_RX_S:
		(bb->set)(0, 1);
		bb->state = IBS_DATA_RX_H;
		break;

	case IBS_DATA_RX_H:
		(bb->set)(1, 1);
		bb->state = IBS_DATA_RX_L;
		break;

	case IBS_DATA_RX_L:
		bb->data[bb->index_read] <<= 1;
		bb->data[bb->index_read] |= !!(bb->read_sda)();
		bb->count--;
		if (bb->count) {
			(bb->set)(0, 1);
			bb->state = IBS_DATA_RX_S;
			break;
		}

		/* slave has released SDA now, bang down ACK */
		if (bb->data[bb->index + 1] != IBCONTROL_DO_READ)
			(bb->set)(0, 1);
		else
			(bb->set)(0, 0);
		bb->state = IBS_DATA_RX_ACK_H;
		break;

	case IBS_DATA_RX_ACK_H:
		if (bb->data[bb->index + 1] != IBCONTROL_DO_READ)
			(bb->set)(1, 1); /* NAK */
		else
			(bb->set)(1, 0); /* ACK */
		bb->state = IBS_DATA_RX_ACK_L;
		break;

	case IBS_DATA_RX_ACK_L:
		if (bb->data[bb->index + 1] != IBCONTROL_DO_READ)
			(bb->set)(0, 1); /* NAK */
		else
			(bb->set)(0, 0); /* ACK */
		bb->index_read++;
		bb->index++;
		switch (bb->data[bb->index]) {
		case IBCONTROL_DO_START:
			bb->state = IBS_START1;
			bb->index++;
			break;
		case IBCONTROL_DO_STOP:
			bb->state = IBS_STOP1;
			bb->index++;
			break;
		case IBCONTROL_DO_READ:
			bb->state = IBS_DATA_RX_S;
			bb->data[bb->index_read] = 0;
			break;
		case IBCONTROL_COMPLETE:
			return 1;
		default:
			bb->state = IBS_ADS_TX_S; /* write it out */
			break;
		}
		break;

		break;


	case IBS_STOP1:
		(bb->set)(0, 0);
		bb->state = IBS_STOP2;
		break;

	case IBS_STOP2:
		(bb->set)(1, 0);
		bb->state = IBS_STOP3;
		break;

	case IBS_STOP3:
		(bb->set)(1, 1);
		bb->state = IBS_STOP4;
		break;

	case IBS_STOP4:
		(bb->set)(1, 1);
		return 1; /* done */
	}

	return 0; /* keep going */
}

static int i2c_complete_synchronously(struct i2c_bitbang * bb)
{
	int ret = 0;

	while (!ret) {
		ret = i2c_next_state(bb);
		(bb->spin)();
	}

	if (ret < 0) {
		puts("i2c transaction failed ");
		printdec(ret);
		puts("\n");
	}
	return ret;
}

int i2c_read_sync(struct i2c_bitbang * bb, unsigned char ads7,
							      unsigned char reg)
{
	i2c_read(bb, ads7, reg);
	if (i2c_complete_synchronously(bb) < 0)
		return -1;

	return bb->data[0];
}

void i2c_write_sync(struct i2c_bitbang * bb, unsigned char ads7,
					     unsigned char reg, unsigned char b)
{
	i2c_write(bb, ads7, reg, b);
	i2c_complete_synchronously(bb);
}
