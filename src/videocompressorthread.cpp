/*
 * videocompressorthread.cpp
 *
 *  Created on: Jan 14, 2010
 *      Author: meg2meg
 */
#include <cstdlib>
#include <stdio.h>
#include <jpeglib.h>
#include <QDebug>
#include <QDateTime>

#include "config.h"
#include "videocompressorthread.h"

VideoCompressorThread::VideoCompressorThread(CycDataBuffer* inpBuf,
                                             CycDataBuffer* outBuf,
                                             int jpgQuality, QObject* parent) :
    StoppableThread(parent), inpBuf(inpBuf), outBuf(outBuf),
    jpgQuality(jpgQuality)
{
}

VideoCompressorThread::~VideoCompressorThread()
{
}

void VideoCompressorThread::stoppableRun()
{
    while(!shouldStop)
    {
        // JPEG-related stuff
        struct jpeg_compress_struct	cinfo;
        struct jpeg_error_mgr		jerr;
        JSAMPROW 					row_pointer;
        unsigned char*				jpgBuf=NULL;
        unsigned long				jpgBufLen=0;

        unsigned char*				data;
        ChunkAttrib					chunkAttrib;

        // Get raw image from the input buffer
        data = inpBuf->getChunk(&chunkAttrib);

        // Initialize JPEG
        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);
        jpeg_mem_dest(&cinfo, &jpgBuf, &jpgBufLen);

        // Set the parameters of the output file
        cinfo.image_width = VIDEO_WIDTH;
        cinfo.image_height = VIDEO_HEIGHT;
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_RGB;

        // Use default compression parameters
        jpeg_set_defaults(&cinfo);
        jpeg_set_quality(&cinfo, jpgQuality, TRUE);

        // Do the compression
        jpeg_start_compress(&cinfo, TRUE);

        // write one row at a time
        while(cinfo.next_scanline < cinfo.image_height)
        {
            row_pointer = (data + (cinfo.next_scanline * cinfo.image_width * 3));
            jpeg_write_scanlines(&cinfo, &row_pointer, 1);
        }

        // clean up after we're done compressing
        jpeg_finish_compress(&cinfo);


        // Insert compressed image into the output buffer
        chunkAttrib.chunkSize = jpgBufLen;
        outBuf->insertChunk(jpgBuf, chunkAttrib);

        // The output buffer needs to be explicitly freed by the libjpeg client
        free(jpgBuf);
        jpeg_destroy_compress(&cinfo);
    }
}
