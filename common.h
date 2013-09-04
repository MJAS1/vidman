/*
 * This file contains definitions that are shared between video recording and playing software
 */

#ifndef COMMON_H_

// Camera configuration
#define VIDEO_HEIGHT     	480
#define VIDEO_WIDTH      	640

#define VIDEO_FILE_VERSION	1

#define MAGIC_VIDEO_STR		"ELEKTA_VIDEO_FILE"

#define COMMON_H_

//Trigger codes
enum TrigCode
{
    NULL_CODE,
    RTS,
    DTR
};

#endif /* COMMON_H_ */

