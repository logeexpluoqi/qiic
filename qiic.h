/**
 * @ Author: luoqi
 * @ Create Time: 2025-03-18 14:57
 * @ Modified by: luoqi
 * @ Modified time: 2025-03-19 16:33
 * @ Description:
 */

#ifndef _QIIC_H_
#define _QIIC_H_

#include <stdint.h>

typedef enum {
    QIIC_PIN_STATE_LOW = 0,
    QIIC_PIN_STATE_HIGH = 1,
} QI2cPinState;

typedef int (*QI2cSckSetFunc)(QI2cPinState state);  // Function pointer to set the SCK pin state
typedef int (*QI2cSdaSetFunc)(QI2cPinState state);  // Function pointer to set the SDA pin state
typedef QI2cPinState (*QI2cSdaGetFunc)(void);       // Function pointer to get the SDA pin state
typedef void (*QI2cDelayUsFunc)(uint32_t us);       // Function pointer to delay in microseconds

typedef struct {
    uint32_t tick;                       // Delay in microseconds for one clock cycle
    int (*sck_set)(QI2cPinState state);     // Function to set the SCK pin state
    int (*sda_set)(QI2cPinState state);     // Function to set the SDA pin state
    QI2cPinState (*sda_get)(void);          // Function to get the SDA pin state
    void (*delay_us)(uint32_t us);          // Function to delay in microseconds
} QI2cObj;

/**
 * @brief Initialize the QI2cObj structure with provided parameters
 * 
 * @param obj Pointer to the QI2cObj structure to be initialized
 * @param clk_frq I2C clock frequency in Hz
 * @param sck_set Function to set the SCK pin state
 * @param sda_set Function to set the SDA pin state
 * @param sda_get Function to get the SDA pin state
 * @param delay_us Function to delay in microseconds
 * @return int 0 on success, non-zero on failure
 */
int qiic_init(QI2cObj *obj, uint32_t clk_frq, QI2cSckSetFunc sck_set, QI2cSdaSetFunc sda_set, QI2cSdaGetFunc sda_get, QI2cDelayUsFunc delay_us);

/**
 * @brief Send data to an I2C device
 * 
 * @param obj Pointer to the QI2cObj structure
 * @param dev_addr I2C device address
 * @param buf Pointer to the buffer containing data to send
 * @param sz Number of bytes to send
 * @return int 0 on success, non-zero on failure
 */
int qiic_send(QI2cObj *obj, uint8_t dev_addr, uint8_t *buf, int sz);

/**
 * @brief Receive data from an I2C device
 * 
 * @param obj Pointer to the QI2cObj structure
 * @param dev_addr I2C device address
 * @param buf Pointer to the buffer to store received data
 * @param sz Number of bytes to receive
 * @return int 0 on success, non-zero on failure
 */
int qiic_recv(QI2cObj *obj, uint8_t dev_addr, uint8_t *buf, int sz);

/**
 * @brief Write data to a memory-mapped register of an I2C device
 * 
 * @param obj Pointer to the QI2cObj structure
 * @param dev_addr I2C device address
 * @param mem_addr Memory address in the device to write to
 * @param buf Pointer to the buffer containing data to write
 * @param sz Number of bytes to write
 * @return int 0 on success, non-zero on failure
 */
int qiic_mem_write(QI2cObj *obj, uint8_t dev_addr, uint8_t mem_addr, uint8_t *buf, int sz);

/**
 * @brief Read data from a memory-mapped register of an I2C device
 * 
 * @param obj Pointer to the QI2cObj structure
 * @param dev_addr I2C device address
 * @param mem_addr Memory address in the device to read from
 * @param buf Pointer to the buffer to store received data
 * @param sz Number of bytes to read
 * @return int 0 on success, non-zero on failure
 */
int qiic_mem_read(QI2cObj *obj, uint8_t dev_addr, uint8_t mem_addr, uint8_t *buf, int sz);

#endif
