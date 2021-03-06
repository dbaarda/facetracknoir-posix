/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2012	Wim Vriend (Developing)									*
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
********************************************************************************/
#pragma once
#ifndef INCLUDED_FTN_FILTER_H
#define INCLUDED_FTN_FILTER_H

#undef FTNOIR_TRACKER_BASE_LIB
#define FTNOIR_TRACKER_BASE_EXPORT Q_DECL_IMPORT

#include "ftnoir_filter_base/ftnoir_filter_base.h"
#include "ui_ftnoir_accela_filtercontrols.h"
#include <qfunctionconfigurator/functionconfig.h>
#include "facetracknoir/global-settings.h"

extern const QList<QPointF> defScaleRotation;
extern const QList<QPointF> defScaleTranslation;

//
// Macro to determine array-size
//
#define NUM_OF(x) (sizeof (x) / sizeof *(x))

//*******************************************************************************************************
// FaceTrackNoIR Filter class.
//*******************************************************************************************************
class FTNOIR_FILTER_BASE_EXPORT FTNoIR_Filter : public IFilter
{
public:
	FTNoIR_Filter();
	~FTNoIR_Filter();

    void Initialize();
	void FilterHeadPoseData(THeadPoseData *current_camera_position, THeadPoseData *target_camera_position, THeadPoseData *new_camera_position, bool newTarget);

private:
	void loadSettings();									// Load the settings from the INI-file
	THeadPoseData newHeadPose;								// Structure with new headpose

	bool	first_run;
	double kFactor, kFactorTranslation;
	double kSensitivity, kSensitivityTranslation;
	double kMagicNumber;									// Stanislaws' magic number (should be 100 according to him...)

	FunctionConfig functionConfig;
	FunctionConfig translationFunctionConfig;
};

//*******************************************************************************************************
// FaceTrackNoIR Filter Settings-dialog.
//*******************************************************************************************************

// Widget that has controls for FTNoIR protocol filter-settings.
class FTNOIR_FILTER_BASE_EXPORT FilterControls: public QWidget, Ui::AccelaUICFilterControls, public IFilterDialog
{
    Q_OBJECT
public:

	explicit FilterControls();
    virtual ~FilterControls();
	void showEvent ( QShowEvent * event );

	void Release();											// Member functions which are accessible from outside the DLL
    void Initialize(QWidget *parent, IFilter *ptr);

private:
    Ui::AccelaUICFilterControls ui;
	void loadSettings();
	void save();

	/** helper **/
	bool settingsDirty;

    IFilter* pFilter;										// If the filter was active when the dialog was opened, this will hold a pointer to the Filter instance
	FunctionConfig functionConfig;
	FunctionConfig translationFunctionConfig;

private slots:
	void doOK();
	void doCancel();
	void settingChanged(bool) { settingsDirty = true; };
	void settingChanged(int) { settingsDirty = true; };
};

//*******************************************************************************************************
// FaceTrackNoIR Filter DLL. Functions used to get general info on the Filter
//*******************************************************************************************************
class FTNoIR_FilterDll : public Metadata
{
public:
	FTNoIR_FilterDll();
	~FTNoIR_FilterDll();

    void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("Accela Filter Mk2"); }
    void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("Accela Mk2"); }
    void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("Accela filter Mk2"); }

    void getIcon(QIcon *icon){ *icon = QIcon(":/images/filter-16.png");	}
};


#endif						//INCLUDED_FTN_FILTER_H
//END

