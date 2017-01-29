#pragma once

#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using std::vector;
using std::string;

class CameraCalibrator {
private:
    vector<vector<Point2f> > imagePoints;
    vector<vector<Point3f> > objectPoints;
    int square_size;
    Size imageSize;
    Mat cameraMatrix, distCoeffs;
    vector<Mat> rvecs, tvecs;
public:
    CameraCalibrator():
        square_size(50)
    { }

    bool addImage(Mat image, Size patternSize);

    bool train() {
        calibrateCamera(objectPoints, imagePoints, imageSize, cameraMatrix, distCoeffs, rvecs, tvecs);
    }

    Mat getCameraMatrix() {
        return cameraMatrix;
    }

    Mat getDistCoeffs() {
        return distCoeffs;
    }

    void save(string fileName);
};

bool CameraCalibrator::addImage(Mat image, Size patternSize) {
    Mat gray;
    cvtColor(image, gray, COLOR_BGR2GRAY);
    vector<Point2f> corners;
    
    imageSize = image.size();

    bool flag = findChessboardCorners(gray, patternSize, corners, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK);

    if (!flag) {
        return false;
    }

    imagePoints.push_back(corners);
    cornerSubPix(gray, corners, Size(11, 11), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));

    vector<Point3f> real;
    for (int i = 0;i < patternSize.height;i++) {
        for (int j = 0;j < patternSize.width;j++) {
            real.push_back(Point3f(i * square_size, j * square_size, 0.0f));
        }
    }

    // drawChessboardCorners(image, patternSize, corners, flag);
    
    // namedWindow("Display", WINDOW_AUTOSIZE);
    // imshow("Display", image);

    objectPoints.push_back(real);

    return flag;
}

void CameraCalibrator::save(string fileName) {
    FileStorage fs(fileName, FileStorage::WRITE);
    fs << "cameraMatrix" << cameraMatrix;
    fs << "distCoeffs" << distCoeffs;
    fs.release();
}