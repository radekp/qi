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

/* controls symbol sequencing on i2c */

enum i2c_bitbang_control {
	IBCONTROL_DO_START = -1,
	IBCONTROL_DO_STOP = -2,
	IBCONTROL_DO_READ = -3,
	IBCONTROL_COMPLETE = -4
};

/* control intra-bit and byte states */

enum i2c_bitbang_states {
	IBS_INIT,

	IBS_START1,
	IBS_START2,

	IBS_ADS_TX_S,
	IBS_ADS_TX_H,
	IBS_ADS_TX_L,
	IBS_ADS_TX_ACK_H,
	IBS_ADS_TX_ACK_L,

	IBS_DATA_RX_S,
	IBS_DATA_RX_H,
	IBS_DATA_RX_L,

	IBS_DATA_RX_ACK_H,
	IBS_DATA_RX_ACK_L,

	IBS_STOP1,
	IBS_STOP2,
	IBS_STOP3,
	IBS_STOP4
};

/* context for bitbang GPIO pins and transaction */

struct i2c_bitbang {

	enum i2c_bitbang_states state;
	int count;
	unsigned int data[8]; /* read result found here */
	int index;
	int index_read;

	char (*read_sda)(void);
	/* data = 0 = op low, 1 == inp */
	void (*set)(char clock, char data);
	/* delay > 1 half-bit time, used by i2c_complete_synchronously() */
	void (*spin)(void);
	void (*close)(void);
};

/* synchronous read and write functions spin until completed or failed
 * i2c_read_sync returns -1 for fail or byte result from device
 */

extern int i2c_read_sync(struct i2c_bitbang * bb, unsigned char ads7,
							     unsigned char reg);
extern void i2c_write_sync(struct i2c_bitbang * bb, unsigned char ads7,
					 unsigned char reg, unsigned char data);


/*
 * set up an asynchronous read or write transaction
 */
extern void i2c_read(struct i2c_bitbang * bb, unsigned char ads7,
							     unsigned char reg);
extern void i2c_write(struct i2c_bitbang * bb, unsigned char ads7,
					 unsigned char reg, unsigned char data);

/*
 * after setting up a read or write transaction above, you loop calling this
 * with >= 1.25us (400kHz) or >= 5us (100kHz) delay between calls.  You don't
 * have to spin but can do something useful if you know it will take more than
 * an i2c bit-time, hiding the time for the i2c transaction completely.
 */
extern int i2c_next_state(struct i2c_bitbang * bb); /* return !=0 = completed */
