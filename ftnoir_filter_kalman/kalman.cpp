/* Copyright (c) 2013 Stanisław Halik <sthalik@misaki.pl>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "ftnoir_filter_kalman.h"
#include "facetracknoir/global-settings.h"
#include <QDebug>
#include <math.h>

void kalman_load_settings(FTNoIR_Filter& self) {
    QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");    // Registry settings (in HK_USER)
    
    QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
    QSettings iniFile( currentFile, QSettings::IniFormat );     // Application settings (in INI-file)
    
    iniFile.beginGroup("ftnoir-filter-kalman");
    self.process_noise_covariance_matrix_all_values = iniFile.value("process-noise-covariance-matrix-all-values", "1e-14").toDouble();
    self.posteriori_error_covariance_matrix_all_values = iniFile.value("posteriori-error-covariance-matrix-all-values", "1e-12").toDouble();
    self.accl = iniFile.value("accel-coefficient", DEFAULT_ACCL).toDouble();
    iniFile.endGroup();
}

void kalman_save_settings(FilterControls& self) {
    QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");    // Registry settings (in HK_USER)
    
    QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
    QSettings iniFile( currentFile, QSettings::IniFormat );     // Application settings (in INI-file)
    
    iniFile.beginGroup("ftnoir-filter-kalman");
    iniFile.setValue("process-noise-covariance-matrix-all-values", self.ui.pnoise->value());
    iniFile.setValue("posteriori-error-covariance-matrix-all-values", self.ui.post->value());
    iniFile.setValue("accel-coefficient", self.ui.accl->value());
    iniFile.endGroup();
}

FTNoIR_Filter::FTNoIR_Filter() {
    kalman_load_settings(*this);
    Initialize();
}

// the following was written by Donovan Baarda <abo@minkirri.apana.org.au>
// https://sourceforge.net/p/facetracknoir/discussion/1150909/thread/418615e1/?limit=25#af75/084b
// minor changes to order of magnitude -sh
void FTNoIR_Filter::Initialize() {
    kalman.init(12, 6, 0, CV_64F);
    double accel_variance = 1e-2;
    kalman.transitionMatrix = *(cv::Mat_<double>(12, 12) <<
    1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1);
    double a = 0.25 * accel_variance;
    double b = 0.5 * accel_variance;
    double c = 1.0 * accel_variance;
    kalman.processNoiseCov = (cv::Mat_<double>(12, 12) <<
    a, 0, 0, 0, 0, 0, b, 0, 0, 0, 0, 0,
    0, a, 0, 0, 0, 0, 0, b, 0, 0, 0, 0,
    0, 0, a, 0, 0, 0, 0, 0, b, 0, 0, 0,
    0, 0, 0, a, 0, 0, 0, 0, 0, b, 0, 0,
    0, 0, 0, 0, a, 0, 0, 0, 0, 0, b, 0,
    0, 0, 0, 0, 0, a, 0, 0, 0, 0, 0, b,
    b, 0, 0, 0, 0, 0, c, 0, 0, 0, 0, 0,
    0, b, 0, 0, 0, 0, 0, c, 0, 0, 0, 0,
    0, 0, b, 0, 0, 0, 0, 0, c, 0, 0, 0,
    0, 0, 0, b, 0, 0, 0, 0, 0, c, 0, 0,
    0, 0, 0, 0, b, 0, 0, 0, 0, 0, c, 0,
    0, 0, 0, 0, 0, b, 0, 0, 0, 0, 0, c);
    cv::setIdentity(kalman.measurementMatrix);
    cv::setIdentity(kalman.measurementNoiseCov, cv::Scalar::all(accl));
    cv::setIdentity(kalman.errorCovPost, cv::Scalar::all(accel_variance * 1e-4));
}

void FTNoIR_Filter::FilterHeadPoseData(THeadPoseData *current_camera_position, THeadPoseData *target_camera_position, THeadPoseData *new_camera_position, bool newTarget) {
    cv::Mat output = kalman.predict();
    if (newTarget) {
        cv::Mat measurement(6, 1, CV_64F);
        measurement.at<double>(0) = target_camera_position->yaw;
        measurement.at<double>(1) = target_camera_position->pitch;
        measurement.at<double>(2) = target_camera_position->roll;
        measurement.at<double>(3) = target_camera_position->x;
        measurement.at<double>(4) = target_camera_position->y;
        measurement.at<double>(5) = target_camera_position->z;
        kalman.correct(measurement);
    }
    new_camera_position->yaw = output.at<double>(0);
    new_camera_position->pitch = output.at<double>(1);
    new_camera_position->roll = output.at<double>(2);
    new_camera_position->x = output.at<double>(3);
    new_camera_position->y = output.at<double>(4);
    new_camera_position->z = output.at<double>(5);
    target_camera_position->yaw = output.at<double>(0);
    target_camera_position->pitch = output.at<double>(1);
    target_camera_position->roll = output.at<double>(2);
    target_camera_position->x = output.at<double>(3);
    target_camera_position->y = output.at<double>(4);
    target_camera_position->z = output.at<double>(5);
}

void FilterControls::doOK() {
    kalman_save_settings(*this);
    close();
}

void FilterControls::doCancel() {
    if (settingsDirty) {
        int ret = QMessageBox::question ( this, "Settings have changed", "Do you want to save the settings?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Discard );
        
        qDebug() << "doCancel says: answer =" << ret;
        
        switch (ret) {
            case QMessageBox::Save:
                kalman_save_settings(*this);
                this->close();
                break;
            case QMessageBox::Discard:
                this->close();
                break;
            case QMessageBox::Cancel:
                // Cancel was clicked
                break;
            default:
                // should never be reached
                break;
        }
    }
    else {
        this->close();
    }
}

extern "C" FTNOIR_FILTER_BASE_EXPORT Metadata* CALLING_CONVENTION GetMetadata()
{
    return new FTNoIR_FilterDll;
}

extern "C" FTNOIR_FILTER_BASE_EXPORT void* CALLING_CONVENTION GetConstructor()
{
    return (IFilter*) new FTNoIR_Filter;
}

extern "C" FTNOIR_FILTER_BASE_EXPORT void* CALLING_CONVENTION GetDialog() {
    return (IFilterDialog*) new FilterControls;
}
