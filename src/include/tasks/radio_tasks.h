#ifndef RADIO_TASKS_H
#define RADIO_TASKS_H

void initRadio();

void initRadioTask(void *pvParameters);

void baseRadioRX(void *pvParameters);

void baseRadioTX(void *pvParameters);

#endif
