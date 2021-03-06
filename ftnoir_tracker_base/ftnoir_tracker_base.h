/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2010	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage																		*
*																				*
* This program is free software; you can redistribute it and/or modify it		*
* under the terms of the GNU General Public License as published by the			*
* Free Software Foundation; either version 3 of the License, or (at your		*
* option) any later version.													*
*																				*
* This program is distributed in the hope that it will be useful, but			*
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY	*
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for	*
* more details.																	*
*																				*
* You should have received a copy of the GNU General Public License along		*
* with this program; if not, see <http://www.gnu.org/licenses/>.				*
* This class implements a tracker-base											*
*********************************************************************************/
/*
	Modifications (last one on top):
	    20122109 - C14: Replaced Release with virtual destructor
		20120009 - WVR: Removed AutoClosePtr (seemed like it didn't work OK)
		20110415 - WVR: Added overloaded operator - and -=
*/
#ifndef FTNOIR_TRACKER_BASE_H
#define FTNOIR_TRACKER_BASE_H

#include "ftnoir_tracker_base_global.h"
#include "ftnoir_tracker_types.h"
#include <QtGui/QWidget>
#include <QtGui/QFrame>
#include <QWaitCondition>
#include <QMutex>
#include <QFrame>

////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
#   define EXTERN_C     extern "C"
#else
#   define EXTERN_C
#endif // __cplusplus

////////////////////////////////////////////////////////////////////////////////
// COM-Like abstract interface.
// This interface doesn't require __declspec(dllexport/dllimport) specifier.
// Method calls are dispatched via virtual table.
// Any C++ compiler can use it.
// Instances are obtained via factory function.
struct ITracker
{
    virtual ~ITracker() {}
    virtual void StartTracker( QFrame* frame ) = 0;
    virtual bool GiveHeadPoseData(THeadPoseData *data) = 0;

    virtual void WaitForExit() = 0;
};

typedef ITracker* ITrackerPtr;

////////////////////////////////////////////////////////////////////////////////
// COM-Like abstract interface.
// This interface doesn't require __declspec(dllexport/dllimport) specifier.
// Method calls are dispatched via virtual table.
// Any C++ compiler can use it.
// Instances are obtained via factory function.
struct ITrackerDialog
{
    virtual ~ITrackerDialog() {}
	virtual void Initialize(QWidget *parent) = 0;
	virtual void registerTracker(ITracker *tracker) = 0;
	virtual void unRegisterTracker() = 0;
};


////////////////////////////////////////////////////////////////////////////////
// COM-Like abstract interface.
// This interface doesn't require __declspec(dllexport/dllimport) specifier.
// Method calls are dispatched via virtual table.
// Any C++ compiler can use it.
// Instances are obtained via factory function.
struct ITrackerDll
{
	virtual ~ITrackerDll() {}
	virtual void Initialize() = 0;

	virtual void getFullName(QString *strToBeFilled) = 0;
	virtual void getShortName(QString *strToBeFilled) = 0;
	virtual void getDescription(QString *strToBeFilled) = 0;
	virtual void getIcon(QIcon *icon) = 0;
};


#endif // FTNOIR_TRACKER_BASE_H
