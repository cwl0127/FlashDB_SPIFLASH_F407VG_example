/*
 * This file is part of the Serial Flash Universal Driver Library.
 *
 * Copyright (c) 2016, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2016-04-23
 */

#include <sfud.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
// #include <stm32f4xx_hal.h>
//#include <Arduino.h>
#include <SPI.h>

typedef struct
{
    // SPI_TypeDef *spix;
    // SPI_HandleTypeDef *spi_handle;
    // GPIO_TypeDef *cs_gpiox;
    // uint16_t cs_gpio_pin;
    SPIClass *spix;
    uint8_t cs_pin;
} spi_user_data, *spi_user_data_t;

static SPIClass spi3(PB5, PB4, PB3);
static SPISettings spiSetting(SPI_SPEED_CLOCK_DIV2_MHZ, MSBFIRST, SPI_MODE3);
static char log_buf[256];

#ifdef __cplusplus
extern "C" {
#endif
void sfud_log_debug(const char *file, const long line, const char *format, ...);
void sfud_log_info(const char *format, ...);
sfud_err sfud_spi_port_init(sfud_flash *flash);
#ifdef __cplusplus
}
#endif

// static SPI_HandleTypeDef hspi3;
// static void spi_configuration(spi_user_data_t spi)
// {
//     SPI_HandleTypeDef *spi_handle = spi->spi_handle;

//     spi_handle->Instance = spi->spix;
//     spi_handle->Init.Mode = SPI_MODE_MASTER;
//     spi_handle->Init.Direction = SPI_DIRECTION_2LINES;
//     spi_handle->Init.DataSize = SPI_DATASIZE_8BIT;
//     spi_handle->TxXferSize = 8;
//     spi_handle->RxXferSize = 8;
//     spi_handle->Init.CLKPhase = SPI_PHASE_2EDGE;
//     spi_handle->Init.CLKPolarity = SPI_POLARITY_HIGH;
//     spi_handle->Init.NSS = SPI_NSS_SOFT;
//     spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
//     spi_handle->Init.FirstBit = SPI_FIRSTBIT_MSB;
//     spi_handle->Init.TIMode = SPI_TIMODE_DISABLE;
//     spi_handle->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
//     spi_handle->State = HAL_SPI_STATE_RESET;
//     HAL_SPI_Init(spi_handle);
// }

static void spi_lock(const sfud_spi *spi)
{
    __disable_irq();
}

static void spi_unlock(const sfud_spi *spi)
{
    __enable_irq();
}

/**
 * SPI write data then read data
 */
static sfud_err spi_write_read(const sfud_spi *spi, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf,
                               size_t read_size)
{
    sfud_err result = SFUD_SUCCESS;
    spi_user_data_t spi_dev = (spi_user_data_t)spi->user_data;
    //HAL_StatusTypeDef state = HAL_OK;

    if (write_size)
    {
        SFUD_ASSERT(write_buf);
    }
    if (read_size)
    {
        SFUD_ASSERT(read_buf);
    }

    //HAL_GPIO_WritePin(spi_dev->cs_gpiox, spi_dev->cs_gpio_pin, GPIO_PIN_RESET);

    digitalWrite(spi_dev->cs_pin, LOW);
    if (write_size)
    {
        spi_dev->spix->transfer((void *)write_buf, write_size);
        // state = HAL_SPI_Transmit(spi_dev->spi_handle, (uint8_t *)write_buf, write_size, 1000);
        // while (HAL_SPI_GetState(spi_dev->spi_handle) != HAL_SPI_STATE_READY)
        //     ;
    }
    
    if (read_size)
    {
        memset((uint8_t *)read_buf, 0xFF, read_size);
        for (uint32_t i = 0; i < read_size; ++i)
        {
            read_buf[i] = spi_dev->spix->transfer(0xFF);
        }
        //spi_dev->spix->transfer((void *)write_buf, (void *)read_buf, read_size);
        // state = HAL_SPI_Receive(spi_dev->spi_handle, read_buf, read_size, 1000);
        // while (HAL_SPI_GetState(spi_dev->spi_handle) != HAL_SPI_STATE_READY)
        //     ;
    }

    //HAL_GPIO_WritePin(spi_dev->cs_gpiox, spi_dev->cs_gpio_pin, GPIO_PIN_SET);
    digitalWrite(spi_dev->cs_pin, HIGH);

    return result;
}

/* about 100 microsecond delay */
static void retry_delay_100us(void)
{
    uint32_t delay = 120;
    while (delay--)
        ;
}

static spi_user_data user_spi = {.spix = &spi3, .cs_pin = PC13};
sfud_err sfud_spi_port_init(sfud_flash *flash)
{
    sfud_err result = SFUD_SUCCESS;

    if (!strcmp(flash->spi.name, "spi3"))
    {
        /* SPI 外设初始化 */
        user_spi.spix->beginTransaction(spiSetting);
        pinMode(user_spi.cs_pin, OUTPUT);
        digitalWrite(user_spi.cs_pin, HIGH);
        /* 同步 Flash 移植所需的接口及数据 */
        flash->spi.wr = spi_write_read;
        flash->spi.lock = spi_lock;
        flash->spi.unlock = spi_unlock;
        flash->spi.user_data = (void *)&user_spi;
        /* about 100 microsecond delay */
        flash->retry.delay = retry_delay_100us;
        /* adout 60 seconds timeout */
        flash->retry.times = 60 * 10000;
    }

    return result;
}

/**
 * This function is print debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 */
void sfud_log_debug(const char *file, const long line, const char *format, ...)
{
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    Serial.printf("[SFUD](%s:%ld) ", file, line);
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    Serial.printf("%s\r\n", log_buf);
    va_end(args);
}

/**
 * This function is print routine info.
 *
 * @param format output format
 * @param ... args
 */
void sfud_log_info(const char *format, ...)
{
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    Serial.print("[SFUD]");
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    Serial.printf("%s\r\n", log_buf);
    va_end(args);
}
