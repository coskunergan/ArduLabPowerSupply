////////////////////////////////////////////////////////////////////////////////////
void Eeprom_Save(void)
{
  byte i;
  EEPROM.update(0, (byte)Charge_Voltage_Limit);
  EEPROM.update(1, (byte)(Charge_Voltage_Limit >> 8));
  EEPROM.update(2, (byte)(Display_Charge_Mode));
  EEPROM.update(3, Time_Hour);
  EEPROM.update(4, Time_Minute);
  byte *ptr = (byte*)(void*)&Calculate_Current_mAh;
  EEPROM.update(5, *ptr++);
  EEPROM.update(6, *ptr++);
  EEPROM.update(7, *ptr++);
  EEPROM.update(8, *ptr++);
  EEPROM.update(9, (byte)Rotary_Voltage);
  EEPROM.update(10, (byte)(Rotary_Voltage >> 8));
  EEPROM.update(11, Storage_Voltage_Index);
  for (i = 0; i < STORAGE_VOLTAGE_SIZE * 2; i += 2)
  {
    EEPROM.update(12 + i, (byte)Storage_Voltage[i/2]);
    EEPROM.update(13 + i, (byte)(Storage_Voltage[i/2] >> 8));
  }
  // eeprom verilerine checksum kontrolü eklnecek.
}
////////////////////////////////////////////////////////////////////////////////
void Eeprom_Read(void)
{
  byte i;
  Charge_Voltage_Limit = (int)EEPROM.read(0);
  Charge_Voltage_Limit |= (int)((int)EEPROM.read(1) << 8);
  Rotary_Limit  = (Charge_Voltage_Limit * 2.5);
  Display_Charge_Mode = (byte)EEPROM.read(2);
  Time_Hour   = EEPROM.read(3);
  Time_Minute   = EEPROM.read(4);
  byte *ptr = (byte*)(void*)&Calculate_Current_mAh;
  *ptr++ = (int)EEPROM.read(5);
  *ptr++ = (int)EEPROM.read(6);
  *ptr++ = (int)EEPROM.read(7);
  *ptr++ = (int)EEPROM.read(8);
  Rotary_Voltage = (int)EEPROM.read(9);
  Rotary_Voltage |= (int)((int)EEPROM.read(10) << 8);
  Dac_Voltage = Rotary_Voltage / 2.5;
  Storage_Voltage_Index = (int)EEPROM.read(11);
  for (i = 0; i < STORAGE_VOLTAGE_SIZE * 2; i += 2)
  {
    Storage_Voltage[i/2] = (int)EEPROM.read(12 + i);
    Storage_Voltage[i/2] |= (int)((int)EEPROM.read(13 + i) << 8);
  }
}
////////////////////////////////////////////////////////////////////////////////
