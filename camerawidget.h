#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

//#include <QPixmap>
#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QImage>
#include <QDebug>
#include <opencv/cv.h>


class CameraWidget : public QWidget
{

public:
    CameraWidget(QWidget *parent = 0);
    ~CameraWidget(void);
    QPixmap toPixmap(IplImage *);
    void putFrame(IplImage *);

private:
    QLabel *m_imageLabel;
    QVBoxLayout *m_layout;
    QImage m_image;

}; 

#endif
