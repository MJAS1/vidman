/*
 * cycframebuffer.cpp
 *
 *  Created on: Jul 2, 2010
 *      Author: andrey
 */

#include <stdlib.h>
#include <iostream>
#include "config.h"
#include "cycdatabuffer.h"

using namespace std;


CycDataBuffer::CycDataBuffer(int bufSize) :
    isRec(false), insertPtr(0), getPtr(0), bufSize(bufSize), buffSemaphore(new QSemaphore())
{
    // Allocate the buffer. Reserve some extra space necessary to handle
    // chunks of varying size.
    dataBuf = new unsigned char[bufSize + 2 * (int(bufSize*MAX_CHUNK_SIZE) + sizeof(ChunkAttrib))];
    if (!dataBuf)
    {
    	cerr << "Cannot allocate memory for circular buffer" << endl;
    	abort();
    }
}


CycDataBuffer::~CycDataBuffer()
{
    delete []dataBuf;
    delete buffSemaphore;
}


void CycDataBuffer::insertChunk(unsigned char* _data, ChunkAttrib &_attrib)
{

	// Check for buffer overflow. CIRC_BUF_MARG is the safety margin against
	// race condition between consumer and producer threads when the buffer
	// is close to full.
	if (buffSemaphore->available() >=  bufSize * (1-CIRC_BUF_MARG))
	{
		cerr << "Circular buffer overflow!" << endl;
		abort();
	}

	// Make sure that the safety margin is at least several (four) times the
	// chunk size. This is necessary to prevent the race condition between
	// consumer and producer threads when the buffer is close to full.
    if(_attrib.chunkSize+sizeof(ChunkAttrib)+_attrib.logSize > bufSize*MAX_CHUNK_SIZE)
	{
		cerr << "The chunk size is too large!" << endl;
		abort();
	}

	// insert the data into the circular buffer
	_attrib.isRec = isRec;

	memcpy(dataBuf + insertPtr, (unsigned char*)(&_attrib), sizeof(ChunkAttrib));
	insertPtr += sizeof(ChunkAttrib);
	buffSemaphore->release(sizeof(ChunkAttrib));

    memcpy(dataBuf + insertPtr, _attrib.log, _attrib.logSize+1);
    insertPtr += _attrib.logSize+1;
    buffSemaphore->release(_attrib.logSize+1);

	memcpy(dataBuf + insertPtr, _data, _attrib.chunkSize);
	buffSemaphore->release(_attrib.chunkSize);

    emit chunkReady(dataBuf + insertPtr, _attrib.logSize);
	insertPtr += _attrib.chunkSize;
	if(insertPtr >= bufSize)
	{
		insertPtr = 0;
	}
}


unsigned char* CycDataBuffer::getChunk(ChunkAttrib* _attrib)
{
	unsigned char* res;

	buffSemaphore->acquire(sizeof(ChunkAttrib));
	memcpy((unsigned char*)_attrib, dataBuf + getPtr, sizeof(ChunkAttrib));
	getPtr += sizeof(ChunkAttrib);

    //memory has to be freed in caller function
    _attrib->log = new char[_attrib->logSize+1];


    buffSemaphore->acquire(_attrib->logSize+1);
    memcpy(_attrib->log, dataBuf+getPtr, _attrib->logSize+1);
    getPtr += _attrib->logSize+1;

	buffSemaphore->acquire(_attrib->chunkSize);
	res = dataBuf + getPtr;

	getPtr += _attrib->chunkSize;
	if(getPtr >= bufSize)
	{
		getPtr = 0;
	}

	return(res);
}


void CycDataBuffer::setIsRec(bool _isRec)
{
	isRec = _isRec;
}
