#include "ftnoir_filter_kalman.h"
#include "facetracknoir/global-settings.h"
#include <QDebug>

FTNoIR_Filter::FTNoIR_Filter() {
    Initialize();
}

void FTNoIR_Filter::Initialize() {
    kalman_dims = std::vector<cv::KalmanFilter>(6);
    //velocities = std::vector<double>(6);
    for (int i = 0; i < 6; i++) {
        //velocities[i] = 0;
        cv::KalmanFilter kalman(2, 1, 0, CV_64F);
        kalman.transitionMatrix = *(cv::Mat_<double>(2, 2) << 1, 1, 0, 1);
        cv::setIdentity(kalman.measurementMatrix);
        cv::setIdentity(kalman.processNoiseCov, cv::Scalar::all(1e-5));
        cv::setIdentity(kalman.measurementNoiseCov, cv::Scalar::all(1e-1));
        cv::setIdentity(kalman.errorCovPost, cv::Scalar::all(1));
        kalman_dims[i] = kalman;
    }
}

void FTNoIR_Filter::FilterHeadPoseData(THeadPoseData *current_camera_position, THeadPoseData *target_camera_position, THeadPoseData *new_camera_position, bool newTarget) {
    std::vector<double> in(6);
    std::vector<double> out(6);
    
    in[0] = out[0] = target_camera_position->x;
    in[1] = out[1] = target_camera_position->y;
    in[2] = out[2] = target_camera_position->z;
    in[3] = out[3] = target_camera_position->yaw;
    in[4] = out[4] = target_camera_position->pitch;
    in[5] = out[5] = target_camera_position->roll;
    

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
    
    current_camera_position->x = out[0];
    current_camera_position->y = out[1];
    current_camera_position->z = out[2];
    current_camera_position->yaw = out[3];
    current_camera_position->pitch = out[4];
    current_camera_position->roll = out[5];
}

void FilterControls::doOK() {
    close();
}

void FilterControls::doCancel() {
    close();
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