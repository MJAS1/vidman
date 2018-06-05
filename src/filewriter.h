#ifndef FILEWRITER_H_
#define FILEWRITER_H_

#include <QString>

#include "stoppablethread.h"

class CycDataBuffer;

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
 */

class FileWriter : public StoppableThread
{

Q_OBJECT

signals:
    void error(const QString&) const;

protected:
    FileWriter(CycDataBuffer* cycBuf, const char* path, const char* suffix,
               const char* ext, QObject* parent = nullptr);
	virtual ~FileWriter();

	virtual void stoppableRun();

    // Return the header to be written at the beginning of the file.
	virtual unsigned char* getHeader(int* _len) = 0;

private:
    CycDataBuffer*	cycBuf_;
    QString			path_, suffix_, ext_;

    int				streamId_;
};

#endif /* FILEWRITER_H_ */
