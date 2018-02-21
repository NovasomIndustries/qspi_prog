//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+ gpio.c
//+
//+
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "gpio.h"












//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+
//+
//+
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int GPIOExport(int port, int pin)
{
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;

	int chip_pin = (32 * (port - 1)) + pin;

    //usleep(DELAY);

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open export for writing!\n");
		return(-1);
	}

	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", chip_pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+
//+
//+
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int GPIOUnexport(int port, int pin)
{
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;

    int chip_pin = (32 * (port - 1)) + pin;

	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open unexport for writing!\n");
		return(-1);
	}

	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", chip_pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+
//+
//+
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int GPIODirection(int port, int pin, int dir)
{
	static const char s_directions_str[]  = "in\0out";
    int chip_pin = (32 * (port - 1)) + pin;

    //printf("chip_pin: %i\r\n", chip_pin);
    //usleep(DELAY);

	char path[DIRECTION_MAX];
	int fd;

	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", chip_pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio direction for writing!\n");
		return(-1);
	}

	if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
		fprintf(stderr, "Failed to set direction!\n");
		return(-1);
	}

	close(fd);
	return(0);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+
//+
//+
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int GPIORead(int port, int pin)
{

	char path[VALUE_MAX];
	char value_str[3];
	int fd;

	int chip_pin = (32 * (port - 1)) + pin;

    //usleep(DELAY);

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", chip_pin);
	fd = open(path, O_RDONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio value for reading!\n");
		return(-1);
	}

	if (-1 == read(fd, value_str, 3)) {
		fprintf(stderr, "Failed to read value!\n");
		return(-1);
	}

	close(fd);

	return(atoi(value_str));
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+
//+
//+
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int GPIOWrite(int port, int pin, int value)
{
	static const char s_values_str[] = "01";

	char path[VALUE_MAX];
	int fd;
	int chip_pin = (32 * (port - 1)) + pin;

    //usleep(DELAY);

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", chip_pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio value for writing!\n");
		return(-1);
	}

	if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
		fprintf(stderr, "Failed to write value!\n");
		return(-1);
	}

	close(fd);
	return(0);

}
