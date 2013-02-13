#pragma once
#ifndef INCLUDED_FTN_FILTER_H
#define INCLUDED_FTN_FILTER_H

#undef FTNOIR_TRACKER_BASE_LIB
#define FTNOIR_TRACKER_BASE_EXPORT Q_DECL_IMPORT

#include "ftnoir_filter_base/ftnoir_filter_base.h"
#include "ui_ftnoir_kalman_filtercontrols.h"
#include "facetracknoir/global-settings.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <QString>
#include <QIcon>
#include <QWidget>
#include <QObject>

class FTNOIR_FILTER_BASE_EXPORT FTNoIR_Filter : public IFilter
{
public:
    FTNoIR_Filter();
    void Initialize();
    void FilterHeadPoseData(THeadPoseData *current_camera_position, THeadPoseData *target_camera_position, THeadPoseData *new_camera_position, bool newTarget);
    
private:
    std::vector<cv::KalmanFilter> kalman_dims;
    //std::vector<double> velocities;
};

class FTNOIR_FILTER_BASE_EXPORT FTNoIR_FilterDll : public Metadata
{
public:
    void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("Kalman filter"); }
    void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("Kalman filter"); }
    void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("Kalman filter"); }
    void getIcon(QIcon *icon){ *icon = QIcon(":/images/filter-16.png"); }
};

class FTNOIR_FILTER_BASE_EXPORT FilterControls: public QWidget, Ui::KalmanUICFilterControls, public IFilterDialog
{
    Q_OBJECT
public:
    
    explicit FilterControls() {
        ui.setupUi(this);
        show();
    }
    virtual ~FilterControls() {}
    void showEvent ( QShowEvent * event ) {
        show();
    }
    
    void Initialize(QWidget *parent, IFilter* ptr) {}
    
private:
    Ui::KalmanUICFilterControls ui;
    
private slots:
    void doOK();
    void doCancel();
};

#endif