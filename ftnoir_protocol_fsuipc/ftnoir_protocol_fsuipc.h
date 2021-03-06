/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2010-2011	Wim Vriend (Developing)								*
*							Ron Hendriks (Researching and Testing)				*
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
*																				*
* FSUIPCServer		FSUIPCServer is the Class, that communicates headpose-data	*
*					to games, using the FSUIPC.dll.			         			*
********************************************************************************/
#pragma once
#ifndef INCLUDED_FSUIPCSERVER_H
#define INCLUDED_FSUIPCSERVER_H

#include "Windows.h"
#include <stdlib.h>
#include "FSUIPC_User.h"

#include "..\ftnoir_protocol_base\ftnoir_protocol_base.h"
#include "ui_FTNoIR_FSUIPCcontrols.h"
#include <QMessageBox>
#include <QSettings>
#include <QLibrary>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QFileDialog>

static const char* FSUIPC_FILENAME = "C:\\Program Files\\Microsoft Games\\Flight Simulator 9\\Modules\\FSUIPC.dll";

//
// Define the structures necessary for the FSUIPC_Write calls
//
#pragma pack(push,1)		// All fields in structure must be byte aligned.
typedef struct
{
 int Control;				// Control identifier
 int Value;					// Value of DOF
} TFSState;
#pragma pack(pop)

class FTNoIR_Protocol : public IProtocol
{
public:
	FTNoIR_Protocol();
	~FTNoIR_Protocol();

	void Release();
    void Initialize();

	bool checkServerInstallationOK( HANDLE handle );
	void sendHeadposeToGame( THeadPoseData *headpose, THeadPoseData *rawheadpose );
	void getNameFromGame( char *dest );					// Take care dest can handle up to 100 chars...

private:
	// Private properties
	QString ProgramName;
	QLibrary FSUIPCLib;
	QString LocationOfDLL;
	float prevPosX, prevPosY, prevPosZ, prevRotX, prevRotY, prevRotZ;

	static int scale2AnalogLimits( float x, float min_x, float max_x );
	void loadSettings();
};

// Widget that has controls for FTNoIR protocol client-settings.
class FSUIPCControls: public QWidget, Ui::UICFSUIPCControls, public IProtocolDialog
{
    Q_OBJECT
public:

	explicit FSUIPCControls();
    virtual ~FSUIPCControls();
	void showEvent ( QShowEvent * event );

	void Release();											// Member functions which are accessible from outside the DLL
    void Initialize(QWidget *parent);
	void registerProtocol(IProtocol *protocol) {
		theProtocol = (FTNoIR_Protocol *) protocol;			// Accept the pointer to the Protocol
	};
	void unRegisterProtocol() {
		theProtocol = NULL;									// Reset the pointer
	};

private:
	Ui::UICFSUIPCControls ui;
	void loadSettings();
	void save();

	/** helper **/
	bool settingsDirty;
	FTNoIR_Protocol *theProtocol;

private slots:
	void doOK();
	void doCancel();
	void settingChanged() { settingsDirty = true; };
	void getLocationOfDLL();
};

//*******************************************************************************************************
// FaceTrackNoIR Protocol DLL. Functions used to get general info on the Protocol
//*******************************************************************************************************
class FTNoIR_ProtocolDll : public IProtocolDll
{
public:
	FTNoIR_ProtocolDll();
	~FTNoIR_ProtocolDll();

	void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("FS2002/FS2004"); };
	void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("FSUIPC"); };
	void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("Microsoft FS2004 protocol"); };

	void getIcon(QIcon *icon) { *icon = QIcon(":/images/FS9.png"); };
};


#endif//INCLUDED_FSUIPCSERVER_H
//END
