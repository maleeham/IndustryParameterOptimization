QT       += core network sql

TEMPLATE = app
TARGET = 
DEPENDPATH += .


SOURCES += main.cpp camerawidget.cpp camerawindow.cpp \
    adc.cpp \
    qextserialport.cpp \
    qextserialenumerator_unix.cpp \
    posix_qextserialport.cpp
HEADERS += camerawidget.h camerawindow.h \
    adc.h \
    qextserialport_global.h \
    qextserialport.h \
    qextserialenumerator.h

OTHER_FILES +=


INCLUDEPATH += /usr/include/
INCLUDEPATH += /usr/include/opencv/
INCLUDEPATH += /usr/include/opencv2/

LIBS += -L/usr/lib/
LIBS += -lopencv_core
LIBS += -lopencv_imgproc
LIBS += -lopencv_highgui
LIBS += -lopencv_ml
LIBS += -lopencv_video
LIBS += -lopencv_features2d
LIBS += -lopencv_calib3d
LIBS += -lopencv_objdetect
LIBS += -lopencv_contrib
LIBS += -lopencv_legacy
LIBS += -lopencv_flann
LIBS += -lpthread
LIBS += -lrt
LIBS += -lv4lconvert
LIBS += -L/home/pi/build-SMTPEmail-raspberrypi-Debug
LIBS += -lSMTPEmail
LIBS += -L/usr/lib/
LIBS += -lwiringPi
LIBS += -lwiringPiDev
