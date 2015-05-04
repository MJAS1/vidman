/*
 * filewriter.cpp
 *
 *  Created on: Jan 10, 2011
 *      Author: meg2meg
 */
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <cerrno>
#include <QDateTime>

#include "filewriter.h"

using namespace std;

FileWriter::FileWriter(CycDataBuffer* cycBuf, const char* path, const char* suffix, const char* ext, QObject* parent) :
    StoppableThread(parent), cycBuf(cycBuf), path(path), suffix(suffix), ext(ext)
{
}


FileWriter::~FileWriter()
{
}


void FileWriter::stoppableRun()
{
	unsigned char*	databuf;
	bool			prevIsRec=false;
	ofstream		outData;
    ChunkAttrib		chunkAttrib;
    uint32_t		chunkSz;

    unsigned char*	header;
    int				headerLen;

	while (true)
	{
		databuf = cycBuf->getChunk(&chunkAttrib);
		if (chunkAttrib.isRec)
		{
			if (!prevIsRec)
			{
                QString name(QDateTime::currentDateTime().toString("%1/yyyy-MM-dd--hh:mm:ss%2.%3"));
                name = name.arg(path).arg(suffix).arg(ext);

                outData.open(name.toStdString().c_str(), ios_base::out | ios_base::binary | ios_base::trunc);
				if(outData.fail())
				{
                    cerr << "Error opening the file " << name.toStdString()  << ". " << strerror(errno) << endl;
					abort();
				}
				header = getHeader(&headerLen);
				outData.write((const char*)header, headerLen);
			}

			chunkSz = chunkAttrib.chunkSize;
			outData.write((const char*)(&(chunkAttrib.timestamp)), sizeof(uint64_t));
			outData.write((const char*)(&chunkSz), sizeof(uint32_t));
			outData.write((const char*)databuf, chunkAttrib.chunkSize);
		}
		else
		{
			if (prevIsRec)
			{
				outData.close();
			}
		}

		prevIsRec = chunkAttrib.isRec;

        delete[]chunkAttrib.log;

		if(shouldStop)
		{
			if(prevIsRec)
			{
				outData.close();
			}
			return;
		}


	}
}
