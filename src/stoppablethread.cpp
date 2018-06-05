/*
 * stoppablethread.cpp
 *
 *  Created on: Jan 14, 2010
 *      Author: meg2meg
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

