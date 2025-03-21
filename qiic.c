/**
 * @ Author: luoqi
 * @ Create Time: 2025-03-18 14:56
 * @ Modified by: luoqi
 * @ Modified time: 2025-03-19 16:30
 * @ Description:
 */

#include "qiic.h"

typedef enum {
    QIIC_ACK = 0,
    QIIC_NAK = 1,
} QI2cAck;

static inline void _iic_start(QI2cObj *obj)
{
    obj->sck_set(QIIC_PIN_STATE_HIGH);
    obj->sda_set(QIIC_PIN_STATE_HIGH);
    obj->delay_us(obj->tick);
    obj->sda_set(QIIC_PIN_STATE_LOW);
    obj->delay_us(obj->tick);
    obj->sck_set(QIIC_PIN_STATE_LOW);
    obj->delay_us(obj->tick);
}

static inline void _iic_stop(QI2cObj *obj)
{
    obj->sda_set(QIIC_PIN_STATE_LOW);
    obj->sck_set(QIIC_PIN_STATE_LOW);
    obj->delay_us(obj->tick);
    obj->sck_set(QIIC_PIN_STATE_HIGH);
    obj->delay_us(obj->tick);
    obj->sda_set(QIIC_PIN_STATE_HIGH);
    obj->delay_us(obj->tick);
}

static inline void _iic_ack(QI2cObj *obj)
{
    obj->sck_set(QIIC_PIN_STATE_LOW);
    obj->sda_set(QIIC_PIN_STATE_LOW);
    obj->delay_us(obj->tick);
    obj->sck_set(QIIC_PIN_STATE_HIGH);
    obj->delay_us(obj->tick);
    obj->sck_set(QIIC_PIN_STATE_LOW);
}

static inline void _iic_nak(QI2cObj *obj)
{
    obj->sck_set(QIIC_PIN_STATE_LOW);
    obj->sda_set(QIIC_PIN_STATE_HIGH);
    obj->delay_us(obj->tick);
    obj->sck_set(QIIC_PIN_STATE_HIGH);
    obj->delay_us(obj->tick);
    obj->sck_set(QIIC_PIN_STATE_LOW);
}

static inline QI2cAck _iic_wait_ack(QI2cObj *obj)
{
    obj->sda_set(QIIC_PIN_STATE_HIGH);
    obj->delay_us(obj->tick);
    obj->sck_set(QIIC_PIN_STATE_HIGH);
    obj->delay_us(obj->tick);
    QI2cAck ack = (obj->sda_get() == QIIC_PIN_STATE_LOW) ? QIIC_ACK : QIIC_NAK;
    obj->sck_set(QIIC_PIN_STATE_LOW);
    return ack;
}


static inline int _iic_send_byte(QI2cObj *obj, uint8_t byte)
{
    obj->sck_set(QIIC_PIN_STATE_LOW);
    for(int i = 0; i < 8; i++) {
        if(byte & 0x80) {
            obj->sda_set(QIIC_PIN_STATE_HIGH);
        } else {
            obj->sda_set(QIIC_PIN_STATE_LOW);
        }
        obj->delay_us(obj->tick);
        obj->sck_set(QIIC_PIN_STATE_HIGH);
        obj->delay_us(obj->tick);
        obj->sck_set(QIIC_PIN_STATE_LOW);
        obj->delay_us(obj->tick);
        byte <<= 1;
    }
    return 1;
}

static inline int _iic_recv_byte(QI2cObj *obj, uint8_t *byte)
{
    uint8_t recv = 0;
    for(int i = 0; i < 8; i++) {
        obj->sck_set(QIIC_PIN_STATE_LOW);
        obj->delay_us(obj->tick);
        obj->sck_set(QIIC_PIN_STATE_HIGH);
        recv <<= 1;
        if(obj->sda_get() == QIIC_PIN_STATE_HIGH) {
            recv |= 0x01;
        }
        obj->delay_us(obj->tick);
    }
    *byte = recv;
    return 1;
}

int qiic_init(QI2cObj *obj, uint32_t clk_frq, QI2cSckSetFunc sck_set, QI2cSdaSetFunc sda_set, QI2cSdaGetFunc sda_get, QI2cDelayUsFunc delay_us)
{
    if(!obj || !sck_set || !sda_set || !sda_get || !delay_us) {
        return -1;
    }
    obj->sck_set = sck_set;
    obj->sda_set = sda_set;
    obj->sda_get = sda_get;
    obj->delay_us = delay_us;
    obj->tick = (1000000 / clk_frq) / 4;

    obj->sck_set(QIIC_PIN_STATE_HIGH);
    obj->sda_set(QIIC_PIN_STATE_HIGH);
    return 0;
}

int qiic_send(QI2cObj *obj, uint8_t dev_addr, uint8_t *buf, int sz)
{
    if(!obj || !buf || sz == 0) {
        return -1;
    }
    _iic_start(obj);
    _iic_send_byte(obj, dev_addr & 0xFE);
    if(_iic_wait_ack(obj) != QIIC_ACK) {
        _iic_stop(obj);
        return -1;
    }
    for(int i = 0; i < sz; i++) {
        _iic_send_byte(obj, buf[i]);
        if(_iic_wait_ack(obj) != QIIC_ACK) {
            _iic_stop(obj);
            return -1;
        }
    }
    _iic_stop(obj);
    return sz;
}

int qiic_recv(QI2cObj *obj, uint8_t dev_addr, uint8_t *buf, int sz)
{
    if(!obj || !buf || sz == 0) {
        return -1;
    }
    _iic_start(obj);
    _iic_send_byte(obj, dev_addr | 0x01);
    if(_iic_wait_ack(obj) != QIIC_ACK) {
        _iic_stop(obj);
        return -1;
    }
    for(int i = 0; i < sz; i++) {
        _iic_recv_byte(obj, &buf[i]);
        if(i != sz - 1) {
            _iic_ack(obj);
        } else {
            _iic_nak(obj);
        }
    }
    _iic_stop(obj);

    return sz;
}

int qiic_mem_write(QI2cObj *obj, uint8_t dev_addr, uint8_t mem_addr, uint8_t *buf, int sz)
{
    if(!obj || !buf || sz == 0) {
        return -1;
    }
    _iic_start(obj);
    _iic_send_byte(obj, dev_addr & 0xFE);
    if(_iic_wait_ack(obj) != QIIC_ACK) {
        _iic_stop(obj);
        return -1;
    }
    _iic_send_byte(obj, mem_addr);
    if(_iic_wait_ack(obj) != QIIC_ACK) {
        _iic_stop(obj);
        return -2;
    }
    for(int i = 0; i < sz; i++) {
        _iic_send_byte(obj, buf[i]);
        if(_iic_wait_ack(obj) != QIIC_ACK) {
            _iic_stop(obj);
            return -3;
        }
    }
    _iic_stop(obj);
    return sz;
}

int qiic_mem_read(QI2cObj *obj, uint8_t dev_addr, uint8_t mem_addr, uint8_t *buf, int sz)
{
    if(!obj || !buf || sz == 0) {
        return -1;
    }
    _iic_start(obj);
    _iic_send_byte(obj, dev_addr & 0xFE);
    if(_iic_wait_ack(obj) != QIIC_ACK) {
        _iic_stop(obj);
        return -1;
    }
    _iic_send_byte(obj, mem_addr);
    if(_iic_wait_ack(obj) != QIIC_ACK) {
        _iic_stop(obj);
        return -2;
    }
    int ret = qiic_recv(obj, dev_addr | 0x01, buf, sz);
    if(ret != sz) {
        return -3;
    }
    return ret;
}
