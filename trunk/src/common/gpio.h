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

#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif


/* GPIO value */
#define GPIO_HIGH     1
#define GPIO_LOW      0

/* GPIO direction code */
#define GPIO_INPUT       0
#define GPIO_OUTPUT      1
#define GPIO_OUTPUT_HIGH 2
#define GPIO_OUTPUT_LOW  3

/* GPIO error code */
#define GPIO_SUCCESS            1
#define GPIO_INVALID_RESOURCE  -1
#define GPIO_ERROR_READ        -2
#define GPIO_ERROR_WRITE       -3
#define GPIO_ERROR_DIRECTION   -4


/**
 * Read specified GPIO pin
 *
 * @param pin_number specified the pin
 *
 * @return integer value of GPIO if success.
 *  return negative integer error code if fail.
 */
int  gpio_read(int pin_number);



/**
 * Write specified GPIO pin
 *
 * @param pin_number specified the pin
 *
 * @param value could be GPIO_HIGH or GPIO_LOW
 *
 * @return 1 if success.
 *  return negative integer error code if fail.
 */
int  gpio_write(int pin_number, int value);


/**
 * set direction of specified GPIO pin
 *
 * @param pin_number specified the pin
 *
 * @param direction could be GPIO_IN or GPIO_OUT, GPIO_OUT_HIGH, GPIO_OUT_LOW
 *
 * @return 1 if success.
 *  return negative integer error code if fail.
 */
int  gpio_set_direction(int pin_number, int direction);

/**
 * get direction of specified GPIO pin
 *
 * @param pin_number specified the pin
 *
 * @return positive integer direction code if success.
 *  return negative integer error code if fail.
 */
int  gpio_get_direction(int pin_number);


/**
 * export specified GPIO pin
 *
 * @param pin_number specified the pin
 *
 * @return 1 if success.
 *  return negative integer error code if fail.
 */
int gpio_export(int pin_number);


/**
 * unexport specified GPIO pin
 *
 * @param pin_number specified the pin
 *
 * @return 1 if success.
 *  return negative integer error code if fail.
 */
int gpio_unexport(int pin_number);

/**
 * Return 1 if specified GPIO pin is exported
 *
 * @param pin_number specified the pin
 *
 * @return 1 if GPIO pin is exported.
 *  return 0 if GPIO pin is not exported
 */
int gpio_is_exported(int pin_number);



#ifdef __cplusplus
}
#endif


#endif /* __GPIO_H__ */
