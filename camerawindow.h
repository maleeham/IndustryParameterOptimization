#ifndef CAMERAWINDOW_H_
#define CAMERAWINDOW_H_

#include <QWidget>
#include <QVBoxLayout>
#include <QDebug>
#include <QPushButton>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "camerawidget.h"
#include <opencv2/imgproc/imgproc.hpp>
#include<opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <QRect>
#include <QTimer>
#include <QTextEdit>
#include <QLabel>
#include <QTimer>
#include "adc.h"
#include "qextserialport.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>


using namespace cv;
using namespace std;

class CameraWindow : public QWidget
{
    Q_OBJECT
public:
    CameraWindow(CvCapture *camera, QWidget *parent=0);
    Mat src;
    Mat srcLR;

    IplImage *frame;
    Mat mat_img;

    void sleepFor(qint64 milliseconds);
    QextSerialPort *port;
    QNetworkAccessManager *m_networkAccessManager;

    void open_port();
    void fileread();
    void sensor_tempemailsend();
    void sensor_smokeemailsend();
    void sensor_fireemailsend();
    void write_port_gsm_hightemp();
    void write_port_gsm_smoke();
    void write_port_gsm_fire();
    void write_port_gsm_pir();
    void updatelink();

private:
    CameraWidget *m_cvwidget;
    CvCapture *m_camera;
    int m_photoCounter;
    // IplImage *frame;
    IplImage *image;
    adc *mcp3208;
    QTimer *t1;
    QTextEdit *text;
    QLabel *label1;
    QTimer *pirTimer;
    QTimer *pirTimer1;
    int lcd;


protected:
    void timerEvent(QTimerEvent*);

public slots:
    void sensor_emailsend();
    void pir();
    void pir1();
    void dataread();
    void lcddata();
    void onReadyRead();


};

#endif
