#ifndef __SCCB_H__
#define __SCCB_H__

#include <stdint.h>

int SCCB_Init(int pin_sda, int pin_scl);
int SCCB_Deinit(void);
uint16_t SCCB_Read(uint8_t slv_addr, uint8_t reg);
uint8_t SCCB_Write(uint8_t slv_addr, uint8_t reg, uint16_t data);
#endif // __SCCB_H__
