/*
 * stoppablethread.cpp
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
#include "stoppablethread.h"

using namespace std;

int StoppableThread::nextId_ = 0;

StoppableThread::StoppableThread(QObject *parent) : QThread(parent),
    shouldStop_(false)
{
    id_ = nextId_++;

    clog << "StoppableThread no. " << id_ << " is created" << endl;
}


StoppableThread::~StoppableThread()
{
    clog << "StoppableThread no. " << id_ << " is destroyed" << endl;
}


void StoppableThread::run()
{
    clog << "Running StoppableThread no. " << id_ << " ..." << endl;
    stoppableRun();
}


void StoppableThread::stop()
{
    clog << "Stopping StoppableThread no. " << id_ << " ...";

    shouldStop_ = true;
    this->wait();

    clog << " done" << endl;
}

