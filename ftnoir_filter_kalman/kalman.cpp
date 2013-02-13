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
    kalman_load_settings(*this);
    Initialize();
}

static void setup(cv::KalmanFilter& f, FTNoIR_Filter& self) {
    f.init(6, 3, 0, CV_64F);
    f.transitionMatrix = *(cv::Mat_<double>(6, 6) <<1, 0, 0, 0, 0, 0,
                                                    0, 1, 0, 0, 0, 0,
                                                    0, 0, 1, 0, 0, 0,
                                                    1, 0, 0, 1, 0, 0,
                                                    0, 1, 0, 0, 1, 0,
                                                    0, 0, 1, 0, 0, 1);
    cv::setIdentity(f.measurementMatrix);
    cv::setIdentity(f.processNoiseCov, cv::Scalar::all(self.process_noise_covariance_matrix_all_values));
    cv::setIdentity(f.measurementNoiseCov, cv::Scalar::all(self.posteriori_error_covariance_matrix_all_values));
    cv::setIdentity(f.errorCovPost, cv::Scalar::all(1));
}

void FTNoIR_Filter::Initialize() {
    setup(kalman_r, *this);
    setup(kalman_t, *this);
}

static void run(cv::KalmanFilter& f, bool newp, double v1, double v2, double v3, double& o1, double& o2, double& o3) {
    cv::Mat pred = f.predict();
    o1 = pred.at<double>(0);
    o2 = pred.at<double>(1);
    o3 = pred.at<double>(2);
    if (newp) {
        cv::Mat measurement(3, 1, CV_64F);
        measurement.at<double>(0) = v1;
        measurement.at<double>(1) = v2;
        measurement.at<double>(2) = v3;
        f.correct(measurement);
    }
}

void FTNoIR_Filter::FilterHeadPoseData(THeadPoseData *current_camera_position, THeadPoseData *target_camera_position, THeadPoseData *new_camera_position, bool newTarget) {
    run(kalman_r, newTarget,
        target_camera_position->yaw, target_camera_position->pitch, target_camera_position->roll,
        new_camera_position->yaw, new_camera_position->pitch, new_camera_position->roll);
    run(kalman_t, newTarget,
        target_camera_position->x, target_camera_position->y, target_camera_position->z,
        new_camera_position->x, new_camera_position->y, new_camera_position->z);
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