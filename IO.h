#ifndef IO_H
#define IO_H

#define DIO_NUM(x, y) (((x) << 8) + (y))
#define DIO_NUM_PORT(x) (((x)&0xFFFF) >> 8)
#define DIO_NUM_PIN(x) ((x)&0xFF)


void Dio_init();
void Dio_WriteChannel(int channelID,int Level);
int Dio_ReadChannel(int channelID);

void Adc_Init();
//void Adc_SetupResultBuffer();

int get_adc(int channelID);

#endif