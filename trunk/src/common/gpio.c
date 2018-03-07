/*
 *  A library for Linux GPIO operation
 *
 *  Author: JoStudio, 2016
 *
 *  Version: 0.5
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "gpio.h"

#define FILENAME_SIZE  128

#define GPIO_PATH "/sys/class/gpio"

/**
 * Export specified GPIO pin
 *
 * @param pin_number specified the pin
 *
 * @return 1 if success.
 *  return negative integer error code if fail.
 */
int gpio_export(int pin_number) {
	char buf[FILENAME_SIZE];
	int  fp;
	int  length;

	if ( gpio_is_exported(pin_number) )
		return GPIO_SUCCESS;

	//write to export file
	fp = open(GPIO_PATH "/export", O_WRONLY);
	if ( fp == -1) {
		return GPIO_INVALID_RESOURCE;
	}

	length = snprintf(buf, sizeof(buf), "%d", pin_number);
	if (write(fp, buf, length * sizeof(char)) == -1) {
		close(fp);
		return GPIO_INVALID_RESOURCE;
	}

	close(fp);
	return gpio_is_exported(pin_number);
}


/**
 * Return 1 if specified GPIO pin is exported
 *
 * @param pin_number specified the pin
 *
 * @return 1 if GPIO pin is exported.
 *  return 0 if GPIO pin is not exported
 */
int gpio_is_exported(int pin_number) {
	char  buf[FILENAME_SIZE];
	struct stat dir;

	//check if the gpioXX directory exists
	snprintf(buf, sizeof(buf), GPIO_PATH "/gpio%d/", pin_number);
	if (stat(buf, &dir) == 0 && S_ISDIR(dir.st_mode)) {
		return GPIO_SUCCESS;
	} else {
		return 0;
	}
}


/**
 * unexport specified GPIO pin
 *
 * @param pin_number specified the pin
 *
 * @return 1 if success.
 *  return negative integer error code if fail.
 */
int gpio_unexport(int pin_number) {
	char buf[FILENAME_SIZE];
	int  fp;
	int  length;

	//write to unexport file
	fp = open(GPIO_PATH "/unexport", O_WRONLY);
	if ( fp == -1) {
		return GPIO_INVALID_RESOURCE;
	}

	length = snprintf(buf, sizeof(buf), "%d", pin_number);
	if (write(fp, buf, length * sizeof(char)) == -1) {
		close(fp);
		return GPIO_INVALID_RESOURCE;
	}

	close(fp);
	return GPIO_SUCCESS;
}


/**
 * Read specified GPIO pin
 *
 * @param pin_number specified the pin
 *
 * @return integer value of GPIO if success.
 *  return negative integer error code if fail.
 */
int  gpio_read(int pin_number) {
	char buf[FILENAME_SIZE];
	int  fp;

	if ( !gpio_is_exported(pin_number) ) {
		if ( 1 != gpio_export(pin_number) )
			return GPIO_INVALID_RESOURCE;
	}

	//read value file
	snprintf(buf, sizeof(buf),  "%s/gpio%d/value", GPIO_PATH, pin_number);
	fp = open(buf, O_RDWR);
	if ( fp == -1) {
		return GPIO_INVALID_RESOURCE;
	}

	if ( lseek(fp, 0, SEEK_SET) < 0 ) {
		close(fp);
	}


	if (read(fp, buf, 2 * sizeof(char)) != 2) {
		close(fp);
		return GPIO_ERROR_READ;
	}

	close(fp);

	return (int) strtol(buf, NULL, 10);
}


/**
 * Write specified GPIO pin
 *
 * @param pin_number specified the pin
 *
 * @param value  could be GPIO_HIGH or GPIO_LOW
 *
 * @return 1 if success.
 *  return negative integer error code if fail.
 */
int  gpio_write(int pin_number, int value) {
	char buf[FILENAME_SIZE];
	int  fp;
	int  length;


	if ( !gpio_is_exported(pin_number) ) {
		if ( 1 != gpio_export(pin_number) )
			return GPIO_INVALID_RESOURCE;
	}

	//write to value file
	snprintf(buf, sizeof(buf),  "%s/gpio%d/value", GPIO_PATH, pin_number);
	fp = open(buf, O_RDWR);
	if ( fp == -1) {
		return GPIO_INVALID_RESOURCE;
	}

	if ( lseek(fp, 0, SEEK_SET) < 0 ) {
		close(fp);
	}

	length = snprintf(buf, sizeof(buf), "%d", value);
	if ( write(fp, buf, length * sizeof(char)) == -1) {
		close(fp);
		return GPIO_ERROR_WRITE;
	}

	close(fp);

	return GPIO_SUCCESS;
}


/**
 * set direction of specified GPIO pin
 *
 * @param pin_number specified the pin
 *
 * @param direction could be GPIO_INPUT or GPIO_OUTPUT, GPIO_OUTPUT_HIGH, GPIO_OUTPUT_LOW
 *
 * @return 1 if success.
 *  return negative integer error code if fail.
 */
int  gpio_set_direction(int pin_number, int direction) {
	char buf[FILENAME_SIZE];
	int  fp;
	int  length;

	if ( !gpio_is_exported(pin_number) ) {
		if ( 1 != gpio_export(pin_number) )
			return GPIO_INVALID_RESOURCE;
	}

	//write direction file
	snprintf(buf, sizeof(buf),  "%s/gpio%d/direction", GPIO_PATH, pin_number);
	fp = open(buf, O_RDWR);
	if ( fp == -1) {
		return GPIO_INVALID_RESOURCE;
	}

	if ( lseek(fp, 0, SEEK_SET) < 0 ) {
		close(fp);
		return GPIO_INVALID_RESOURCE;
	}

	switch (direction) {
		case GPIO_OUTPUT:
			length = snprintf(buf, sizeof(buf), "out");
			break;
		case GPIO_INPUT:
			length = snprintf(buf, sizeof(buf), "in");
			break;
		case GPIO_OUTPUT_HIGH:
			length = snprintf(buf, sizeof(buf), "high");
			break;
		case GPIO_OUTPUT_LOW:
			length = snprintf(buf, sizeof(buf), "low");
			break;
		default:
			close(fp);
			return GPIO_ERROR_DIRECTION;
	}

	if (write(fp, buf, length * sizeof(char)) == -1) {
		close(fp);
		return GPIO_ERROR_WRITE;
	}

	close(fp);

	return GPIO_SUCCESS;
}


/**
 * get direction of specified GPIO pin
 *
 * @param pin_number specified the pin
 *
 * @return positive integer direction code if success.
 *  return negative integer error code if fail.
 */
int  gpio_get_direction(int pin_number) {
	char buf[FILENAME_SIZE];
	int  fp;
	int  length;
	int  result;

	if ( !gpio_is_exported(pin_number) ) {
		if ( 1 != gpio_export(pin_number) )
			return GPIO_INVALID_RESOURCE;
	}

	//read direction file
	snprintf(buf, sizeof(buf),  "%s/gpio%d/direction", GPIO_PATH, pin_number);
	fp = open(buf, O_RDONLY);
	if ( fp == -1) {
		return GPIO_INVALID_RESOURCE;
	}

	if ( lseek(fp, 0, SEEK_SET) < 0 ) {
		close(fp);
		return GPIO_INVALID_RESOURCE;
	}

	memset(buf, '\0', sizeof(buf));
	length = read(fp, buf, sizeof(buf));
	close(fp);

	if (length <= 0) {
		return GPIO_INVALID_RESOURCE;
	}

	if (strncmp(buf, "out", 3) == 0) {
		result = GPIO_OUTPUT;
	} else if (strncmp(buf, "in", 2) == 0) {
		result = GPIO_INPUT;
	} else if (strncmp(buf, "high", 4) == 0) {
		result = GPIO_OUTPUT_HIGH;
	} else if (strncmp(buf, "low", 3) == 0) {
		result = GPIO_OUTPUT_LOW;
	} else {
		result = GPIO_ERROR_DIRECTION;
	}

	return result;

}



