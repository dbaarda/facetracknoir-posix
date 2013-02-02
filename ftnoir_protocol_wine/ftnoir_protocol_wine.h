/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
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
*																				*
* FTServer		FTServer is the Class, that communicates headpose-data			*
*				to games, using the FreeTrackClient.dll.		         		*
********************************************************************************/
#pragma once
#ifndef INCLUDED_FTSERVER_H
#define INCLUDED_FTSERVER_H

#include "ftnoir_protocol_base/ftnoir_protocol_base.h"
#include "ui_ftnoir_winecontrols.h"
#include "fttypes.h"
#include <QMessageBox>
#include <QSettings>
#include <QLibrary>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include "facetracknoir/global-settings.h"
#include "compat/compat.h"

class FTNoIR_Protocol : public IProtocol
{
public:
	FTNoIR_Protocol();
	~FTNoIR_Protocol();

    void Initialize();

    bool checkServerInstallationOK();
	void sendHeadposeToGame( THeadPoseData *headpose, THeadPoseData *rawheadpose );
	void getNameFromGame( char *dest );					// Take care dest can handle up to 100 chars...

private:
    PortableLockedShm lck_shm;
    WineSHM* shm;
    QProcess wrapper;
};

// Widget that has controls for FTNoIR protocol client-settings.
class FTControls: public QWidget, public IProtocolDialog
{
    Q_OBJECT
public:

    FTControls();
    void showEvent ( QShowEvent * event ) {show();}
    void Initialize(QWidget *parent) {show();}
    void registerProtocol(IProtocol *protocol) {}
    void unRegisterProtocol() {}

private:
    Ui::UICFTControls ui;

private slots:
	void doOK();
	void doCancel();
};

//*******************************************************************************************************
// FaceTrackNoIR Protocol DLL. Functions used to get general info on the Protocol
//*******************************************************************************************************
class FTNoIR_ProtocolDll : public Metadata
{
public:
	FTNoIR_ProtocolDll();
	~FTNoIR_ProtocolDll();

    void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("Wine"); }
    void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("Wine"); }
    void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("Wine glue wrapper"); }

    void getIcon(QIcon *icon) { *icon = QIcon(":/images/wine.ico"); }
};


#endif//INCLUDED_FTSERVER_H
//END
