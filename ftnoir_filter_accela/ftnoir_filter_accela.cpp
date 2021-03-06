/* Copyright (c) 2012 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
/*
	Modifications (last one on top):
		20120807 - WVR: FunctionConfig is now also used for the Filter. The extrapolation was adapted from Stanislaw.
					    Additional changes: I have added two parameters to the constructor of FunctionConfig and
						renamed 3 member-functions (getFilterFullName is now called getFullName).
*/
#include "ftnoir_filter_accela/ftnoir_filter_accela.h"
#include "math.h"
#include <QDebug>
#include <float.h>
#include "facetracknoir/global-settings.h"

#if !defined(_WIN32) && !defined(__WIN32)
#   define _isnan isnan
#endif

FTNoIR_Filter::FTNoIR_Filter() :
	functionConfig("Accela-Scaling-Rotation", 4, 6),
	translationFunctionConfig("Accela-Scaling-Translation", 4, 6)
{
	first_run = true;
	kMagicNumber = 100.0f;
	loadSettings();					// Load the Settings
}

FTNoIR_Filter::~FTNoIR_Filter()
{

}

void FTNoIR_Filter::Initialize()
{
}

void FTNoIR_Filter::loadSettings() {
	QList<QPointF> defPoints;

	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	defPoints.clear();
    for (int i = 0; i < defScaleRotation.size(); i++) {		// Get the default points (hardcoded!)
		defPoints.append(defScaleRotation[i]);
	}
	functionConfig.loadSettings(iniFile, defPoints);

	defPoints.clear();
    for (int i = 0; i < defScaleTranslation.size(); i++) {		// Get the default points (hardcoded!)
		defPoints.append(defScaleTranslation[i]);
	}
	translationFunctionConfig.loadSettings(iniFile, defPoints);

	iniFile.beginGroup ( "Accela" );
	kMagicNumber = iniFile.value ( "Reduction", 100 ).toFloat();
	iniFile.endGroup ();

}

void FTNoIR_Filter::FilterHeadPoseData(THeadPoseData *current_camera_position, THeadPoseData *target_camera_position, THeadPoseData *new_camera_position, bool newTarget)
{
	double target[6];
	double prev_output[6];
	float output[6];
	int i=0;

	prev_output[0]=current_camera_position->x;
	prev_output[1]=current_camera_position->y;
	prev_output[2]=current_camera_position->z;
	prev_output[3]=current_camera_position->yaw;
	prev_output[4]=current_camera_position->pitch;
	prev_output[5]=current_camera_position->roll;

	target[0]=target_camera_position->x;
	target[1]=target_camera_position->y;
	target[2]=target_camera_position->z;
	target[3]=target_camera_position->yaw;
	target[4]=target_camera_position->pitch;
	target[5]=target_camera_position->roll;

	if (first_run)
	{
		functionConfig.setTrackingActive(true);
		translationFunctionConfig.setTrackingActive(true);
		new_camera_position->x=target[0];
		new_camera_position->y=target[1];
		new_camera_position->z=target[2];
		new_camera_position->yaw=target[3];
		new_camera_position->pitch=target[4];
		new_camera_position->roll=target[5];

		first_run=false;
		return;
	}

	for (i=0;i<6;i++)
	{
		if (_isnan(target[i]))
			return;

		if (_isnan(prev_output[i]))
			return;

		double e2 = target[i];
		double start = prev_output[i];
		double vec = e2 - start;
		int sign = vec < 0 ? -1 : 1;
		double x = fabs(vec);
		QList<QPointF> points = (i >= 3 ? functionConfig : translationFunctionConfig).getPoints();
		int extrapolatep = 0;
		double ratio;
		double maxx;
		double add;
		// extrapolation of a spline
		if (points.size() > 1) {
			QPointF last = points[points.size() - 1];
			QPointF penultimate = points[points.size() - 2];
			ratio = (last.y() - penultimate.y()) / (last.x() - penultimate.x());
			extrapolatep = 1;
			add = last.y();
			maxx = last.x();
		}
		double foo = extrapolatep && x > maxx ? add + ratio * (x - maxx) : (i >= 3 ? functionConfig : translationFunctionConfig).getValue(x);
		// the idea is that "empty" updates without new head pose data are still
		// useful for filtering, as skipping them would result in jerky output.
		// the magic "100" is the amount of calls to the filter by FTNOIR per sec.
		// WVR: Added kMagicNumber for Patrick
        double velocity = foo / (kMagicNumber > 0 ? kMagicNumber : 100.0);
		double sum = start + velocity * sign;
		bool done = (sign > 0 ? sum >= e2 : sum <= e2);
		if (done) {
			output[i] = e2;
		} else {
			output[i] = sum;
		}

		if (_isnan(output[i]))
			return;
	}

	new_camera_position->x=output[0];
	new_camera_position->y=output[1];
	new_camera_position->z=output[2];
	new_camera_position->yaw=output[3];
	new_camera_position->pitch=output[4];
	new_camera_position->roll=output[5];

	current_camera_position->x=output[0];
	current_camera_position->y=output[1];
	current_camera_position->z=output[2];
	current_camera_position->yaw=output[3];
	current_camera_position->pitch=output[4];
	current_camera_position->roll=output[5];
}

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Filter object.

// Export both decorated and undecorated names.
//   GetFilter     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetFilter@0  - Common name decoration for __stdcall functions in C language.

extern "C" FTNOIR_FILTER_BASE_EXPORT void* CALLING_CONVENTION GetConstructor()
{
    return (IFilter*) new FTNoIR_Filter;
}
