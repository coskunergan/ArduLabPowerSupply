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

Adafruit_INA219 ina219;

#define MAX_TEMPERATURE  60.0
#define TEMPERATURE_FILTER_SN  5

#define AMPER   209
#define VOLT    11
#define TFT_GREY 0x7BEF

#define encoder0PinA 2
#define encoder0PinB 3
#define encoder0Btn 4

int pin_relay = 13;
int encoder0Pos = 0;
int mah_current_limit = 0;
bool Rot_Button = false;

boolean volt_display_graph_update = true;
boolean volt_display_trace_update = true;

boolean amper_display_graph_update = true;
boolean amper_display_trace_update = true;

double x, y, z;
byte dp_chart_amper;
byte j, i, r;
double buff_amper[90];
double buff_volt[90];
double Amper_Max_Value = 0;
double Volt_Max_Value = 0;
double Amper_Min_Value = 999;
double Volt_Min_Value = 999;
double temp;
float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
float current_mA2 = 0;
float current_mAh = 0;
float loadvoltage = 0;
float loadvoltage2 = 0;
int limit_value;
int temp_limit_value;
byte index = 0;
byte second = 0;
byte minute = 0;
byte hour = 0;
int temperature_array[TEMPERATURE_FILTER_SN] = {0};
int temperature_index = 0 ;
int temperature_temp = 0 ;
int temperature = 0 ;
byte relay_output = 0;
byte relay_on_off = 0;
byte Prev_Current_Mode = 0;
byte Set_Current_Mode = 0;
byte second_tick = 0;
bool Tft_Power_State = true;
bool Measure_Refresh = false;
bool Display_Refresh = false;
bool Button_Refresh = false;
byte Button_Debounce = 0;
bool Second_Procces = false;
bool Minute_Procces = false;
bool Disply_tettt = false;
bool display_mah_mode = false;

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
void Time_Tick(void) // per 10mS
{
  //----------------------
  // Per 10mS region
  if (!Button_Debounce)
  {
    Button_Refresh = true;
  }
  else
  {
    Button_Debounce--;
  }
  //----------------------
  // Per 100mS region
  if ((second_tick % 10) == 0)
  {
    Measure_Refresh = true;
  }
  //----------------------
  // Per second region
  if (second_tick++ >= 100)
  {
    second_tick = 1;
    Second_Procces = true;
    second++;
    if (second >= 60)
    {
      second = 0;
      Minute_Procces = true;
      minute++;
      if (minute >= 60)
      {
        minute = 0;
        hour++;
        if (hour >= 99)
        {
          hour = 0;
        }
      }
    }
  }
  //----------------------
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void setup()
{
  watchdogSetup();
  pinMode(pin_relay, OUTPUT);
  digitalWrite(pin_relay, HIGH);
  pinMode(10, OUTPUT);// test

  Timer1.initialize(10000);         // per 10mS
  Timer1.attachInterrupt(Time_Tick);  // attaches Time_Tick() as a timer overflow interrupt

  Serial.begin(115200);
  Serial.println("Restart!");

  // Initialize the INA219.
  ina219.begin();
  // Or to use a lower 16V, 400mA range (higher precision on volts and amps):
  ina219.setCalibration_16V_400mA();

  pinMode(42, OUTPUT);
  digitalWrite(42, HIGH);

  tft.begin();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(1);

  pinMode(encoder0PinA, INPUT_PULLUP);
  pinMode(encoder0PinB, INPUT_PULLUP);
  pinMode(encoder0Btn, INPUT_PULLUP);
  attachInterrupt(0, doEncoder, CHANGE);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void Display_Draw_Digits(void)
{
  double limit;
  double power_mW;
  byte buff[11];
  //-------------------------------------
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString(" Voltage:", 5, 110, 2);
  if (loadvoltage >= 10)
  {
    tft.drawFloat(loadvoltage, 1, 5, 130, 7);
  }
  else
  {
    tft.drawFloat(loadvoltage, 2, 5, 130, 7);
  }
  tft.drawString("V ", 115, 158, 4);
  tft.drawString(" Time: ", 5, 180, 2);
  buff[0] = (hour % 60 / 10) + '0';
  buff[1] = (hour % 10) + '0';
  buff[2] = ':';
  buff[3] = (minute % 60 / 10) + '0';
  buff[4] = (minute % 10) + '0';
  buff[5] = ':';
  buff[6] = (second % 60 / 10) + '0';
  buff[7] = (second % 10) + '0';
  buff[8] = ' ';
  buff[9] = 's';
  buff[10] = 'n';
  buff[11] = 0;
  tft.drawString(buff, 45, 180, 2);
  //--------------------------------------
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString(" Current:", 160, 110, 2);
  if (current_mA >= 1000)
  {
    tft.drawFloat(current_mA / 1000, 2, 160, 130, 7);
    tft.drawString("A     ", 268, 158, 4);
  }
  else if (current_mA >= 100)
  {
    tft.fillRect(160, 130, 11, 50, TFT_BLACK);
    tft.drawNumber(current_mA, 170, 130, 7);
    tft.drawString("mA", 268, 158, 4);
  }
  else if (current_mA >= 10)
  {
    tft.drawFloat(current_mA, 1, 160, 130, 7);
    tft.drawString("mA", 268, 158, 4);
  }
  else
  {
    tft.drawFloat(current_mA, 2, 160, 130, 7);
    tft.drawString("mA", 268, 158, 4);
  }
  //-------------------------------------
  power_mW = current_mA * loadvoltage;
  tft.drawString(" P: ", 160, 180, 2);
  tft.drawFloat(power_mW, 1, 185, 180, 2);
  if (power_mW >= 1000)
  {
    tft.drawString(" mW ", 229, 180, 2);
  }
  else if (power_mW >= 100)
  {
    tft.drawString(" mW ", 221, 180, 2);
  }
  else if (power_mW >= 10)
  {
    tft.drawString(" mW  ", 213, 180, 2);
  }
  else
  {
    tft.drawString(" mW    ", 205, 180, 2);
  }
  //-------------------------------------
  limit = (double)limit_value * 2.93255;// 1023/3
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawString(" Limit: ", 325, 110, 2);
  if (display_mah_mode)
  {
    if (limit >= 1000)
    {
      tft.drawFloat(limit / 1000, 2, 390, 110, 2);
      tft.drawString("A     ", 425, 110, 2);
    }
    else if (limit >= 100)
    {
      tft.drawNumber(limit, 390, 110, 2);
      tft.drawString("mA", 425, 110, 2);
    }
    else if (limit >= 10)
    {
      tft.drawFloat(limit, 1, 390, 110, 2);
      tft.drawString("mA", 425, 110, 2);
    }
    else
    {
      tft.drawFloat(limit, 2, 390, 110, 2);
      tft.drawString("mA", 425, 110, 2);
    }
    limit =  mah_current_limit * 10;
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("Ah     ", 435, 158, 4);
  }
  else
  {
    tft.drawString("        ", 390, 110, 2);
    if (limit >= 1000)
    {
      tft.drawString("A     ", 435, 158, 4);
    }
    else// if (limit >= 100)
    {
      tft.drawString("mA", 435, 158, 4);
    }
  }
  if (display_mah_mode || limit >= 1000)
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
  tft.drawFloat(current_mAh, 1, 355, 180, 2);
  if (current_mAh >= 1000)
  {
    tft.drawString(" mAh ", 399, 180, 2);
  }
  else if (current_mAh >= 100)
  {
    tft.drawString(" mAh ", 391, 180, 2);
  }
  else if (current_mAh >= 10)
  {
    tft.drawString(" mAh  ", 383, 180, 2);
  }
  else
  {
    tft.drawString(" mAh   ", 375, 180, 2);
  }
  //-------------------------------------
  tft.fillRect(0, 305, 480, 320, TFT_GREY);
  if (relay_output)
  {
    tft.setTextColor(TFT_BLACK, TFT_GREEN);
  }
  else
  {
    tft.setTextColor(TFT_BLACK, TFT_RED);
  }
  tft.drawString(" Output:  ", 13, 309, 1);
  tft.drawNumber(relay_output, 60, 309, 1);
  if (temperature > MAX_TEMPERATURE)
  {
    tft.setTextColor(TFT_BLACK, TFT_RED);
  }
  else
  {
    tft.setTextColor(TFT_BLACK, TFT_GREEN);
  }
  tft.drawString(" T:   `C", 83, 309, 1);
  tft.drawNumber(temperature, 105, 309, 1);
  tft.setTextColor(TFT_MAROON, TFT_GREY);
  tft.drawString(" Coskun ERGAN  V:1.0.0", 340, 309, 1);
  //-------------------------------------
  //Serial.println("Encoder: ");
  //Serial.print(valRotary);
  //Serial.println(" ");
  //Serial.println("Button: ");
  //Serial.print(Rot_Button);
  //Serial.println(" ");
}
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
void doEncoder()
{
  if (digitalRead(encoder0PinA) == digitalRead(encoder0PinB))
  {
    encoder0Pos++;
  }
  else if (encoder0Pos)
  {
    encoder0Pos--;
  }
  mah_current_limit = encoder0Pos / 2.5;
  display_mah_mode = true;
}
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
void loop(void)
{
  wdt_reset();
  if (Button_Refresh && !Button_Debounce )
  {
    Button_Refresh = false;
    Rot_Button = digitalRead(encoder0Btn);
    if (Tft_Power_State == 1)
    {
      if (!Rot_Button)
      {
        Button_Debounce = 20;// 200ms
        Tft_Power_State = 0;
        tft.fillScreen(TFT_BLACK);
        for (int i = 21; i <= 41; i++)
        {
          digitalWrite(i, LOW);
          pinMode(i, INPUT);
        }
        digitalWrite(42, LOW);
        Display_Refresh = false;
      }
    }
    else
    {
      if (!Rot_Button)
      {
        Button_Debounce = 20;// 200ms
        digitalWrite(42, HIGH);
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
  }
  //--------------------------------------------------------------
  if (Measure_Refresh)
  {
    digitalWrite(10, HIGH);// test
    Measure_Refresh = false;
    digitalWrite(10, LOW);// test
    limit_value = analogRead(A0);

    if ((limit_value > 5) && ((limit_value > temp_limit_value + 5) || (limit_value < temp_limit_value - 5)))
    {
      temp_limit_value = limit_value;
      display_mah_mode = false;
    }
    if (relay_output == 1)
    {
      shuntvoltage = ina219.getShuntVoltage_mV();
      busvoltage = ina219.getBusVoltage_V();
      current_mA2 = current_mA;
      loadvoltage2 = loadvoltage;
      current_mA = ina219.getCurrent_mA();
      loadvoltage = busvoltage + (shuntvoltage / 1000);
    }
    else
    {
      shuntvoltage = 0;
      busvoltage = 0;
      current_mA2 = 0;
      loadvoltage2 = 0;
      current_mA = 0;
      loadvoltage = 0;
    }

    if (current_mA < 0)
    {
      current_mA = 0;
    }
    if (current_mA < 400)
    {
      Set_Current_Mode = 1;
    }
    else if (current_mA < 1300)
    {
      Set_Current_Mode = 2;
    }
    else
    {
      Set_Current_Mode = 3;
    }
    if (loadvoltage >= 16 && Set_Current_Mode == 1)
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
      // current_mA = ina219.getCurrent_mA();
    }
    if ((Set_Current_Mode != 1) && (current_mA < 1.5))
    {
      current_mA = 0;
    }
    loadvoltage -= (current_mA * 0.00047);
    if ((relay_output == 0) && (loadvoltage < 1.0)) // analog kısım kapalı iken 1 v un altındaki voltajlar gösterilmez.
    {
      loadvoltage = 0;
    }
    if (loadvoltage < 16)
    {
      if (loadvoltage >= 12)
      {
        current_mA -= 0.2;
      }
      else if (loadvoltage >= 7)
      {
        current_mA -= 0.1;
      }
    }
    if (current_mA < 0)
    {
      current_mA = 0;
    }
    if (loadvoltage < 0)
    {
      loadvoltage = 0;
    }
    Display_Refresh = true;
  }
  //--------------------------------------------------------------
  if (Display_Refresh)
  {
    Display_Refresh = false;
    if (Tft_Power_State)
    {
      Display_Draw_Digits();
    }
    if (0)
    {
      Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
      Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
      Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
      Serial.print("Current:       "); Serial.print(current_mA * 10000); Serial.println(" mA");
      Serial.println("");
    }
    if (Disply_tettt)
    {
      //////////////////////////////////////////////////////////////////////////////
      //############################################################################
      //////////////////////////////////////////////////////////////////////////////
      if (Tft_Power_State)
      {
        temp = (Volt_Max_Value - Volt_Min_Value) / 5;
        for (i = 0; i < 90; i += 1)
        {
          Trace(tft, i, buff_volt[i], 2, 80, 80, 440, 90, 0, 90, 5, Volt_Min_Value, Volt_Max_Value, temp, volt_display_trace_update, TFT_BLACK , VOLT);
        }
      }
      //////////////////////////////////////////////////////////////////////////////
      //############################################################################
      //////////////////////////////////////////////////////////////////////////////
      temp = (loadvoltage2 > loadvoltage) ? loadvoltage2 : loadvoltage;
      if (temp >= 0.25)
      {
        tft.fillRect(0, 0, 11, 100, TFT_BLACK);
        Volt_Min_Value = temp - 0.25;
        Volt_Max_Value = temp + 0.25;
      }
      else
      {
        tft.fillRect(0, 0, 11, 100, TFT_BLACK);
        Volt_Min_Value = 0;
        Volt_Max_Value =  0.5;
      }

      for ( i = 89; i > 1; i--)
      {
        buff_volt[i] = buff_volt[i - 2];
        if (Volt_Min_Value >= buff_volt[i])
        {
          Volt_Min_Value = buff_volt[i];
        }
        if (Volt_Max_Value <= buff_volt[i])
        {
          Volt_Max_Value = buff_volt[i];
        }
      }
      buff_volt[1] = loadvoltage2;
      buff_volt[0] = loadvoltage;

      temp = (Volt_Max_Value - Volt_Min_Value) / 5;
      //////////////////////////////////////////////////////////////////////////////
      //############################################################################
      //////////////////////////////////////////////////////////////////////////////
      volt_display_trace_update = true;
      if (Tft_Power_State)
      {
        for (i = 0; i < 90; i += 1)
        {
          Trace(tft, i, buff_volt[i], 2, 80, 80, 440, 90, 0, 90, 5, Volt_Min_Value, Volt_Max_Value, temp, volt_display_trace_update, TFT_CYAN , VOLT);
        }
      }
      //////////////////////////////////////////////////////////////////////////////
      //############################################################################
      //////////////////////////////////////////////////////////////////////////////
    }
    if (!Disply_tettt)
    {
      if (Amper_Max_Value > 99)
      {
        dp_chart_amper = 1;
      }
      else
      {
        dp_chart_amper = 2;
      }
      //////////////////////////////////////////////////////////////////////////////
      //############################################################################
      //////////////////////////////////////////////////////////////////////////////
      if (Tft_Power_State)
      {
        temp = (Amper_Max_Value - Amper_Min_Value) / 5;
        for (i = 0; i < 90; i += 1)
        {
          Trace(tft, i, buff_amper[i], dp_chart_amper, 80, 80, 440, 90, 0, 90, 5, Amper_Min_Value, Amper_Max_Value, temp, amper_display_trace_update, TFT_BLACK , AMPER);
        }
      }
      //////////////////////////////////////////////////////////////////////////////
      //############################################################################
      //////////////////////////////////////////////////////////////////////////////
      temp = (current_mA2 > current_mA) ? current_mA2 : current_mA;
      if (temp >= 0.5)
      {
        tft.fillRect(0, 198, 11, 100, TFT_BLACK);
        Amper_Min_Value = temp - 0.5;
        Amper_Max_Value = temp + 0.5;
      }
      else
      {
        tft.fillRect(0, 198, 11, 100, TFT_BLACK);
        Amper_Min_Value = 0;
        Amper_Max_Value = 1;
      }
      for ( i = 89; i > 1; i--)
      {
        buff_amper[i] = buff_amper[i - 2];
        if (Amper_Min_Value >= buff_amper[i])
        {
          Amper_Min_Value = buff_amper[i];
        }
        if (Amper_Max_Value <= buff_amper[i])
        {
          Amper_Max_Value = buff_amper[i];
        }
      }
      buff_amper[1] = current_mA2;
      buff_amper[0] = current_mA;

      temp = (Amper_Max_Value - Amper_Min_Value) / 5;
      //////////////////////////////////////////////////////////////////////////////
      //############################################################################
      //////////////////////////////////////////////////////////////////////////////
      amper_display_trace_update = true;
      if (Tft_Power_State)
      {
        for (i = 0; i < 90; i += 1)
        {
          Trace(tft, i, buff_amper[i], dp_chart_amper, 80, 80, 440, 90, 0, 90, 5, Amper_Min_Value, Amper_Max_Value, temp, amper_display_trace_update, TFT_YELLOW , AMPER);
        }
      }
      //////////////////////////////////////////////////////////////////////////////
      //############################################################################
      //////////////////////////////////////////////////////////////////////////////
    }
    Disply_tettt = !Disply_tettt;
  }
  //--------------------------------------------------------------------------
  if (Second_Procces)
  {
    Second_Procces = false;
    temperature_array[temperature_index] = 0.41 * analogRead(A1) - 273.15; // 5V referanslı adc ye göre LM335 için sıcaklık hesabı.
    temperature_index = (temperature_index + 1) % TEMPERATURE_FILTER_SN;
    temperature_temp = 0;
    for (int i = 0; i < TEMPERATURE_FILTER_SN; i++)
    {
      temperature_temp += temperature_array[i];
    }
    temperature_temp /= TEMPERATURE_FILTER_SN;
    temperature = temperature_temp;
    if (temperature > MAX_TEMPERATURE)
    {
      relay_output = 0;
      digitalWrite(pin_relay, HIGH);// çıkış kapalı
    }
    else
    {
      if ( (mah_current_limit <= current_mA) && display_mah_mode )
      {
        relay_output = 0;
        digitalWrite(pin_relay, HIGH);// çıkış kapalı
      }
      else
      {
        relay_output = 1;
        digitalWrite(pin_relay, LOW);// çıkış açık
      }
    }
    current_mAh += (current_mA / 3600);
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




