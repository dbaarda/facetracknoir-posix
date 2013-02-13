#include "ftnoir_filter_kalman.h"
#include "facetracknoir/global-settings.h"
#include <QDebug>

void kalman_load_settings(FTNoIR_Filter& self) {
    QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");    // Registry settings (in HK_USER)
    
    QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
    QSettings iniFile( currentFile, QSettings::IniFormat );     // Application settings (in INI-file)
    
    iniFile.beginGroup("ftnoir-filter-kalman");
    self.process_noise_covariance_matrix_all_values = iniFile.value("process-noise-covariance-matrix-all-values", 1e-5).toDouble();
    self.posteriori_error_covariance_matrix_all_values = iniFile.value("posteriori-error-covariance-matrix-all-values", 1e-1).toDouble();
    iniFile.endGroup();
}

void kalman_save_settings(FilterControls& self) {
    QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");    // Registry settings (in HK_USER)
    
    QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
    QSettings iniFile( currentFile, QSettings::IniFormat );     // Application settings (in INI-file)
    
    iniFile.beginGroup("ftnoir-filter-kalman");
    iniFile.setValue("process-noise-covariance-matrix-all-values", self.ui.pnoise->value());
    iniFile.setValue("posteriori-error-covariance-matrix-all-values", self.ui.post->value());
    iniFile.endGroup();
}

FTNoIR_Filter::FTNoIR_Filter() {
    Initialize();
    kalman_load_settings(*this);
}

void FTNoIR_Filter::Initialize() {
    qDebug() << "kalman init";
    kalman_dims = std::vector<cv::KalmanFilter>(6);
    //velocities = std::vector<double>(6);
    for (int i = 0; i < 6; i++) {
        //velocities[i] = 0;
        cv::KalmanFilter kalman(2, 1, 0, CV_64F);
        kalman.transitionMatrix = *(cv::Mat_<double>(2, 2) << 1, 1, 0, 1);
        cv::setIdentity(kalman.measurementMatrix);
        cv::setIdentity(kalman.processNoiseCov, cv::Scalar::all(process_noise_covariance_matrix_all_values));
        cv::setIdentity(kalman.measurementNoiseCov, cv::Scalar::all(posteriori_error_covariance_matrix_all_values));
        cv::setIdentity(kalman.errorCovPost, cv::Scalar::all(1));
        kalman_dims[i] = kalman;
    }
}

void FTNoIR_Filter::FilterHeadPoseData(THeadPoseData *current_camera_position, THeadPoseData *target_camera_position, THeadPoseData *new_camera_position, bool newTarget) {
    std::vector<double> in(6);
    std::vector<double> out(6);
    
    in[0] = target_camera_position->x;
    in[1] = target_camera_position->y;
    in[2] = target_camera_position->z;
    in[3] = target_camera_position->yaw;
    in[4] = target_camera_position->pitch;
    in[5] = target_camera_position->roll;
    
    for (int i = 0; i < 6; i++) {
        //double velocity = in[i] - velocities[i];
        cv::Mat pred = kalman_dims[i].predict();
        //out[i] = in[i] + pred.at<double>(0, 0);
        out[i] = pred.at<double>(0, 0);
        if (newTarget) {
            cv::Mat measurement(1, 1, CV_64F);
            //measurement.at<double>(0, 0) = velocity;
            measurement.at<double>(0, 0) = in[i];
            kalman_dims[i].correct(measurement);
        }
        //velocities[i] = out[i] - in[i];
    }
    
    new_camera_position->x = out[0];
    new_camera_position->y = out[1];
    new_camera_position->z = out[2];
    new_camera_position->yaw = out[3];
    new_camera_position->pitch = out[4];
    new_camera_position->roll = out[5];
    
#if 1
    current_camera_position->x = out[0];
    current_camera_position->y = out[1];
    current_camera_position->z = out[2];
    current_camera_position->yaw = out[3];
    current_camera_position->pitch = out[4];
    current_camera_position->roll = out[5];
#endif
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