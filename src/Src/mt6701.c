#include "soft_i2c.h"
#include "stm32f1xx_hal.h"

#define TARGET_PPR             100   // 100 PPR
#define TARGET_DIR             1     // 1 - CW, 0 - CCW 

// Calculated reference value for resolution registers (PPR - 1)
#define TARGET_ABZ_RES         (TARGET_PPR - 1) 

#define MODE_LOW()     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET)
#define MODE_HIGH()    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET)

// MT6701 I2C Slave Address (7-bit: 0x06. Shifted left: Write = 0x0C, Read = 0x0D)
#define MT6701_I2C_ADDR_WRITE  0x0C
#define MT6701_I2C_ADDR_READ   0x0D

// Register Addresses
#define REG_DIR_AABZ           0x29
#define REG_ABZ_RES_H          0x30
#define REG_ABZ_RES_L          0x31
#define REG_DSP_RESET          0x3A  // System Soft Reset register

// NVM Programming registers (I2C programming space)
#define REG_NVM_UNLOCK         0x09  // Authorization register (Unlock Key)
#define REG_NVM_CMD            0x0A  // Programmer command register

// MagnTek commands and access codes
#define NVM_UNLOCK_KEY         0xB3  // Key to unlock EEPROM programming
#define NVM_CMD_BURN           0x05  // Physical EEPROM burning command


void mt6701_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);    
        
    MODE_HIGH();  
    soft_i2c_init();
}

void mt6701_deinit(void)
{
   
   soft_i2c_deinit();
   MODE_LOW(); 

}

/**
 * Read a single byte from a specific register
 */
unsigned char mt6701_read_reg(unsigned char reg_addr) {
    unsigned char data = 0;
    
    soft_i2c_start();
    soft_i2c_write(MT6701_I2C_ADDR_WRITE);
    soft_i2c_write(reg_addr);
    
    soft_i2c_start(); // Repeated Start condition
    soft_i2c_write(MT6701_I2C_ADDR_READ);
    data = soft_i2c_read(0);
    soft_i2c_stop();
    
    return data;
}

/**
 * Write a single byte to a specific register
 */
void mt6701_write_reg(unsigned char reg_addr, unsigned char data) {
    soft_i2c_start();
    soft_i2c_write(MT6701_I2C_ADDR_WRITE);
    soft_i2c_write(reg_addr);
    soft_i2c_write(data);
    soft_i2c_stop();
}

/**
 * Configure MT6701 encoder parameters in RAM during startup
 * @param ppr Desired Pulses Per Revolution (range: 1 to 1024)
 * @param dir Rotation direction: 0 = CCW (Counter-Clockwise), 1 = CW (Clockwise)
 */
void mt6701_configure(uint16_t ppr, uint8_t dir) 
{
    // Sanity check to match MT6701 hardware specifications
    if (ppr < 1) ppr = 1;
    if (ppr > 1024) ppr = 1024;
    
    // 1. Configure rotation direction (DIR) in register 0x29 [bit 1]
    unsigned char reg29 = mt6701_read_reg(REG_DIR_AABZ);
    if (dir) {
        reg29 |= (1 << 1);  // Set bit 1 (DIR = 1, CW)
    } else {
        reg29 &= ~(1 << 1); // Clear bit 1 (DIR = 0, CCW)
    }
    mt6701_write_reg(REG_DIR_AABZ, reg29);
    
    // Calculate the value for the resolution register: VAL = PPR - 1
    uint16_t abz_res = ppr - 1;
    
    // 2. Configure MSB of ABZ resolution in register 0x30 [bits 1:0]
    unsigned char reg30 = mt6701_read_reg(REG_ABZ_RES_H);
    reg30 &= 0xFC; // Clear 2 lowest bits (11111100b) to preserve UVW configuration
    reg30 |= ((abz_res >> 8) & 0x03); // Extract bits 9 and 8 from abz_res
    mt6701_write_reg(REG_ABZ_RES_H, reg30);
    
    // 3. Configure LSB of ABZ resolution in register 0x31 [bits 7:0]
    unsigned char reg31 = (unsigned char)(abz_res & 0xFF);
    mt6701_write_reg(REG_ABZ_RES_L, reg31);
    
    // 4. Trigger Internal DSP Reset to reload configuration into encoder block   
    mt6701_write_reg(REG_DSP_RESET, 0x80);
    
}


/**
 * Verify current MT6701 registers and permanently burn EEPROM if settings mismatch.
 * Execute this while MODE pin is HIGH (PB15 = 1) and VDD = 5V.
 */
void mt6701_eeprom_check_and_burn(void) 
{

     HAL_Delay(1000); 
  
  // 1. Read current status from the sensor
    unsigned char current_reg29 = mt6701_read_reg(REG_DIR_AABZ);
    unsigned char current_reg30 = mt6701_read_reg(REG_ABZ_RES_H);
    unsigned char current_reg31 = mt6701_read_reg(REG_ABZ_RES_L);
   

    
    // Extract actual direction and resolution fields
    uint8_t current_dir = (current_reg29 & (1 << 1)) ? 1 : 0;
    uint16_t current_abz_res = ((current_reg30 & 0x03) << 8) | current_reg31;
    
    // 2. Compare against predefined targets
    if ((current_dir == TARGET_DIR) && (current_abz_res == TARGET_ABZ_RES)) 
    {
        // Configuration matches perfectly, skip burning to save EEPROM wear
        return; 
    }
    
    // 3. If settings mismatch, prepare and push updates to RAM buffers:
    unsigned char next_reg29 = current_reg29;
    if (TARGET_DIR) 
    {
        next_reg29 |= (1 << 1);
    } else 
    {
        next_reg29 &= ~(1 << 1);
    }
    
    unsigned char next_reg30 = current_reg30 & 0xFC;
    next_reg30 |= ((TARGET_ABZ_RES >> 8) & 0x03);
    
    unsigned char next_reg31 = (unsigned char)(TARGET_ABZ_RES & 0xFF);
    
    // Load values into MT6701 temporary RAM
    mt6701_write_reg(REG_DIR_AABZ, next_reg29);
    mt6701_write_reg(REG_ABZ_RES_H, next_reg30);
    mt6701_write_reg(REG_ABZ_RES_L, next_reg31);
    
 
#if 1 
 
    HAL_Delay(5); // Stabilization delay
    
    // 4. CRITICAL: Execute EEPROM Burning sequence (Strictly requires 5V on VDD!)
    
    // Step A: Send the unlock password to the custom NVM block
    mt6701_write_reg(REG_NVM_UNLOCK, NVM_UNLOCK_KEY);
    HAL_Delay(2);
    
    // Step B: Send the actual physical burning execution command
    mt6701_write_reg(REG_NVM_CMD, NVM_CMD_BURN);
    
    // Step C: Absolute timeout block. The chip physically blows high-voltage cells.
    // Ensure no power interruption or bus noise occurs within this window.
    HAL_Delay(1000); 
#endif
    
}

