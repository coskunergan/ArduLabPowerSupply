////////////////////////////////////////////////////////////////////////////////
void Display_Draw_Digits(void)
{
  double limit;
  static double limit_temp;
  double power_W;
  byte buff[11];
  //-------------------------------------
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString(" Voltage:", 5, 110, 2);
  if (Measure_Load_Voltage >= 10)
  {
    tft.drawFloat(Measure_Load_Voltage, 1, 5, 130, 7);
  }
  else
  {
    tft.drawFloat(Measure_Load_Voltage, 2, 5, 130, 7);
  }
  tft.drawString("V ", 115, 158, 4);
  tft.drawString(" Time: ", 5, 180, 2);
  buff[0] = (Time_Hour % 60 / 10) + '0';
  buff[1] = (Time_Hour % 10) + '0';
  buff[2] = ':';
  buff[3] = (Time_Minute % 60 / 10) + '0';
  buff[4] = (Time_Minute % 10) + '0';
  buff[5] = ':';
  buff[6] = (Time_Second % 60 / 10) + '0';
  buff[7] = (Time_Second % 10) + '0';
  buff[8] = ' ';
  buff[9] = 's';
  buff[10] = 'n';
  buff[11] = 0;
  tft.drawString(buff, 45, 180, 2);
  //--------------------------------------
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString(" Current:", 160, 110, 2);
  if (Measure_Current_mA >= 1000)
  {
    tft.drawFloat(Measure_Current_mA / 1000, 2, 160, 130, 7);
    tft.drawString("A     ", 268, 158, 4);
  }
  else if (Measure_Current_mA >= 100)
  {
    tft.fillRect(160, 130, 11, 50, TFT_BLACK);
    tft.drawNumber(Measure_Current_mA, 170, 130, 7);
    tft.drawString("mA", 268, 158, 4);
  }
  else if (Measure_Current_mA >= 10)
  {
    tft.drawFloat(Measure_Current_mA, 1, 160, 130, 7);
    tft.drawString("mA", 268, 158, 4);
  }
  else
  {
    tft.drawFloat(Measure_Current_mA, 2, 160, 130, 7);
    tft.drawString("mA", 268, 158, 4);
  }
  //-------------------------------------
  power_W = (Measure_Current_mA / 1000) * Measure_Load_Voltage;
  tft.drawString(" P: ", 160, 180, 2);
  if (power_W > 1.0)
  {
    tft.drawFloat(power_W, 5, 185, 180, 2);
    if (power_W >= 10)
    {
      tft.drawString(" W   ", 238, 180, 2);
    }
    else
    {
      tft.drawString(" W   ", 230, 180, 2);
    }
  }
  else
  {
    power_W *= 1000;
    tft.drawFloat(power_W, 2, 185, 180, 2);
    if (power_W >= 100)
    {
      tft.drawString("  mW   ", 229, 180, 2);
    }
    else if (power_W >= 10)
    {
      tft.drawString("  mW   ", 221, 180, 2);
    }
    else
    {
      tft.drawString("  mW   ", 214, 180, 2);
    }
  }
  //-------------------------------------
  limit = (double)Current_Limit_Value;
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawString(" Limit: ", 325, 110, 2);
  if (Display_Charge_Mode)
  {
    if (limit >= 1000)
    {
      tft.drawFloat(limit / 1000, 2, 390, 110, 2);
      tft.drawString(" A      ", 420, 110, 2);
    }
    else if (limit >= 100)
    {
      tft.drawNumber(limit, 390, 110, 2);
      tft.drawString(" mA ", 414, 110, 2);
    }
    else if (limit >= 10)
    {
      tft.drawFloat(limit, 1, 390, 110, 2);
      tft.drawString(" mA ", 420, 110, 2);
    }
    else
    {
      tft.drawFloat(limit, 2, 390, 110, 2);
      tft.drawString(" mA ", 420, 110, 2);
    }
    limit =  Charge_Voltage_Limit * 10;
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    if (limit >= 10000)
    {
      tft.drawString("   ", 462, 131, 4);
      tft.drawString("   ", 462, 157, 4);
      tft.drawString("V", 467, 163, 2);
    }
    else
    {
      tft.drawString("      ", 435, 131, 4);
      tft.drawString("V     ", 435, 157, 4);
    }
  }
  else
  {
    tft.drawString("         ", 390, 110, 2);
    tft.drawString("        ", 435, 131, 4);
    if (limit >= 1000)
    {
      tft.drawString("A       ", 435, 157, 4);
    }
    else// if (limit >= 100)
    {
      tft.drawString("mA      ", 435, 157, 4);
    }
  }
  if (Display_Charge_Mode || limit >= 1000)
  {
    tft.drawFloat(limit / 1000, 2, 325, 130, 7);
  }
  else if (limit >= 100)
  {
    tft.fillRect(325, 130, 11, 50, TFT_BLACK);
    tft.drawNumber(limit, 335, 130, 7);
  }
  else if (limit >= 10)
  {
    tft.drawFloat(limit, 1, 325, 130, 7);
  }
  else
  {
    tft.drawFloat(limit, 2, 325, 130, 7);
  }
  //-------------------------------------
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString(" Ah: ", 325, 180, 2);
  tft.drawFloat(Calculate_Current_mAh, 1, 355, 180, 2);
  if (Calculate_Current_mAh >= 1000)
  {
    tft.drawString(" mAh ", 399, 180, 2);
  }
  else if (Calculate_Current_mAh >= 100)
  {
    tft.drawString(" mAh ", 391, 180, 2);
  }
  else if (Calculate_Current_mAh >= 10)
  {
    tft.drawString(" mAh  ", 383, 180, 2);
  }
  else
  {
    tft.drawString(" mAh   ", 375, 180, 2);
  }
  //-------------------------------------
  tft.fillRect(0, 305, 480, 320, TFT_GREY);
  if (Analog_Power_State)
  {
    tft.setTextColor(TFT_BLACK, TFT_GREEN);
  }
  else
  {
    tft.setTextColor(TFT_BLACK, TFT_RED);
  }
  tft.drawString(" Output:  ", 13, 309, 1);
  tft.drawNumber(Analog_Power_State, 60, 309, 1);
  if (Temperature >= MAX_Temperature)
  {
    tft.setTextColor(TFT_BLACK, TFT_RED);
  }
  else if (Temperature >= MAX_Temperature -10)
  {
    tft.setTextColor(TFT_BLACK, TFT_ORANGE);
  }  
  else
  {
    tft.setTextColor(TFT_BLACK, TFT_GREEN);
  }
  tft.drawString(" T:   `C", 83, 309, 1);
  tft.drawNumber(Temperature, 105, 309, 1);
  tft.setTextColor(TFT_MAROON, TFT_GREY);
  tft.drawString(VERSION_STRING, 300, 309, 1);
  tft.drawNumber((CPU_Load_Avr * 100) / 255, 460, 309, 1);
//  tft.drawNumber(  test, 433, 309, 1);
  
  //-------------------------------------
  //Serial.println("Encoder: ");
  //Serial.print(valRotary);
  //Serial.println(" ");
  //Serial.println("Button: ");
  //Serial.print(Rotary_Button);
  //Serial.println(" ");
}
////////////////////////////////////////////////////////////////////////////////////
