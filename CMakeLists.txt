cmake_minimum_required(VERSION 2.8)
project( DisplayImage )
find_package( OpenCV REQUIRED )
add_executable( test main.cpp CameraCalibrator.hpp )
target_link_libraries( test ${OpenCV_LIBS} )
