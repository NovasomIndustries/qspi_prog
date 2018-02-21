//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+ gpio.h
//+
//+
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define IN  0
#define OUT 1

#define LOW  0
#define HIGH 1

#define BUFFER_MAX 4
#define VALUE_MAX 30
#define DIRECTION_MAX 40

#define DELAY   2000



int GPIOExport(int port, int pin);
int GPIOUnexport(int port, int pin);
int GPIODirection(int port, int pin, int dir);
int GPIORead(int port, int pin);
int GPIOWrite(int port, int pin, int value);
