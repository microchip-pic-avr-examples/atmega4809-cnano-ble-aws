/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
 */

#include <stdbool.h>
#include "bme280.h"
#include "weather.h"
#include "delay.h"

/**
  Section: Macro Declarations
 */


#define DEFAULT_STANDBY_TIME    BME280_STANDBY_10MS
#define DEFAULT_FILTER_COEFF    BME280_FILTER_COEFF_OFF
#define DEFAULT_TEMP_OSRS       BME280_OVERSAMP_X1
#define DEFAULT_PRESS_OSRS      BME280_OVERSAMP_X1
#define DEFAULT_HUM_OSRS        BME280_OVERSAMP_X1
#define DEFAULT_SENSOR_MODE     BME280_FORCED_MODE

/**
  Section: Variable Definitions
 */

static bool weather_initialized = false;

/**
  Section: Private functions
 */

static void Weather_initializeClick(void)
{
    BME280_reset();
    
    DELAY_milliseconds(5);    //Startup delay for BME280 sensor
    
    BME280_readFactoryCalibrationParams();

    BME280_setStandbyTime(DEFAULT_STANDBY_TIME);
    BME280_setFilterCoefficient(DEFAULT_FILTER_COEFF);
    BME280_setOversamplingTemperature(DEFAULT_TEMP_OSRS);
    BME280_setOversamplingPressure(DEFAULT_PRESS_OSRS);
    BME280_setOversamplingHumidity(DEFAULT_HUM_OSRS);
    BME280_setSensorMode(DEFAULT_SENSOR_MODE);
    BME280_initializeSensor();

    weather_initialized = true;
}

/**
  Section: Driver APIs
 */

void Weather_readSensors(void)
{
    if (!weather_initialized)
    {
        Weather_initializeClick();
    }

    if (DEFAULT_SENSOR_MODE == BME280_FORCED_MODE)
    {
        BME280_startForcedSensing();
    }

    while(BME280_isMeasuring())
    {
        // Wait until the results have been transferred to the data registers
    }
    BME280_readMeasurements();
}

float Weather_getTemperatureDegC(void)
{
    float volatile temperature;
    temperature = BME280_getTemperature();
    return temperature;
}

float Weather_getPressureKPa(void)
{
    return BME280_getPressure();
}

float Weather_getHumidityRH(void)
{
    return BME280_getHumidity();
}

void Weather_gotoSleep(void)
{
    BME280_sleep();
    weather_initialized = false;
}
