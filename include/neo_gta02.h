/*
 * (C) Copyright 2007 OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 *
 * Configuation settings for the FIC Neo1973 GTA02 Linux GSM phone
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define TEXT_BASE 		0x00000000 /* xiangfu add*/

#define CFG_ENV_SIZE		0x40000	/* 128k Total Size of Environment Sector */
/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 400*1024)
					/* >> CFG_VIDEO_LOGO_MAX_SIZE */
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

#endif	/* __CONFIG_H */
