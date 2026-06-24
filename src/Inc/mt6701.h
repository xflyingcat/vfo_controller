#ifndef __MT6701_H__
#define __MT6701_H__

/**
 * Configure MT6701 encoder parameters in RAM during startup
 * @param ppr Desired Pulses Per Revolution (range: 1 to 1024)
 * @param dir Rotation direction: 0 = CCW (Counter-Clockwise), 1 = CW (Clockwise)
 */
void mt6701_configure(uint16_t ppr, uint8_t dir);
void mt6701_eeprom_check_and_burn(void);
void mt6701_init(void);
void mt6701_deinit(void);

#endif

