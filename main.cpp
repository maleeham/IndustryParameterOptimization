#include <QApplication>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <assert.h>
#include "camerawidget.h"
#include "camerawindow.h"
#include <opencv2/imgproc/imgproc.hpp>
#include<opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#define h 240
#define w 320

using namespace cv;
using namespace std;


int main(int argc, char **argv) {

//   system("echo 1 > /proc/sys/vm/overcommit_memory");
//   CvCapture *camera = cvCreateCameraCapture(1);
  CvCapture *camera = cvCreateCameraCapture(0);

    assert(camera);
    IplImage *image = cvQueryFrame(camera);
    assert(image);

    QApplication app(argc, argv);

    CameraWindow *window = new CameraWindow(camera);

 //   window->setFixedSize(350,380);
    window->setFixedSize(350,500);
    window->setWindowTitle("Video View");
    window->show();
    int retval = app.exec();   

    cvReleaseCapture(&camera);
    
    return retval;
}

