/*
 * settings.cpp
 *
 *  Created on: Mar 7, 2011
 *      Author: andrey
 */
#include <stdio.h>
#include <QSettings>
#include <iostream>

#include "settings.h"
#include "config.h"

Settings::Settings()
{
	QSettings settings(ORG_NAME, APP_NAME);


	//---------------------------------------------------------------------
	// Video settings
	//

	// JPEG quality
	if(!settings.contains("video/jpeg_quality"))
	{
		settings.setValue("video/jpeg_quality", 80);
		jpgQuality = 80;
	}
	else
	{
		jpgQuality = settings.value("video/jpeg_quality").toInt();
	}

	// Use color mode
	if(!settings.contains("video/color"))
	{
        settings.setValue("video/color", true);
		color = true;
	}
	else
	{
        color = settings.value("video/color").toBool();
	}

	//---------------------------------------------------------------------
	// Misc settings
	//

	// Data storage folder
	if(!settings.contains("misc/data_storage_path"))
	{
		settings.setValue("misc/data_storage_path", "/videodat");
		sprintf(storagePath, "/videodat");
	}
	else
	{
		sprintf(storagePath, settings.value("misc/data_storage_path").toString().toLocal8Bit().data());
	}
}

