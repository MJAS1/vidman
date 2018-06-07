/*
 * config.h
 *
 * Original file:
 * Author: Andrey Zhdanov
 * Copyright (C) 2014 BioMag Laboratory, Helsinki University Central Hospital
 *
 * Modifications:
 * Author: Manu Sutela
 * Copyright (C) 2018 Aalto University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONFIG_H_
#include "common.h"

// Camera configuration
#define VIDEO_DEV_PATH   	"/dev/video0"
#define N_CAMERA_BUFFERS 	1

#define SHUTTER_ADDR        0xf0081c
#define SHUTTER_MIN_VAL		1
#define SHUTTER_MAX_VAL		0xfff
#define SHUTTER_OFFSET		0x82000000

#define GAIN_ADDR           0xf00820
#define GAIN_MIN_VAL		0
#define GAIN_MAX_VAL		0x2a8
#define GAIN_OFFSET         0x82000000

#define WHITEBALANCE_ADDR	0xf0080c
#define WHITEBALANCE_OFFSET	0x82000000
#define UV_MIN_VAL          0
#define UV_MAX_VAL          0x238
#define VR_MIN_VAL          0
#define VR_MAX_VAL          0x238
#define UV_REG_SHIFT        0x1000

// Buffer parameters
#define CIRC_BUF_MARG		0.2         // When less than this fraction of the buffer
                                        // is left free, overflow error is generated.
                                        // Used as a safety measure against race
                                        // condition between consumer and producer
                                        // threads when the circular buffer is close
                                        // to full.

#define MAX_CHUNK_SIZE		(CIRC_BUF_MARG/10)
										// Maximum chunk size as a fraction of the
										// circular buffer size. Should be at least
										// several times smaller than CIRC_BUF_MARG
										// for the buffer to work properly.

// Buffer sizes
#define CIRC_VIDEO_BUFF_SZ	100000000	// in bytes
#define CIRC_AUDIO_BUFF_SZ	100000000	// in bytes

// Used for storing application settings
#define ORG_NAME            "BECS"
#define APP_NAME            "VidMan"

#define CONFIG_H_


#endif /* CONFIG_H_ */
