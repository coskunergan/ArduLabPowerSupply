//////////////////////////////////////////////////////////////////////////////
void setup()
{
 // watchdogSetup();

  pinMode(Switch_Pin1, INPUT_PULLUP);
  pinMode(Switch_Pin2, INPUT_PULLUP);

  Timer1.initialize(10000);         // per 10mS
  Timer1.attachInterrupt(ISR_Time_Tick);  // attaches Time_Tick() as a timer overflow interrupt

  Serial.begin(115200);
  Serial.println("Restart!");

  // Initialize the INA219.
  ina219.begin();
  // Or to use a lower 16V, 400mA range (higher precision on volts and amps):
  ina219.setCalibration_16V_400mA();

  // For Adafruit MCP4725A1 the address is 0x62 (default) or 0x63 (ADDR pin tied to VCC)
  // For MCP4725A0 the address is 0x60 or 0x61
  // For MCP4725A2 the address is 0x64 or 0x65
  dac.begin(0x61);

  //---------------------
  //dac.setVoltage(492, true);// default eeprom 3.60V
  //---------------------
  
  pinMode(Lcd_Power_Pin, OUTPUT);
  digitalWrite(Lcd_Power_Pin, HIGH);

  tft.begin();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(3);

  pinMode(encoder0PinA, INPUT_PULLUP);
  pinMode(encoder0PinB, INPUT_PULLUP);
  pinMode(encoder0Btn, INPUT_PULLUP);
  attachInterrupt(0, ISR_doEncoder, CHANGE);

  Eeprom_Read();

  dac.setVoltage(Dac_Voltage, false);

  if (!digitalRead(encoder0Btn))
  {
    delay(10);
    if (!digitalRead(encoder0Btn))
    {
      delay(10);
      if (!digitalRead(encoder0Btn))
      {
        Calculate_Current_mAh = 0;
        Time_Hour = Time_Minute = Time_Second = 0;
        Eeprom_Save();
      }
    }
  }
  wdt_reset();
 
}
////////////////////////////////////////////////////////////////////////////////
void watchdogSetup(void)
{
  cli();  // disable all interrupts
  wdt_reset(); // reset the WDT timer
  /*
    WDTCSR configuration:
    WDIE = 1: Interrupt Enable
    WDE = 1 :Reset Enable
    WDP3 = 0 :For 2000ms Time-out
    WDP2 = 1 :For 2000ms Time-out
    WDP1 = 1 :For 2000ms Time-out
    WDP0 = 1 :For 2000ms Time-out
  */
  // Enter Watchdog Configuration mode:
  WDTCSR |= (1 << WDCE) | (1 << WDE);
  // Set Watchdog settings:
  WDTCSR = (1 << WDIE) | (1 << WDE) | (0 << WDP3) | (1 << WDP2) | (1 << WDP1) | (0 << WDP0);
  sei();
}
////////////////////////////////////////////////////////////////////////////////
