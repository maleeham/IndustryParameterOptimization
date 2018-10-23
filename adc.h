#ifndef ADC_H
#define ADC_H


#include <QtGui>
class adc: public QWidget
{
    Q_OBJECT

public:
      adc(QWidget *parent=0);
int read_mcp3208_adc(unsigned char);

    

    
};

#endif // ADC_H
