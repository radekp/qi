/*
 * (C) Copyright 2007 OpenMoko, Inc.
 * Author: xiangfu liu <xiangfu@openmoko.org>
 *
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

/* NOTE this code is not good, some define is 
 * is already show in other files 
 */

# define CLKDIVN_val    (7)    
# define CAMDIVN_val    (0)
# define CONFIG_SYS_CLK_FREQ  (12000000)/* the GTA02 has this input clock */

# define MPLL    ((142 << 12) + (7 << 4) + 1)
# define BAUDRATE    	(115200)

typedef unsigned long   ulong;

static ulong get_PLLCLK(int pllreg)
{
  ulong r, m, p, s;

  if (pllreg == MPLL){
    r = MPLL;
  }

  m = ((r & 0xFF000) >> 12) + 8;
  p = ((r & 0x003F0) >> 4) + 2;
  s = r & 0x3;
  /* To avoid integer overflow, changed the calc order */
  if (pllreg == MPLL)
    return ( 2 * m * (CONFIG_SYS_CLK_FREQ / (p << s )) );
  else
    return ( m * (CONFIG_SYS_CLK_FREQ / (p << s )) );
}

ulong get_FCLK(void)
{
  return(get_PLLCLK(MPLL));
}


ulong get_HCLK(void)
{
  switch (CLKDIVN_val & 0x6) {
  case 0x0:
    return get_FCLK();
  case 0x2:
    return get_FCLK()/2;
  case 0x4:
    return (CAMDIVN_val & 0x200) ? get_FCLK()/8 : get_FCLK()/4;
  case 0x6:
    return (CAMDIVN_val & 0x100) ? get_FCLK()/6 : get_FCLK()/3;
  }
  return 0;
}

ulong get_PCLK(void)
{
  return((CLKDIVN_val & 0x1) ? get_HCLK()/2 : get_HCLK());
}

