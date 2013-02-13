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
    virtual ~FTNoIR_Filter() {
    }
    void Initialize();
    void FilterHeadPoseData(THeadPoseData *current_camera_position, THeadPoseData *target_camera_position, THeadPoseData *new_camera_position, bool newTarget);
    cv::KalmanFilter kalman_r, kalman_t;
    double process_noise_covariance_matrix_all_values_r, process_noise_covariance_matrix_all_values_t;
    double posteriori_error_covariance_matrix_all_values_r, posteriori_error_covariance_matrix_all_values_t;
};

void kalman_load_settings(FTNoIR_Filter& self);
void kalman_save_settings(FTNoIR_Filter& self);

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
    explicit FilterControls() : settingsDirty(false) {
        ui.setupUi(this);
        QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");    // Registry settings (in HK_USER)
        
        QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
        QSettings iniFile( currentFile, QSettings::IniFormat );     // Application settings (in INI-file)
        
        iniFile.beginGroup("ftnoir-filter-kalman");
        ui.postr->setValue(iniFile.value("posteriori-error-covariance-matrix-all-values-r", 1e-10).toDouble());
        ui.pnoiser->setValue(iniFile.value("process-noise-covariance-matrix-all-values-r", 1e-08).toDouble());
        ui.postt->setValue(iniFile.value("posteriori-error-covariance-matrix-all-values-t", 5e-08).toDouble());
        ui.pnoiset->setValue(iniFile.value("process-noise-covariance-matrix-all-values-t", 5e-06).toDouble());
        iniFile.endGroup();
        connect(ui.btnOk, SIGNAL(clicked()), this, SLOT(doOK()));
        connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
        connect(ui.postr, SIGNAL(valueChanged(double)), this, SLOT(settingsChanged(double)));
        connect(ui.pnoiser, SIGNAL(valueChanged(double)), this, SLOT(settingsChanged(double)));
        connect(ui.postt, SIGNAL(valueChanged(double)), this, SLOT(settingsChanged(double)));
        connect(ui.pnoiset, SIGNAL(valueChanged(double)), this, SLOT(settingsChanged(double)));
        show();
    }
    virtual ~FilterControls() {}
    void showEvent ( QShowEvent * event ) {
        show();
    }
    
    void Initialize(QWidget *parent, IFilter* ptr) {
    }
    
    bool settingsDirty;
    Ui::KalmanUICFilterControls ui;
    
public slots:
    void doOK();
    void doCancel();
    void settingsChanged(double unused) {
        settingsDirty = true;
    }
};

#endif