/*
 * filewriter.cpp
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

#include <iostream>
#include <fstream>
#include <cerrno>
#include <QDateTime>
#include <QDebug>

#include "filewriter.h"
#include "cycdatabuffer.h"

using namespace std;

FileWriter::FileWriter(CycDataBuffer* cycBuf, const char* path,
                       const char* suffix, const char* ext, QObject* parent) :
    StoppableThread(parent), cycBuf_(cycBuf), path_(path), suffix_(suffix),
    ext_(ext)
{
}

FileWriter::~FileWriter()
{
}

void FileWriter::stoppableRun()
{
    unsigned char*  databuf;
    bool            prevIsRec=false;
    ofstream        outData;
    ChunkAttrib     chunkAttrib;

    unsigned char*  header;
    int             headerLen;

    while (true) {
        databuf = cycBuf_->getChunk(&chunkAttrib);
        if (chunkAttrib.isRec) {
            if (!prevIsRec) {
                QString name(QDateTime::currentDateTime()
                             .toString("%1/yyyy-MM-dd--hh:mm:ss%2.%3"));
                name = name.arg(path_).arg(suffix_).arg(ext_);

                outData.open(name.toStdString().c_str(),
                             ios_base::out | ios_base::binary | ios_base::trunc);
                if(outData.fail()) {
                    cerr << "Error opening the file " << name.toStdString()
                         << ". " << strerror(errno) << endl;
                    emit error(QString("Error opening the file %1. %2")
                               .arg(name).arg(strerror(errno)));
                    continue;
                }
                header = getHeader(&headerLen);
                outData.write((const char*)header, headerLen);
            }

            outData.write((const char*)(&(chunkAttrib.timestamp)),
                          sizeof(uint64_t));
            outData.write((const char*)(&(chunkAttrib.trigCode)),
                          sizeof(uint8_t));
            outData.write((const char*)(&chunkAttrib.chunkSize),
                          sizeof(uint32_t));
            outData.write((const char*)databuf, chunkAttrib.chunkSize);
        }
        else {
            if (prevIsRec) {
                outData.close();
            }
        }

        prevIsRec = chunkAttrib.isRec;

        if(shouldStop_) {
            if(prevIsRec) {
                outData.close();
            }
            return;
        }
    }
}
