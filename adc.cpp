#include "adc.h"
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <QMessageBox>
#include <QDebug>
#define SPI_CHANNEL 0
#define SPI_SPEED   100000
//#define CS_MCP3208  8
adc::adc(QWidget *parent):QWidget(parent)
{
   // qDebug()<<"ADC Init:";
//    pinMode(CS_MCP3208, 1);
    if(wiringPiSetup()==-1)
    {
         QMessageBox::information(this,"WiringPi","Unable to start wiringPi");
    }
     if(wiringPiSPISetup(SPI_CHANNEL,SPI_SPEED)==-1)
     {
          QMessageBox::information(this,"WiringPiSPI","wiringPiSPISetup Faild");
     }
  //   pinMode(CS_MCP3208, 1);

}
int adc::read_mcp3208_adc(unsigned char adcChannel)
{
  //  qDebug()<<"Adc Read";
  unsigned char buff[3];
  int adcValue = 0;

  buff[0] = 0x06 | ((adcChannel & 0x07) >> 7);
  buff[1] = ((adcChannel & 0x07) << 6);
  buff[2] = 0x00;

 // digitalWrite(CS_MCP3208, 0);  // Low : CS Active

  wiringPiSPIDataRW(SPI_CHANNEL, buff, 3);

  buff[1] = 0x0F & buff[1];
  adcValue = ( buff[1] << 8) | buff[2];

 // digitalWrite(CS_MCP3208, 1);  // High : CS Inactive

  return adcValue;
}
