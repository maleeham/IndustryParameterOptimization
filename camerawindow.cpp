#include "camerawindow.h"
//OpenCV Headers
#include<cv.h>
#include<highgui.h>
#include <imgproc/imgproc_c.h>
#include "opencv2/imgproc/imgproc.hpp"
//Input-Output
#include<stdio.h>
//Blob Library Headers
#include <QDebug>
#include <QRect>
#include <stdlib.h>
#include <stdio.h>
#include <QThread>
#include <QCoreApplication>
#include <QTime>
#include <QDateTime>
#include <QFileDialog>
#include <stdlib.h>
#include <QDebug>
#include <QList>
#include <QFile>
#include <QStringList>
#include "../SmtpClient-for-Qt-master/src/SmtpMime"
#include <wiringPi.h>
#include <lcd.h>
#include <QtSql/QSqlError>
#include <QtSql/QtSql>
#include <QtSql/QSqlQuery>


#define LCD_RS  26               //Register select pin
#define LCD_E   27               //Enable Pin
#define LCD_D4  21               //Data pin 4
#define LCD_D5  22               //Data pin 5
#define LCD_D6  23               //Data pin 6
#define LCD_D7  24               //Data pin 7

#define port_name_gsm "/dev/ttyAMA0"


int  count1=0;
QString filename;


using namespace cv;
using namespace std;

#define h 240
#define w 320

FILE *fp;
char ch[100],au[100];

int t,l,s,f;
int fl=0;



QByteArray recvnumb;
QByteArray num,eail;
QByteArray sendnum;
QString sendemail;
QString pird="NO",fired="NO";

int dset=0;

class SleeperThread : public QThread
{
public:
    static void msleep(unsigned long msecs)
    {
        QThread::msleep(msecs);
    }
};
CameraWindow::CameraWindow(CvCapture *cam, QWidget *parent)
    : QWidget(parent)
{
    // wiringPiSetup();
    mcp3208 = new adc;
    m_networkAccessManager=new QNetworkAccessManager(this);

    //Initialise LCD(int rows, int cols, int bits, int rs, int enable, int d0, int d1, int d2, int d3, int d4, int d5, int d6, int d7)
    if (lcd = lcdInit (2, 16,4, LCD_RS, LCD_E ,LCD_D4 , LCD_D5, LCD_D6,LCD_D7,0,0,0,0))
    {
        qDebug()<< "lcdInit failed! \n";
        return ;
    }
    lcdPosition(lcd,0,0);           //Position cursor on the first line in the first column
    lcdPuts(lcd, "  IOT SECURITY    ");  //Print the text on the LCD at the current cursor postion
    lcdPosition(lcd,3,1);           //Position cursor on the first line in the first column
    lcdPuts(lcd, "    SYSTEM    ");  //Print the text on the LCD at the current cursor postion
	
	
    pinMode(28,INPUT); // PIR
    pinMode(0,OUTPUT); // Buzzer
    pinMode(1,OUTPUT); //led
    pinMode(2,OUTPUT);
    digitalWrite(0,0);
    digitalWrite(2,0);

    fileread();
    m_camera = cam;
    m_photoCounter = 0;
    m_cvwidget = new CameraWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;



    layout->addWidget(m_cvwidget);
    setLayout(layout);
    frame=cvCreateImage(cvSize(w,h),8,3);   //Original Image

    open_port();
    SleeperThread::msleep(1000);
    port->write("AT\r");
    SleeperThread::msleep(1000);
    port->write("AT+CMGF=1\r");
    SleeperThread::msleep(1000);

    //write_port_gsm_hightemp();

    startTimer(10);  // 0.1-second timer


    t1=new QTimer(this);
    connect(t1,SIGNAL(timeout()),this,SLOT(sensor_emailsend()));
    t1->start(60000);


    pirTimer = new QTimer(this);
    connect(pirTimer,SIGNAL (timeout()),this,SLOT(pir()));
    pirTimer->setSingleShot(false);

    pirTimer1 = new QTimer(this);
    connect(pirTimer1,SIGNAL (timeout()),this,SLOT(pir1()));
    pirTimer1->setSingleShot(false);

    pirTimer1->start(5000);
    pirTimer->start(100);


    QTimer *Timer1 = new QTimer(this);
    connect(Timer1,SIGNAL (timeout()),this,SLOT(dataread()));
    Timer1->setSingleShot(false);
    Timer1->start(500);
}




//Puts a new frame in the widget every 100 msec
void CameraWindow::timerEvent(QTimerEvent*)
{
    IplImage *fram = cvQueryFrame(m_camera);
    cvResize(fram,frame,CV_INTER_LINEAR );
    m_cvwidget->putFrame(frame);
}


void CameraWindow::sleepFor(qint64 milliseconds)
{
    qint64 timeToExitFunction = QDateTime::currentMSecsSinceEpoch()+milliseconds;
    while(timeToExitFunction>QDateTime::currentMSecsSinceEpoch())
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

void CameraWindow::pir()
{
    if(digitalRead(28) == HIGH)
    {
        count1++;
        qDebug()<<"Pir1 Detected";
        pird="YES";
    }
    else
    {
        pird="NO";
    }

}

void CameraWindow::pir1()
{
    if(count1>1)
    {
        count1=0;
        lcdClear(lcd);
        lcdPosition(lcd,0,0);           //Position cursor on the first line in the first column
        lcdPuts(lcd, "HUMAN DETECTED");  //Print the text on the LCD at the current cursor postion
        qDebug()<<"PIR Detected";
        IplImage *image = cvQueryFrame(m_camera);
        QPixmap photo = m_cvwidget->toPixmap(image);
        if (photo.save("/home/pi/build-QtOpenCV-raspberrypi-Debug/" + QString::number(m_photoCounter) + ".jpg"))
        {

            qDebug("Picture successfully saved!");
            photo.save("/media/pi/5726-0B9F/" + QString::number(m_photoCounter) + ".jpg");
            qDebug()<<"Start login";
            SmtpClient smtp("smtp.gmail.com", 465, SmtpClient::SslConnection);
            smtp.setUser("ggeets41@gmail.com");
            smtp.setPassword("geeta4599");
            qDebug()<<"start Send email";
            MimeMessage message;
            EmailAddress sender("ggeets41@gmail.com", "Iot Device");
            message.setSender(&sender);
            EmailAddress to(sendemail, "User");
            message.addRecipient(&to);
            message.setSubject("Human Alert");
            MimeText text;
            text.setText("Image\n");
            message.addPart(&text);
            // Now we create the attachment object
            MimeAttachment attachment (new QFile(QString::number(m_photoCounter) + ".jpg"));
            // the file type can be setted. (by default is application/octet-stream)
            attachment.setContentType("image/jpg");
            // Now add it to message
            message.addPart(&attachment);
            if (!smtp.connectToHost()) {
                qDebug() << "Failed to connect to host!" << endl;
                return;
            }
            if (!smtp.login()) {
                qDebug() << "Failed to login!" << endl;
                return;
            }
            if (!smtp.sendMail(message)) {
                qDebug() << "Failed to send mail!" << endl;
                return;
            }
            qDebug()<<"email is sending sucess";
            lcdClear(lcd);
            lcdPosition(lcd,0,0);           //Position cursor on the first line in the first column
            lcdPuts(lcd, "EMail is sending");
            lcdPosition(lcd,0,1);
            lcdPuts(lcd, "sucess");
            m_photoCounter++;
            smtp.quit();
            updatelink();
        }
        else
        {
            qDebug("Error while saving the picture");
        }
    }

}

void CameraWindow::dataread()
{
    t=mcp3208->read_mcp3208_adc(0)/10;
    l=mcp3208->read_mcp3208_adc(1)/10;
    s=mcp3208->read_mcp3208_adc(2);
    f=mcp3208->read_mcp3208_adc(3)/100;
    t=t-5;
    if(l<100)
    {
        qDebug() << "LOW LDR ";
        lcdClear(lcd);
        lcdPosition(lcd,0,0);           //Position cursor on the first line in the first column
        lcdPuts(lcd, "LED ON");
        digitalWrite(1,1);
    }
    else
    {
        lcdClear(lcd);
        lcdPosition(lcd,0,0);           //Position cursor on the first line in the first column
        lcdPuts(lcd, "LED OFF");
        digitalWrite(1,0);
    }
    lcdClear(lcd);
    lcdPosition(lcd,0,0);           //Position cursor on the first line in the first column
    char buf[30];
    sprintf(buf,"T=%d L=%d",t,l);
    lcdPuts(lcd,buf);
    lcdPosition(lcd,0,1);           //Position cursor on the first line in the first column
    sprintf(buf,"S=%d F=%d",s,f);
    lcdPuts(lcd,buf);
    if(t>50)
    {
        qDebug() << "HIGH TEMPERATURE";
        digitalWrite(0,1);
        sensor_tempemailsend();
        SleeperThread::msleep(1000);
        write_port_gsm_hightemp();
        digitalWrite(0,0);

    }
    if(f>0)
    {
        qDebug() << "FIRE DETECTED ";
        digitalWrite(0,1);
        sensor_fireemailsend();
        SleeperThread::msleep(1000);
        write_port_gsm_fire();
        digitalWrite(0,0);
        fired="YES";
    }
    else
    {
        fired="NO";
    }
    if(s>3000)
    {
        qDebug() << "SMOKE DETECTED ";
        digitalWrite(0,1);
        sensor_smokeemailsend();
        SleeperThread::msleep(1000);
        write_port_gsm_smoke();
        digitalWrite(0,0);

    }

}

void CameraWindow::lcddata()
{
    lcdClear(lcd);
    lcdPosition(lcd,0,0);           //Position cursor on the first line in the first column
    char buf[30];
    sprintf(buf,"T=%d L=%d",t,l);
    lcdPuts(lcd,buf);
    lcdPosition(lcd,0,1);           //Position cursor on the first line in the first column
    sprintf(buf,"S=%d F=%d",s,f);
    lcdPuts(lcd,buf);

}

void CameraWindow::open_port()
{
    qDebug()<<"GSM PORT OPEN";
    this->port = new QextSerialPort(port_name_gsm, QextSerialPort::EventDriven);
    port->flush();
    port->setBaudRate(BAUD9600);
    port->setFlowControl(FLOW_OFF);
    port->setParity(PAR_NONE);
    port->setDataBits(DATA_8);
    port->setStopBits(STOP_1);

    if (port->open(QIODevice::ReadWrite) == true)
    {
        connect(port,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
    }
    else
    {
        QMessageBox::information(this, "SYSTEM ERROR "," Attempt to open a non-existent port");
    }
}

void CameraWindow::sensor_emailsend()
{

    qDebug()<<"Start login";
    SmtpClient smtp("smtp.gmail.com", 465, SmtpClient::SslConnection);
    smtp.setUser("ggeets41@gmail.com");
    smtp.setPassword("geeta4599");
    qDebug()<<"start Send email";
    MimeMessage message;
    EmailAddress sender("ggeets41@gmail.com", "Iot Device");
    message.setSender(&sender);
    EmailAddress to(sendemail, "User");
    message.addRecipient(&to);
    message.setSubject("Sensors Data");
    MimeText text;
    text.setText("Temperature:"+QString::number(t)+"\nLDR:"+QString::number(l)+"\nSMOKE:"+QString::number(s)+"\nFIRE:"+QString::number(f));
    message.addPart(&text);
    if (!smtp.connectToHost()) {
        qDebug() << "Failed to connect to host!" << endl;
        return;
    }
    if (!smtp.login()) {
        qDebug() << "Failed to login!" << endl;
        return;
    }
    if (!smtp.sendMail(message)) {
        qDebug() << "Failed to send mail!" << endl;
        return;
    }
    qDebug()<<"email is sending sucess";

    lcddata();
    smtp.quit();

     updatelink();


}

void CameraWindow::onReadyRead()
{
    SleeperThread::msleep(1000);

    QByteArray bytes= port->readAll();
    qDebug()<<bytes;

    for(int i=0;i<bytes.size();i++)
    {
        int j=0;

        if(bytes.at(i)=='+' && bytes.at(i+6)=='"' )
        {
            qDebug() << " PLUS CAME";
            recvnumb[0]=bytes.at(i+10);
            recvnumb[1]=bytes.at(i+11);
            recvnumb[2]=bytes.at(i+12);
            recvnumb[3]=bytes.at(i+13);
            recvnumb[4]=bytes.at(i+14);
            recvnumb[5]=bytes.at(i+15);
            recvnumb[6]=bytes.at(i+16);
            recvnumb[7]=bytes.at(i+17);
            recvnumb[8]=bytes.at(i+18);
            recvnumb[9]=bytes.at(i+19);
            qDebug() << "Receive Number" << recvnumb;
        }

        if(bytes.at(i)=='*')
        {
            do
            {
                num[j++]=bytes.at(++i);
            }while(bytes.at(i+1)!=':');
            j=0;
            i++;
            do
            {
                eail[j++]=bytes.at(++i);
            }while(bytes.at(i+1)!='#');

            qDebug()<< "Number" << num;
            qDebug()<<"EMAIL" << eail;

            dset=1;
        }
    }


    if(dset==1)
    {

        dset=0;

        QString setemail = eail;
        QString setnumber = num;

        lcdClear(lcd);
        lcdPosition(lcd,0,0);           //Position cursor on the first line in the first column
        lcdPuts(lcd, "MESSAGE RECVD..");  //Print the text on the LCD at the current cursor postion

        QSqlDatabase db=QSqlDatabase::addDatabase("QMYSQL");
        db.setHostName("localhost");
        db.setPort(3306);
        db.setDatabaseName("iotdevice");
        db.setUserName("iotmaster");
        db.setPassword("iotmotion");
        if (!db.open())
        {
            qDebug() << "Database error occurred"<<db.lastError();

        }
        else
        {
            qDebug()<<"Database connected";
            qDebug()<<db.databaseName();

        }



        QSqlQuery queryy;

        queryy.exec("
                    ATE `iotdevicedata` SET `mnumber`='"+setnumber+"',`eid`='"+setemail+"' WHERE 1");

        qDebug()<<"DONE";

        lcdClear(lcd);
        lcdPosition(lcd,0,0);           //Position cursor on the first line in the first column
        lcdPuts(lcd,"NUM:");  //Print the text on the LCD at the current cursor postion
        lcdPosition(lcd,0,4);           //Position cursor on the first line in the first column
        SleeperThread::msleep(1000);
        lcdClear(lcd);
        lcdPuts(lcd,num);  //Print the text on the LCD at the current cursor postion
        lcdPosition(lcd,0,0);           //Position cursor on the first line in the first column
        lcdPuts(lcd,eail);  //Print the text on the LCD at the current cursor postion

    }
}

void CameraWindow::fileread()
{

    qDebug()<<" File Read Function Called";

    QSqlDatabase db=QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setPort(3306);
    db.setDatabaseName("iotdevice");
    db.setUserName("iotmaster");
    db.setPassword("iotmotion");
    if (!db.open())
    {
        qDebug() << "Database error occurred"<<db.lastError();

    }
    else
    {
        qDebug()<<"Database connected";
        qDebug()<<db.databaseName();

    }

    QSqlQuery query;
    query.exec("select * from iotdevicedata where 1");
    while(query.next())
    {

        qDebug() << "Sno:" <<query.value(0).toInt() << "Mnumber:"<<query.value(1).toString() << "Email ID:"<<query.value(2).toString();

        sendnum=query.value(1).toByteArray();
        qDebug() << "Number: " << sendnum;
        sendemail=query.value(2).toString();
        qDebug() << "SEND EMAIL: " << sendemail;

    }

}



void CameraWindow::write_port_gsm_hightemp()
{
    qDebug() <<"SENDING HIGH TEMP SMS";
    qDebug() << "Number:" << sendnum;

    port->write("AT\r");
    SleeperThread::msleep(10);
    port->write("AT+CMGF=1\r");
    SleeperThread::msleep(10);
    port->write("AT+CMGS=");
    port->write("\"");
    port->write(sendnum);
    port->write("\"");
    SleeperThread::msleep(10);
    port->write("\r");
    SleeperThread::msleep(1000);
    port->write("High Temperature:");
    port->write(QByteArray::number(t));
    SleeperThread::msleep(100);
    port->write("\x1A");
    SleeperThread::msleep(5000);
}

void CameraWindow::write_port_gsm_smoke()
{
    qDebug() <<"SENDING  SMOKE SMS";
    port->write("AT\r");
    SleeperThread::msleep(10);
    port->write("AT+CMGF=1\r");
    SleeperThread::msleep(10);
    port->write("AT+CMGS=");
    port->write("\"");
    port->write(sendnum);
    port->write("\"");
    SleeperThread::msleep(10);
    port->write("\r");
    SleeperThread::msleep(1000);
    port->write("SMOKE DETECTED");
    SleeperThread::msleep(35);
    port->write("\x1A");
    SleeperThread::msleep(5000);
}

void CameraWindow::write_port_gsm_fire()
{
    qDebug() <<"SENDING  FIRE SMS";

    port->write("AT\r");
    SleeperThread::msleep(10);
    port->write("AT+CMGF=1\r");
    qDebug() << "2";
    SleeperThread::msleep(10);
    port->write("AT+CMGS=");
    port->write("\"");
    port->write(sendnum);
    port->write("\"");
    SleeperThread::msleep(10);
    port->write("\r");
    qDebug() << "3";
    SleeperThread::msleep(1000);
    port->write("FIRE DETECTED");
    SleeperThread::msleep(35);
    port->write("\x1A");
    SleeperThread::msleep(5000);
}

void CameraWindow::write_port_gsm_pir()
{
    qDebug() <<"SENDING  PIR SMS";

    port->write("AT\r");
    SleeperThread::msleep(10);
    port->write("AT+CMGF=1\r");
    SleeperThread::msleep(10);
    port->write("AT+CMGS=");
    port->write("\"");
    port->write(sendnum);
    port->write("\"");
    SleeperThread::msleep(10);
    port->write("\r");
    SleeperThread::msleep(1000);
    port->write("HUMAN DETECTED SENT MAIL");
    SleeperThread::msleep(35);
    port->write("\x1A");
    SleeperThread::msleep(5000);
}

void CameraWindow::updatelink()
{
    m_networkAccessManager->get(QNetworkRequest(QUrl("http://www.rpihealth.com/iot_motion/put_data.php?temp="+QByteArray::number(t)+"&smk="+QByteArray::number(s)+"&ldr="+QByteArray::number(l)+"&fir="+fired+"&pir="+pird+"")));

}

void CameraWindow::sensor_tempemailsend()
{

    qDebug()<<"Start login";
    SmtpClient smtp("smtp.gmail.com", 465, SmtpClient::SslConnection);
    smtp.setUser("ggeets41@gmail.com");
    smtp.setPassword("geeta4599");
    qDebug()<<"start Send email";
    MimeMessage message;
    EmailAddress sender("ggeets41@gmail.com", "Iot Device");
    message.setSender(&sender);
    EmailAddress to(sendemail, "User");
    message.addRecipient(&to);
    message.setSubject("HIGH TEMPERATURE DETECTED");
    MimeText text;
    text.setText("Temperature:"+QString::number(t));
    message.addPart(&text);
    if (!smtp.connectToHost()) {
        qDebug() << "Failed to connect to host!" << endl;
        return;
    }
    if (!smtp.login()) {
        qDebug() << "Failed to login!" << endl;
        return;
    }
    if (!smtp.sendMail(message)) {
        qDebug() << "Failed to send mail!" << endl;
        return;
    }
    qDebug()<<"email is sending sucess";


    lcddata();
    smtp.quit();
updatelink();


}

void CameraWindow::sensor_smokeemailsend()
{
    qDebug()<<"Start login";
    SmtpClient smtp("smtp.gmail.com", 465, SmtpClient::SslConnection);
    smtp.setUser("ggeets41@gmail.com");
    smtp.setPassword("geeta4599");
    qDebug()<<"start Send email";
    MimeMessage message;
    EmailAddress sender("ggeets41@gmail.com", "Iot Device");
    message.setSender(&sender);
    EmailAddress to(sendemail, "User");
    message.addRecipient(&to);
    message.setSubject("SMOKE DETECTED");
    MimeText text;
    text.setText("SMOKE DETECTED");
    message.addPart(&text);
    if (!smtp.connectToHost()) {
        qDebug() << "Failed to connect to host!" << endl;
        return;
    }
    if (!smtp.login()) {
        qDebug() << "Failed to login!" << endl;
        return;
    }
    if (!smtp.sendMail(message)) {
        qDebug() << "Failed to send mail!" << endl;
        return;
    }
    qDebug()<<"email is sending sucess";

    lcddata();
    smtp.quit();
    updatelink();
}

void CameraWindow::sensor_fireemailsend()
{
    qDebug()<<"Start login Fire Email";
    SmtpClient smtp("smtp.gmail.com", 465, SmtpClient::SslConnection);
    smtp.setUser("ggeets41@gmail.com");
    smtp.setPassword("geeta4599");
    qDebug()<<"start Send email";
    MimeMessage message;
    EmailAddress sender("ggeets41@gmail.com", "Iot Device");
    message.setSender(&sender);
    EmailAddress to(sendemail, "User");
    message.addRecipient(&to);
    message.setSubject("FIRE DETECTED");
    MimeText text;
    text.setText("FIRE DETECTED");
    message.addPart(&text);
    if (!smtp.connectToHost()) {
        qDebug() << "Failed to connect to host!" << endl;
        return;
    }
    if (!smtp.login()) {
        qDebug() << "Failed to login!" << endl;
        return;
    }
    if (!smtp.sendMail(message)) {
        qDebug() << "Failed to send mail!" << endl;
        return;
    }
    qDebug()<<"email is sending sucess";

    lcddata();
    smtp.quit();
    updatelink();
}
