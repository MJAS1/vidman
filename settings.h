/*
 * settings.h
 *
 *  Created on: Mar 7, 2011
 *      Author: andrey
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

//! Application-wide settings preserved across multiple invocations.
/*!
 * This class contains application-wide settings read from disc. To read the
 * settings simply create the instance of this class and read the values from
 * the corresponding public variables of the class. The settings are supposed
 * to be read only; they can be changed by manually editing the text file
 * between program invocations but they should stay constant for the whole
 * lifetime of a single program instance. The class should be completely
 * thread-safe (CURRENTLY IT IS NOT).
 */
class Settings {
	// TODO: make member variables immutable as much as possible
	// TODO: make the class thread-safe
	// TODO: implement singleton pattern?
public:
	Settings();

	// video
	int				jpgQuality;
	bool			color;

	// misc
	char			storagePath[500];
};

#endif /* SETTINGS_H_ */
