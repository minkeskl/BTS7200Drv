#ifndef IO_H
#define IO_H

void Dio_init();
void Dio_WriteChannel(int channelID,int Level);
int Dio_ReadChannel(int channelID);

void Adc_Init();
//void Adc_SetupResultBuffer();

int get_adc(int channelID);

#endif