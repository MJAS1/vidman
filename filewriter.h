/*
 * filewriter.h
 *
 *  Created on: Jan 10, 2011
 *      Author: meg2meg
 */

#ifndef FILEWRITER_H_
#define FILEWRITER_H_

#include <QString>
#include "stoppablethread.h"
#include "cycdatabuffer.h"

//! Base class for audio/video stream writers
/*!
 * This base class implements functionality that is common to audio and video
 * stream writers - writing from a cyclic buffer to a file, creating/closing
 * files as necessary. Derived classes should implement getHeader() that should
 * return specific file header appropriate for the given file. Typically the
 * header would contain some string identifying file type and parameters (e.g.
 * sampling rate, etc.)
 *
 * When stopping the thread always call the stop() method while chunks are
 * still being continuously inserted into the CycDataBuffer associated with
 * the object - otherwise stop() will hang forever.
 *
 * Derived classes should typically call init() inside the constructor and
 * cleanup() inside the destructor.
 */
class FileWriter : public StoppableThread
{
protected:
    FileWriter(CycDataBuffer* cycBuf, const char* path, const char* suffix, const char* ext);
	virtual ~FileWriter();
	virtual void stoppableRun();

	//! Return the header to be written at the beginning of the file.
	virtual unsigned char* getHeader(int* _len) = 0;

private:
	CycDataBuffer*	cycBuf;
    QString			path, suffix, ext;

	int				streamId;
};

#endif /* FILEWRITER_H_ */
