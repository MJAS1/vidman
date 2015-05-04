/*
 * stoppablethread.cpp
 *
 *  Created on: Jan 14, 2010
 *      Author: meg2meg
 */

#include <iostream>
#include "stoppablethread.h"

using namespace std;

int StoppableThread::nextId = 0;

StoppableThread::StoppableThread(QObject *parent) : QThread(parent), shouldStop(false)
{
	id = nextId++;

	clog << "StoppableThread no. " << id << " is created" << endl;
}


StoppableThread::~StoppableThread()
{
	clog << "StoppableThread no. " << id << " is destroyed" << endl;
}


void StoppableThread::run()
{
	clog << "Running StoppableThread no. " << id << " ..." << endl;
	stoppableRun();
}


void StoppableThread::stop()
{
	clog << "Stopping StoppableThread no. " << id << " ...";

	shouldStop = true;
	this->wait();

	clog << " done" << endl;
}

