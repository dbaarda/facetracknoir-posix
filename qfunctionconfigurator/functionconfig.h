/* Copyright (c) 2011-2012, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include <QList>
#include <QPointF>
#include <QString>
#include <QSettings>
#include <QMutex>
#include "ftnoir_tracker_base/ftnoir_tracker_base.h"

#ifndef FUNCTION_CONFIG_H
#define FUNCTION_CONFIG_H

#define MEMOIZE_PRECISION 500

class FTNOIR_TRACKER_BASE_EXPORT FunctionConfig {
private:
    QMutex _mutex;
	QList<QPointF> _points;
	void reload();
	double* _data;
	int _size;
	QString _title;
	double getValueInternal(int x);
	QPointF lastValueTracked;								// The last input value requested by the Tracker, with it's output-value.
	bool _tracking_active;
	int _max_Input;
	int _max_Output;

public:
	//
	// Contructor(s) and destructor
	//
	FunctionConfig(QString title, QList<QPointF> points, int intMaxInput, int intMaxOutput);
	FunctionConfig(QString title, int intMaxInput, int intMaxOutput);
	virtual ~FunctionConfig();

	double getValue(double x);
	bool getLastPoint(QPointF& point);						// Get the last Point that was requested.

	//
	// Functions to manipulate the Function
	//
	void removePoint(int i);
	void addPoint(QPointF pt);
	void movePoint(int idx, QPointF pt);
	QList<QPointF> getPoints();
	void setMaxInput(int MaxInput) {
		_max_Input = MaxInput;
	}
	void setMaxOutput(int MaxOutput) {
		_max_Output = MaxOutput;
	}

	//
	// Functions to load/save the Function-Points to an INI-file
	//
	void saveSettings(QSettings& settings);
	void loadSettings(QSettings& settings);
	void loadSettings(QSettings& settings, QList<QPointF>& defPoints);

	void setTrackingActive(bool blnActive) {
		_tracking_active = blnActive;
	}
    QString getTitle() { return _title; }
};

#endif
