/* Copyright (c) 2012 Stanisław Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include <QPointF>
#include <QList>
#include "functionconfig.h"
#include <QtAlgorithms>
#include <QtAlgorithms>
#include <QSettings>
#include <math.h>
#include <QPixmap>
#include <QDebug>

//
// Constructor with List of Points in argument.
//
FunctionConfig::FunctionConfig(QString title, QList<QPointF> points, int intMaxInput, int intMaxOutput) :
    _mutex(QMutex::Recursive)
{
	_title = title;
	_points = points;
	_data = 0;
	_size = 0;
	lastValueTracked = QPointF(0,0);
	_tracking_active = false;
	_max_Input = intMaxInput;					// Added WVR 20120805
	_max_Output = intMaxOutput;
	reload();
}

//
// Constructor with only Title in arguments.
//
FunctionConfig::FunctionConfig(QString title, int intMaxInput, int intMaxOutput) {
	_title = title;
	_points = QList<QPointF>();
	_data = 0;
	_size = 0;
	lastValueTracked = QPointF(0,0);
	_tracking_active = false;
	_max_Input = intMaxInput;					// Added WVR 20120805
	_max_Output = intMaxOutput;
	reload();
}

//
// Calculate the value of the function, given the input 'x'.
// Used to draw the curve and, most importantly, to translate input to output.
// The return-value is also stored internally, so the Widget can show the current value, when the Tracker is running.
//
double FunctionConfig::getValue(double x) {
	int x2 = (int) (std::min<double>(std::max<double>(x, -360), 360) * MEMOIZE_PRECISION);
    _mutex.lock();
	double ret = getValueInternal(x2);
	lastValueTracked.setX(x);
	lastValueTracked.setY(ret);
    _mutex.unlock();
	return ret;
}

//
// The return-value is also stored internally, so the Widget can show the current value, when the Tracker is running.
//
bool FunctionConfig::getLastPoint(QPointF& point ) {

    _mutex.lock();
	point = lastValueTracked;
    _mutex.unlock();
	
	return _tracking_active;
}

double FunctionConfig::getValueInternal(int x) {
	double sign = x < 0 ? -1 : 1;
	x = x < 0 ? -x : x;
	double ret;
	if (!_data)
		ret = 0;
	else if (_size == 0)
		ret = 0;
	else if (x < 0)
		ret = 0;
	else if (x < _size && x >= 0)
		ret = _data[x];
	else
		ret = _data[_size - 1];
	return ret * sign;
}

static __inline QPointF ensureInBounds(QList<QPointF> points, int i) {
	int siz = points.size();
	if (siz == 0 || i < 0)
		return QPointF(0, 0);
	if (siz > i)
		return points[i];
	return points[siz - 1];
}

static bool sortFn(const QPointF& one, const QPointF& two) {
	return one.x() < two.x();
}

void FunctionConfig::reload() {
	_size = 0;

	if (_points.size())
		qStableSort(_points.begin(), _points.end(), sortFn);

	if (_data)
        delete[] _data;
	_data = NULL;
	if (_points.size()) {
		_data = new double[_size = MEMOIZE_PRECISION * _points[_points.size() - 1].x()];

		for (int k = 0; k < _points[0].x() * MEMOIZE_PRECISION; k++) {
			_data[k] = _points[0].y() * k / (_points[0].x() * MEMOIZE_PRECISION);
		}

		for (int i = 1; i < _points.size(); i++) {
			QPointF a = ensureInBounds(_points, i - 2);
			QPointF b = ensureInBounds(_points, i - 1);
			QPointF c = ensureInBounds(_points, i);
			QPointF d = ensureInBounds(_points, i + 1);

			double Pc = c.y();
			double Pb = b.y();
			double dist = c.x() - b.x();
			double Pa = b.x() - a.x() == 0 ? b.y() : b.y() + (a.y() - b.y()) * dist / (b.x() - a.x());
			double Pd = d.x() - c.x() == 0 ? d.y() : c.y() + (d.y() - c.y()) * dist / (d.x() - c.x());

			double y_4 = Pd - 3*Pc + 3*Pb - Pa;
			double y_3 = -Pd + 4*Pc - 5*Pb + 2*Pa;
			double y_2 = Pc-Pa;
			double y_1 = 2*Pb;

			int end = _points[i].x() * MEMOIZE_PRECISION;
			int start = _points[i-1].x() * MEMOIZE_PRECISION;
			for (int j = start; j < end; j++) {
				double t = end-start == 0 ? 0 : (j-start)/(double)(end-start);
				double t2 = t*t;
				double t3 = t*t*t;

				_data[j] = 0.5*(y_4*t3 + y_3*t2 + y_2*t + y_1);
			}
		}
	}
}

FunctionConfig::~FunctionConfig() {
	if (_data)
		delete _data;
}

//
// Remove a Point from the Function.
// Used by the Widget.
//
void FunctionConfig::removePoint(int i) {
    _mutex.lock();
	_points.removeAt(i);
	reload();
    _mutex.unlock();
}

//
// Add a Point to the Function.
// Used by the Widget and by loadSettings.
//
void FunctionConfig::addPoint(QPointF pt) {
    _mutex.lock();
	_points.append(pt);
	reload();
    _mutex.unlock();
}

//
// Move a Function Point.
// Used by the Widget.
//
void FunctionConfig::movePoint(int idx, QPointF pt) {
    _mutex.lock();
	_points[idx] = pt;
	reload();
    _mutex.unlock();
}

//
// Return the List of Points.
// Used by the Widget.
//
QList<QPointF> FunctionConfig::getPoints() {
	QList<QPointF> ret;
    _mutex.lock();
	for (int i = 0; i < _points.size(); i++) {
		ret.append(_points[i]);
	}
    _mutex.unlock();
	return ret;
}

//
// Load the Points for the Function from the INI-file designated by settings.
// Settings for a specific Curve are loaded from their own Group in the INI-file.
//
void FunctionConfig::loadSettings(QSettings& settings) {
QPointF newPoint;

	QList<QPointF> points;
	settings.beginGroup(QString("Curves-%1").arg(_title));
	
    int max = settings.value("point-count", 0).toInt();
	for (int i = 0; i < max; i++) {
		newPoint = QPointF(settings.value(QString("point-%1-x").arg(i), (i + 1) * _max_Input/2).toDouble(),
                           settings.value(QString("point-%1-y").arg(i), (i + 1) * _max_Output/2).toDouble());
        //
		// Make sure the new Point fits in the Function Range.
		// Maybe this can be improved?
		//
		if (newPoint.x() > _max_Input) {
			newPoint.setX(_max_Input);
		}
		if (newPoint.y() > _max_Output) {
			newPoint.setY(_max_Output);
		}
		points.append(newPoint);
	}
    settings.endGroup();
    _mutex.lock();
	_points = points;
	reload();
    _mutex.unlock();
}

//
// Load the Points for the Function from the INI-file designated by settings.
// Settings for a specific Curve are loaded from their own Group in the INI-file.
//
// A default List of points is available in defPoints, to be used if "point-count" is missing completely.
//
void FunctionConfig::loadSettings(QSettings& settings, QList<QPointF>& defPoints) {
QPointF newPoint;

	QList<QPointF> points;
	settings.beginGroup(QString("Curves-%1").arg(_title));

	qDebug() << "FunctionConfig::loadSettings2 title = " << _title;
	
	//
	// Read the number of points, that was last saved. Default to defPoints, if key doesn't exist.
	//
	int max = settings.value("point-count", 0).toInt();
	if (max <= 0) {
		qDebug() << "FunctionConfig::loadSettings2 number of points = " << defPoints.count();
		points = defPoints;
	}
	else {
		qDebug() << "FunctionConfig::loadSettings2 max = " << max;
		for (int i = 0; i < max; i++) {
			newPoint = QPointF(settings.value(QString("point-%1-x").arg(i), (i + 1) * _max_Input/2).toDouble(),
                               settings.value(QString("point-%1-y").arg(i), (i + 1) * _max_Output/2).toDouble());
			//
			// Make sure the new Point fits in the Function Range.
			// Maybe this can be improved?
			//
			if (newPoint.x() > _max_Input) {
				newPoint.setX(_max_Input);
			}
			if (newPoint.y() > _max_Output) {
				newPoint.setY(_max_Output);
			}
			points.append(newPoint);
		}
	}
	settings.endGroup();
    _mutex.lock();
	_points = points;
	reload();
    _mutex.unlock();
}

//
// Save the Points for the Function to the INI-file designated by settings.
// Settings for a specific Curve are saved in their own Group in the INI-file.
// The number of Points is also saved, to make loading more convenient.
//
void FunctionConfig::saveSettings(QSettings& settings) {
    _mutex.lock();
	settings.beginGroup(QString("Curves-%1").arg(_title));
	int max = _points.size();
	settings.setValue("point-count", max);
	for (int i = 0; i < max; i++) {
		settings.setValue(QString("point-%1-x").arg(i), _points[i].x());
		settings.setValue(QString("point-%1-y").arg(i), _points[i].y());
    }

    for (int i = max; true; i++)
    {
        QString x = QString("point-%1-x").arg(i);
        if (!settings.contains(x))
            break;
        settings.remove(x);
        settings.remove(QString("point-%1-y").arg(i));
    }
	settings.endGroup();
    _mutex.unlock();
}
