#ifndef _IIC_H
#define _IIC_H

unsigned char read_24(unsigned char add);
void write_24(unsigned char add,unsigned char dat);

void write_adc(unsigned char add);
unsigned char read_adc(unsigned char add);

#endif