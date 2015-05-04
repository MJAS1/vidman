/*
 * cycframebuffer.h
 *
 *  Created on: Jul 2, 2010
 *      Author: andrey
 */

#ifndef CYCDATABUFFER_H_
#define CYCDATABUFFER_H_

#include <stdint.h>
#include <QObject>
#include <QSemaphore>
#include <QString>
#include <common.h>

//! Attributes associated with each data chunk.
typedef struct
{
    int         logSize;
	int			chunkSize;
	uint64_t	timestamp;
	bool		isRec;
    int         trigCode;
    char*       log;
} ChunkAttrib;


//! Cyclic buffer capable of holding data chunks of variable size.
/*!
 * Cyclic buffer for synchronizing producer/consumer threads. Supports single
 * producer, single primary consumer and multiple secondary consumers. Producer
 * and primary consumer access the buffer through insertChunk() and getChunk()
 * respectively. No additional synchronization between the producer and the
 * primary consumer is needed - the buffer takes care of it.
 *
 * Secondary consumers are only notified of every new chunk inserted trough
 * chunkReady() signal. The buffer provides no hard guaranty that the data will
 * not be overwritten before secondary consumers access it, but if the buffer
 * is large enough in comparison to the chunk insertion rate, the secondary
 * consumers are fast enough and the are general system load is low enough,
 * this should not typically happen.
 *
 * Neither primary nor secondary consumers should modify the content of the
 * data chunks.
 *
 * The class assumes that in general the primary consumer is considerably
 * faster than the producer, so that buffer is nearly empty most of the time.
 */
class CycDataBuffer : public QObject
{
	Q_OBJECT

public:
	/*!
	 * Buffer size is in bytes. Due to implementation details (semaphore counts
	 * bytes rather than chunks) buffer size is limited by the size of "int" on
	 * your platform.
	 */
    CycDataBuffer(int _bufSize, QObject* parent = 0);
	virtual ~CycDataBuffer();
    void insertChunk(unsigned char* _data, ChunkAttrib &_attrib);

	/*!
	 * Acquire a chunk and return a pointer to it. The chunk is implicitly
	 * released next time getChunk is called.
	 */
	unsigned char* getChunk(ChunkAttrib* _attrib);
	void setIsRec(bool _isRec);

signals:
	/*!
	 * This signal is raised when a new chunk of data has been copied to the
	 * buffer. _data points to the chunk's data. The corresponding ChunkAttrib
	 * structure is placed immediately before _data.
	 */
    void chunkReady(unsigned char* _data, int logSize);

private:
	volatile bool	isRec;

    unsigned char*	dataBuf;

	int				insertPtr;
	int				getPtr;
	int				bufSize;

    QSemaphore*		buffSemaphore;	// counts number of bytes available for reading
};

#endif /* CYCDATABUFFER_H_ */
