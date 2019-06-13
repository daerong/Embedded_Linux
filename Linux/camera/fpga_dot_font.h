/* This file is FPGA-DOT MATRIX font file.
   FILE : fpga_dot_font.h
   AUTH : Hong, Sung-Hyun
          Huins, Inc. */

#ifndef __FPGA_NUMBER__
#define __FPGA_NUMBER__

unsigned char fpga_number[10][10] = {
	{0x00,0x00,0x1C,0x1C,0x7F,0x77,0x7F,0x1C,0x1C,0x00}, // X
	{0x30,0x48,0x48,0x48,0x48,0x48,0x48,0x48,0x48,0x33}, // X
	{0x30,0x48,0x48,0x48,0x48,0x48,0x48,0x4B,0x48,0x33}, // X
	{0x30,0x48,0x48,0x48,0x48,0x4B,0x48,0x4B,0x48,0x33}, // X
	{0x30,0x48,0x48,0x4B,0x48,0x4B,0x48,0x4B,0x48,0x33}, // X
	{0x30,0x4B,0x48,0x4B,0x48,0x4B,0x48,0x4B,0x48,0x33}, // X
	{0x75,0x45,0x75,0x45,0x47,0x00,0x44,0x44,0x44,0x01} // X
};

unsigned char fpga_set_full[10] = {
	// memset(array,0x7e,sizeof(array));
	0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f
};

unsigned char fpga_set_blank[10] = {
	// memset(array,0x00,sizeof(array));
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

#endif
