#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include "CameraCalibrator.hpp"

using namespace cv;
using namespace std;

Size boardSize(9, 6);

void initCamera() {
    Mat image, gray;
    
    CameraCalibrator cc;

    for (int i = 0;i < 50;i++) {
        stringstream stream;
        stream << "pic/" << i << ".bmp";
        string file = stream.str();

        image = imread(file, 1);
        if (cc.addImage(image, boardSize))
            printf("file%d done!\n", i);
    }

    cc.train();

    cc.save("CameraCalibrator.txt");
}

Mat initTransform(VideoCapture cap) {
    bool flag = false;
    Mat transMatrix;
    while (!flag) {
        Mat frame;
        cap >> frame;
        imshow("Original", frame);

        vector<Point2f> corners;
        flag = findChessboardCorners(frame, boardSize, corners);
        if (flag) {
            Mat gray;
            cvtColor(frame, gray, CV_BGR2GRAY);
            cornerSubPix(gray, corners, Size(11, 11), Size(-1, -1), 
                TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
            
            vector<Point2f> real(4);
            real[0] = corners[0];
            real[1] = corners[8];
            real[2] = corners[45];
            real[3] = corners[53];
        
            vector<Point2f> points(4); 
            points[0] = Point2f(8, 5);
            points[1] = Point2f(0, 5);
            points[2] = Point2f(8, 0);
            points[3] = Point2f(0, 0);
    
            transMatrix = getPerspectiveTransform(points, real);
            return transMatrix;
        }

        if (waitKey(30) >= 0)
            break;
    }
}

void showBirdEye(VideoCapture cap, Mat map1, Mat map2, Mat trans, int zoom) {
    Mat birdView, frame, undistorted;
    while (1) {
        cap >> undistorted;
        bool flag;
        vector<Point2f> corners;
        flag = findChessboardCorners(undistorted, boardSize, corners);
        
        if (flag) {
            Mat gray;
            cvtColor(undistorted, gray, CV_BGR2GRAY);
            cornerSubPix(gray, corners, Size(11, 11), Size(-1, -1), 
                TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
            
            vector<Point2f> real(4);
            real[0] = corners[0];
            real[1] = corners[8];
            real[2] = corners[45];
            real[3] = corners[53];
        
            vector<Point2f> points(4); 
            points[0] = Point2f(8, 5);
            points[1] = Point2f(0, 5);
            points[2] = Point2f(8, 0);
            points[3] = Point2f(0, 0);
    
            trans = getPerspectiveTransform(points, real);
        }

        remap(undistorted, frame, map1, map2, INTER_NEAREST);
        trans.at<double>(2, 2) = zoom;
        warpPerspective(undistorted, birdView, trans, frame.size(), WARP_INVERSE_MAP+INTER_LINEAR);
        imshow("Original", undistorted);
        imshow("BirdView", birdView);

        char c = (char)waitKey(30);
        if (c == ']') {
            zoom += 5;
        } else if (c == '[') {
            zoom -= 5;
        } else if (c > 0) {
            break;
        }
    }
}

const char* keys = {
    "{c | 0 | use camera or not }"
    "{fn | CameraCalibrator.txt | calibrator data }"
    "{z | 25 | zoom }"
};

int main(int argc, char** argv ) {
    CommandLineParser parser(argc, argv, keys);

    // string path = parser.get<string>("fn");

    FileStorage fs("CameraCalibrator.txt", FileStorage::READ);
    if (!fs.isOpened()) {
        initCamera();
        std::cout << "init Camera done" << endl;
        exit(0);
        // fs.open("CameraCalibrator.txt", FileStorage::READ);
    } 

    Mat cameraMatrix, distCoeffs;

    fs["cameraMatrix"] >> cameraMatrix;
    fs["distCoeffs"] >> distCoeffs;
    
    // Mat frame;
    // frame = imread("Bitmap/file1.bmp", 1);

    // Size boardSize(9, 6);
    // Mat newCameraMatrix = cameraMatrix.clone();
    // Mat map1, map2, R;
    // initUndistortRectifyMap(cameraMatrix, distCoeffs, R, newCameraMatrix, frame.size(), CV_32FC1, map1, map2);

    // Mat t = frame.clone();
    // remap(t, frame, map1, map2, INTER_NEAREST);

    // namedWindow("frame", WINDOW_AUTOSIZE);
    // imshow("frame", frame);
    // t.release();

    VideoCapture cap;
    int z = parser.get<int>("z");
    int videonum = parser.get<int>("c");
    cap.open(videonum);
    cout << videonum << endl;

    if (!cap.isOpened()) {
        cout << "video not open";
    }
    
    Mat temp;
    cap >> temp;
    Mat map1, map2, R;
    Mat newCameraMatrix = cameraMatrix.clone();
    initUndistortRectifyMap(cameraMatrix, distCoeffs, R, newCameraMatrix, temp.size(), CV_32FC1, map1, map2);

    namedWindow("Original");
    Mat trans = initTransform(cap);
    trans.at<double>(2, 2) = z;

    // std::cout << trans;

    namedWindow("BirdView");
    showBirdEye(cap, map1, map2, trans, z);

    // bool flag = false;
    // // Mat transMatrix;
    // namedWindow("Original");
    // namedWindow("BirdView");
    //     frame = imread("Bitmap/file1.bmp");
    //     imshow("Original", frame);

    //     vector<Point2f> corners;
    //     vector<Point2f> real(4);
    //     flag = findChessboardCorners(frame, boardSize, corners);

    //     cout << corners[0] << endl << "***************" << endl;
    //     // if (flag) {
    //     Mat gray;
    //     cvtColor(frame, gray, CV_BGR2GRAY);
    //     cornerSubPix(gray, corners, Size(11, 11), Size(-1, -1), 
    //         TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));

    //     real[0] = corners[0];
    //     real[1] = corners[8];
    //     real[2] = corners[45];
    //     real[3] = corners[53];

    //     vector<Point2f> points(4);
    //     points[0] = Point2f(8, 5);
    //     points[1] = Point2f(0, 5);
    //     points[2] = Point2f(8, 0);
    //     points[3] = Point2f(0, 0);
    //     // for (int i = 0;i < boardSize.height;i++)
    //     //     for (int j = 0;j < boardSize.width;j++) {
    //     //         points.push_back(Point2f((double)i+0.1, (double)j+0.1));
    //     //     }   
    //     // cout << points << endl << "============" << endl;
    //     // cout << real;
    //     // cout << corners << endl;

    //     Mat transMatrix = getPerspectiveTransform(points, real);
    // // }


    // // cout << transMatrix << endl << "sss" << endl;
    // transMatrix.at<double>(2, 2) = 25;
    // // cout << transMatrix << endl << "sss" << endl;
    // Mat birdView, undistorted;
    // undistorted = frame.clone();
    
    // remap(undistorted, frame, map1, map2, INTER_NEAREST);
    // imshow("Original", frame);
    // warpPerspective(frame, birdView, transMatrix, frame.size(), WARP_INVERSE_MAP+INTER_LINEAR);
    // imshow("BirdView", birdView);
    // int k = waitKey(0);
    // cout << endl << k;

    return 0;
}
