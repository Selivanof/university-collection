/*!
 * \file      dht.h
 * \brief     Implements a interface for the DHT11 sensor.
 *
 * Exposes a function to read temperature and humidity data from an DHT11 sensor
 */

#ifndef DHT_H
#define DHT_H

#include <stdint.h>

#include "platform.h"

/*! \brief Reads temperature and humidity data from the DHT11 sensor.
 *  \param pin   The pin of the DHT11 sensor.
 *  \param temperature   Pointer to a float where the temperature data will be stored.
 *  \param humidity   Pointer to a float where the humidity data will be stored.
 *  Returns 1 if the data read was successfully, 0 if there was an error.
 */
uint8_t ReadDHT11(Pin pin, float *temperature, float *humidity);

#endif  // DHT_H