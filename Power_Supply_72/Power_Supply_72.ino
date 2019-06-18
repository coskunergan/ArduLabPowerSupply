/*

  Coşkun ERGAN
  02.01.2017
  Power Suppley Roject

  V1.0.0

*/
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <TFT_HX8357.h> // Hardware-specific library
#include "TimerOne.h"
#include <avr/wdt.h>
#include <EEPROM.h>

Adafruit_INA219 ina219;

#define VERSION_STRING  " Coskun ERGAN  V:1.0.1"
#define MAX_Temperature  60.0  // TEST!
#define Temperature_FILTER_SN  5

#define CURRENT_POZITION   209
#define VOLTAGE_POZITION    11
#define BUTTON_LONG_PRESS_TIME  200  // 2 sn
#define BUTTON_DEBOUNCE_TIME 5  // 50 ms
#define TFT_GREY 0x7BEF

#define encoder0PinA 2
#define encoder0PinB 3
#define encoder0Btn 4

#define CURRENCT_OFFSET_CORRECT_0_16(x) ((0.03 * x) + 0.066)
#define CURRENCT_OFFSET_CORRECT_16_32(x) ((0.028 * x) + 0.745)

#define LIMIT_CORRECT(x) ((-0.038 *x*x*x*x) + (1.768 * x*x*x) - (17.15 * x*x) + (68.68 * x) - 54.28) // 4 . derece polinom
//#define LIMIT_CORRECT(x) ( (9.506*x*x) - (75.66 * x) + 145.9 )// 2. derece polinom

bool Tft_Power_State = true;
bool Measure_Refresh = false;
bool Display_Refresh = false;
bool Second_Procces = false;
bool Minute_Procces = false;
bool Display_Update = false;
bool Display_Charge_Mode = false;
bool Rotary_Button = false;
bool Volt_Graph_Update_State = true;
bool Current_Graph_Update_State = true;
bool Charge_Finish = false;
bool Button_Pressed = false;
bool Button_Released = false;
bool Button_Long_Pressed = false;

byte i = 0;
byte DB_Character = 0;
byte Button_Debounce = 0;
byte Time_Second = 0;
byte Time_Minute = 0;
byte Time_Hour = 0;
byte Analog_Power_State = 0;
byte Prev_Current_Mode = 0;
byte Set_Current_Mode = 0;
byte Second_Counter = 0;

int Button_Counter = 0;
int Analog_Power_Pin = 13;
int Lcd_Power_Pin = 42;
int Rotary_Encoder = 0;
int Charge_Voltage_Limit = 0;
int Current_Limit_Value = 0;
int Temperature_Array[Temperature_FILTER_SN] = {0};
int Temperature_Index = 0 ;
int Temperature_Temp = 0 ;
int Temperature = 0 ;

float Measure_Shunt_Voltage = 0;
float Measure_Bus_Voltage = 0;
float Measure_Current_mA = 0;
float Before_Current_mA = 0;
float Calculate_Current_mAh = 0;
float Measure_Load_Voltage = 0;
float Before_Load_Voltage = 0;

double Current_Draw_Buffer[90];
double Voltage_Draw_Buffer[90];
double Current_Max_Value = 0;
double Voltage_Max_Value = 0;
double Current_Min_Value = 999;
double Voltage_Min_Value = 999;
double Measure_Value_Temp = 0;

TFT_HX8357 tft = TFT_HX8357();       // Invoke custom library
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void Button_Procces(void)
{
  if (!digitalRead(encoder0Btn))
  {
    if (Button_Counter == 0)
    {
      if (Button_Debounce == 0)
      {
        Button_Pressed = true;
        Button_Debounce = BUTTON_DEBOUNCE_TIME;
      }
    }
    Button_Counter++;
  }
  else
  {
    if (Button_Counter)
    {
      if (Button_Debounce == 0)
      {
        Button_Released = true;
        Button_Debounce = BUTTON_DEBOUNCE_TIME;
      }
    }
    Button_Counter = 0;
  }
  if (Button_Counter == BUTTON_LONG_PRESS_TIME)
  {
    if (Button_Debounce == 0)
    {
      Button_Long_Pressed = true;
      Button_Debounce = BUTTON_DEBOUNCE_TIME;
    }
  }
  if (Button_Debounce)
  {
    Button_Debounce--;
  }
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void Time_Tick(void) // per 10mS
{
  //----------------------
  // Per 10mS region
  Button_Procces();
  //----------------------
  // Per 100mS region
  if ((Second_Counter % 10) == 0)
  {
    Measure_Refresh = true;
  }
  //----------------------
  // Per second region
  if (Second_Counter++ >= 100)
  {
    Second_Counter = 1;
    Second_Procces = true;
    Time_Second++;
    if (Time_Second >= 60)
    {
      Time_Second = 0;
      Minute_Procces = true;
      Time_Minute++;
      if (Time_Minute >= 60)
      {
        Time_Minute = 0;
        Time_Hour++;
        if (Time_Hour >= 99)
        {
          Time_Hour = 0;
        }
      }
    }
  }
  //----------------------
}
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
void Eeprom_Save(void)
{
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
  // eeprom verilerine checksum kontrolü eklnecek.
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void Eeprom_Read(void)
{
  Charge_Voltage_Limit = (int)EEPROM.read(0);
  Charge_Voltage_Limit |= (int)((int)EEPROM.read(1) << 8);
  Rotary_Encoder  = (Charge_Voltage_Limit * 2.5);
  Display_Charge_Mode = (byte)EEPROM.read(2);
  Time_Hour   = EEPROM.read(3);
  Time_Minute   = EEPROM.read(4);
  byte *ptr = (byte*)(void*)&Calculate_Current_mAh;
  *ptr++ = (int)EEPROM.read(5);
  *ptr++ = (int)EEPROM.read(6);
  *ptr++ = (int)EEPROM.read(7);
  *ptr++ = (int)EEPROM.read(8);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void setup()
{
  watchdogSetup();
  pinMode(Analog_Power_Pin, OUTPUT);
  digitalWrite(Analog_Power_Pin, HIGH);
  pinMode(10, OUTPUT);// test

  Timer1.initialize(10000);         // per 10mS
  Timer1.attachInterrupt(Time_Tick);  // attaches Time_Tick() as a timer overflow interrupt

  Serial.begin(115200);
  Serial.println("Restart!");

  // Initialize the INA219.
  ina219.begin();
  // Or to use a lower 16V, 400mA range (higher precision on volts and amps):
  ina219.setCalibration_16V_400mA();

  pinMode(Lcd_Power_Pin, OUTPUT);
  digitalWrite(Lcd_Power_Pin, HIGH);

  tft.begin();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(1);

  pinMode(encoder0PinA, INPUT_PULLUP);
  pinMode(encoder0PinB, INPUT_PULLUP);
  pinMode(encoder0Btn, INPUT_PULLUP);
  attachInterrupt(0, doEncoder, CHANGE);

  Eeprom_Read();

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
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
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
  if (Temperature > MAX_Temperature)
  {
    tft.setTextColor(TFT_BLACK, TFT_RED);
  }
  else
  {
    tft.setTextColor(TFT_BLACK, TFT_GREEN);
  }
  tft.drawString(" T:   `C", 83, 309, 1);
  tft.drawNumber(Temperature, 105, 309, 1);
  tft.setTextColor(TFT_MAROON, TFT_GREY);
  tft.drawString(VERSION_STRING, 340, 309, 1);
  //-------------------------------------
  //Serial.println("Encoder: ");
  //Serial.print(valRotary);
  //Serial.println(" ");
  //Serial.println("Button: ");
  //Serial.print(Rotary_Button);
  //Serial.println(" ");
}
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
void doEncoder()
{
  if (digitalRead(encoder0PinA) == digitalRead(encoder0PinB))
  {
    Rotary_Encoder++;
  }
  else if (Rotary_Encoder)
  {
    Rotary_Encoder--;
  }
  Charge_Voltage_Limit = (Rotary_Encoder / 2.5);
  Display_Charge_Mode = true;
}
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
int ss = 0;
void loop(void)
{

  wdt_reset();

  if (Button_Released)
  {
    Button_Released = false;
    Display_Charge_Mode = !Display_Charge_Mode;
    Charge_Finish=false;
  }
  if (Button_Pressed)
  {
    Button_Pressed = false;
    if (Tft_Power_State == 0)
    {
      digitalWrite(Lcd_Power_Pin, HIGH);
      for (int i = 21; i <= 41; i++)
      {
        pinMode(i, OUTPUT);
      }
      tft.begin();
      tft.setRotation(1);
      tft.fillScreen(TFT_BLACK);
      Tft_Power_State = 1;
    }
  }
  if (Button_Long_Pressed)
  {
    Button_Long_Pressed = false;
    if (Tft_Power_State == 1)
    {
      Tft_Power_State = 0;
      tft.fillScreen(TFT_BLACK);
      for (int i = 21; i <= 41; i++)
      {
        digitalWrite(i, LOW);
        pinMode(i, INPUT);
      }
      digitalWrite(Lcd_Power_Pin, LOW);
    }
  }
  //--------------------------------------------------------------
  if (Measure_Refresh)
  {
    digitalWrite(10, HIGH);// test
    Measure_Refresh = false;
    digitalWrite(10, LOW);// test
    Current_Limit_Value = analogRead(A0) * 2.93255;// 1023/3;

    // Current_Limit_Value = analogRead(A0) ;

    // Current_Limit_Value = LIMIT_CORRECT( (double)analogRead(A0) / 46.5);

    if (Current_Limit_Value < 0)
    {
      Current_Limit_Value = 0;
    }
    if (Current_Limit_Value > 3000)
    {
      Current_Limit_Value = 3000;
    }
    
    Measure_Shunt_Voltage = ina219.getShuntVoltage_mV();
    Measure_Bus_Voltage = ina219.getBusVoltage_V();
    Before_Current_mA = Measure_Current_mA;
    Before_Load_Voltage = Measure_Load_Voltage;
    Measure_Current_mA = ina219.getCurrent_mA();
    Measure_Load_Voltage = Measure_Bus_Voltage + (Measure_Shunt_Voltage / 1000);
    //    Measure_Load_Voltage = 3.0; // TEST!
    //    Measure_Current_mA = 100; // TEST!



    //-------------------------------------
    if (Measure_Current_mA < 400)
    {
      Set_Current_Mode = 1;
    }
    else if (Measure_Current_mA < 1300)
    {
      Set_Current_Mode = 2;
    }
    else
    {
      Set_Current_Mode = 3;
    }
    if (Measure_Load_Voltage >= 16 && Set_Current_Mode == 1)
    {
      Set_Current_Mode = 2;
    }
    if (Set_Current_Mode != Prev_Current_Mode)
    {
      Prev_Current_Mode = Set_Current_Mode;
      switch (Set_Current_Mode)
      {
        case 1:
          ina219.setCalibration_16V_400mA();
          break;
        case 2:
          ina219.setCalibration_32V_1A();
          break;
        case 3:
          ina219.setCalibration_32V_2A();
          break;
      }
    }

    if (Measure_Load_Voltage < 0)
    {
      Measure_Load_Voltage = 0;
    }

    if ((Analog_Power_State == 0) && (Measure_Load_Voltage < 1.0)) // analog kısım kapalı iken 1 v un altındaki voltajlar gösterilmez.
    {
      Measure_Load_Voltage = 0;
    }
    if (Set_Current_Mode == 1 )
    {
      Measure_Current_mA -= CURRENCT_OFFSET_CORRECT_0_16(Measure_Load_Voltage);
    }
    else
    {
      Measure_Current_mA -= CURRENCT_OFFSET_CORRECT_16_32(Measure_Load_Voltage);
    }

    if (Measure_Current_mA < 0.1)
    {
      Measure_Current_mA = 0;
    }

    Display_Refresh = true;

    if (0)
    {
      // Serial.print("T: "); Serial.print(Time_Hour); Serial.print(':'); Serial.print(Time_Minute); Serial.print(':'); Serial.print(Time_Second); Serial.println(" Sn");
      Serial.print("V: "); Serial.print(Measure_Load_Voltage); Serial.println(" ");
      Serial.print("U: "); Serial.print(Measure_Current_mA); Serial.println(" ");
      Serial.print("L: "); Serial.print(Current_Limit_Value); Serial.println(" ");
      // Serial.println("");
    }
  }
  //--------------------------------------------------------------
  if (Display_Refresh)
  {
    Display_Refresh = false;
    if (Tft_Power_State)
    {
      Display_Draw_Digits();
    }

    if (Display_Update)
    {
      //////////////////////////////////////////////////////////////////////////////
      //############################################################################
      //////////////////////////////////////////////////////////////////////////////
      if (Tft_Power_State)
      {
        Measure_Value_Temp = (Voltage_Max_Value - Voltage_Min_Value) / 5;
        for (i = 0; i < 90; i += 1)
        {
          Trace(tft, i, Voltage_Draw_Buffer[i], 2, 80, 80, 440, 90, 0, 90, 5, Voltage_Min_Value, Voltage_Max_Value, Measure_Value_Temp, Volt_Graph_Update_State, TFT_BLACK , VOLTAGE_POZITION);
        }
      }
      //////////////////////////////////////////////////////////////////////////////
      //############################################################################
      //////////////////////////////////////////////////////////////////////////////
      Measure_Value_Temp = (Before_Load_Voltage > Measure_Load_Voltage) ? Before_Load_Voltage : Measure_Load_Voltage;
      if (Measure_Value_Temp >= 0.25)
      {
        tft.fillRect(0, 0, 11, 100, TFT_BLACK);
        Voltage_Min_Value = Measure_Value_Temp - 0.25;
        Voltage_Max_Value = Measure_Value_Temp + 0.25;
      }
      else
      {
        tft.fillRect(0, 0, 11, 100, TFT_BLACK);
        Voltage_Min_Value = 0;
        Voltage_Max_Value =  0.5;
      }

      for ( i = 89; i > 1; i--)
      {
        Voltage_Draw_Buffer[i] = Voltage_Draw_Buffer[i - 2];
        if (Voltage_Min_Value >= Voltage_Draw_Buffer[i])
        {
          Voltage_Min_Value = Voltage_Draw_Buffer[i];
        }
        if (Voltage_Max_Value <= Voltage_Draw_Buffer[i])
        {
          Voltage_Max_Value = Voltage_Draw_Buffer[i];
        }
      }
      Voltage_Draw_Buffer[1] = Before_Load_Voltage;
      Voltage_Draw_Buffer[0] = Measure_Load_Voltage;

      Measure_Value_Temp = (Voltage_Max_Value - Voltage_Min_Value) / 5;
      //////////////////////////////////////////////////////////////////////////////
      //############################################################################
      //////////////////////////////////////////////////////////////////////////////
      Volt_Graph_Update_State = true;
      if (Tft_Power_State)
      {
        for (i = 0; i < 90; i += 1)
        {
          Trace(tft, i, Voltage_Draw_Buffer[i], 2, 80, 80, 440, 90, 0, 90, 5, Voltage_Min_Value, Voltage_Max_Value, Measure_Value_Temp, Volt_Graph_Update_State, TFT_CYAN , VOLTAGE_POZITION);
        }
      }
      //////////////////////////////////////////////////////////////////////////////
      //############################################################################
      //////////////////////////////////////////////////////////////////////////////
    }
    if (!Display_Update)
    {
      if (Current_Max_Value > 99)
      {
        DB_Character = 1;
      }
      else
      {
        DB_Character = 2;
      }
      //////////////////////////////////////////////////////////////////////////////
      //############################################################################
      //////////////////////////////////////////////////////////////////////////////
      if (Tft_Power_State)
      {
        Measure_Value_Temp = (Current_Max_Value - Current_Min_Value) / 5;
        for (i = 0; i < 90; i += 1)
        {
          Trace(tft, i, Current_Draw_Buffer[i], DB_Character, 80, 80, 440, 90, 0, 90, 5, Current_Min_Value, Current_Max_Value, Measure_Value_Temp, Current_Graph_Update_State, TFT_BLACK , CURRENT_POZITION);
        }
      }
      //////////////////////////////////////////////////////////////////////////////
      //############################################################################
      //////////////////////////////////////////////////////////////////////////////
      Measure_Value_Temp = (Before_Current_mA > Measure_Current_mA) ? Before_Current_mA : Measure_Current_mA;
      if (Measure_Value_Temp >= 0.5)
      {
        tft.fillRect(0, 198, 11, 100, TFT_BLACK);
        Current_Min_Value = Measure_Value_Temp - 0.5;
        Current_Max_Value = Measure_Value_Temp + 0.5;
      }
      else
      {
        tft.fillRect(0, 198, 11, 100, TFT_BLACK);
        Current_Min_Value = 0;
        Current_Max_Value = 1;
      }
      for ( i = 89; i > 1; i--)
      {
        Current_Draw_Buffer[i] = Current_Draw_Buffer[i - 2];
        if (Current_Min_Value >= Current_Draw_Buffer[i])
        {
          Current_Min_Value = Current_Draw_Buffer[i];
        }
        if (Current_Max_Value <= Current_Draw_Buffer[i])
        {
          Current_Max_Value = Current_Draw_Buffer[i];
        }
      }
      Current_Draw_Buffer[1] = Before_Current_mA;
      Current_Draw_Buffer[0] = Measure_Current_mA;

      Measure_Value_Temp = (Current_Max_Value - Current_Min_Value) / 5;
      //////////////////////////////////////////////////////////////////////////////
      //############################################################################
      //////////////////////////////////////////////////////////////////////////////
      Current_Graph_Update_State = true;
      if (Tft_Power_State)
      {
        for (i = 0; i < 90; i += 1)
        {
          Trace(tft, i, Current_Draw_Buffer[i], DB_Character, 80, 80, 440, 90, 0, 90, 5, Current_Min_Value, Current_Max_Value, Measure_Value_Temp, Current_Graph_Update_State, TFT_YELLOW , CURRENT_POZITION);
        }
      }
      //////////////////////////////////////////////////////////////////////////////
      //############################################################################
      //////////////////////////////////////////////////////////////////////////////
    }
    Display_Update = !Display_Update;
  }
  //--------------------------------------------------------------------------
  if (Second_Procces)
  {
    Second_Procces = false;

    Temperature_Array[Temperature_Index] = 0.41 * analogRead(A1) - 273.15; // 5V referanslı adc ye göre LM335 için sıcaklık hesabı.
    Temperature_Index = (Temperature_Index + 1) % Temperature_FILTER_SN;
    Temperature_Temp = 0;
    for (int i = 0; i < Temperature_FILTER_SN; i++)
    {
      Temperature_Temp += Temperature_Array[i];
    }
    Temperature_Temp /= Temperature_FILTER_SN;
    Temperature = Temperature_Temp;
    if (Temperature > MAX_Temperature)
    {
      Analog_Power_State = 0;
      digitalWrite(Analog_Power_Pin, HIGH);// çıkış kapalı
    }
    else
    {
      if ((Display_Charge_Mode) && ((Charge_Finish) || (Charge_Voltage_Limit <= Measure_Load_Voltage * 100))  )
      {
        Charge_Finish = true;
        Analog_Power_State = 0;
        digitalWrite(Analog_Power_Pin, HIGH);// çıkış kapalı
      }
      else
      {
        Analog_Power_State = 1;
        digitalWrite(Analog_Power_Pin, LOW);// çıkış açık
      }
    }

    Calculate_Current_mAh += (Measure_Current_mA / 3600);

    Eeprom_Save();

    Prev_Current_Mode = 0; // refresh init ina219 periodicly

  }
  //--------------------------------------------------------------------------
  if (Minute_Procces)
  {
    Minute_Procces = false;
    if (Tft_Power_State)
    {
      tft.fillScreen(TFT_BLACK);
    }
  }
  //--------------------------------------------------------------------------
}
////////////////////////////////////////////////////////////////////////////////




