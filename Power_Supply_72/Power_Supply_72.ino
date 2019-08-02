/*

  Coşkun ERGAN
  02.01.2017
  Power Suppley Roject

  V1.0.0

*/
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Adafruit_MCP4725.h>
#include <TFT_HX8357.h> // Hardware-specific library
#include "TimerOne.h"
#include <avr/wdt.h>
#include <EEPROM.h>

Adafruit_MCP4725 dac;
Adafruit_INA219 ina219;

#define VERSION_STRING  " Coskun ERGAN  V:2.0"
#define MAX_Temperature  60.0  // TEST!
#define Temperature_FILTER_SN  5
#define TEMPERATURE_CAL   0.48
#define CURRENT_POZITION   209
#define VOLTAGE_POZITION    11
#define BUTTON_LONG_PRESS_TIME  100  // 1 sn
#define BUTTON_DEBOUNCE_TIME 5  // 50 ms
#define TFT_GREY 0x7BEF

#define encoder0PinA 2
#define encoder0PinB 3
#define encoder0Btn 4

#define CURRENCT_OFFSET_CORRECT_0_16(x) ((0.030 * x) - 0.036)
#define CURRENCT_OFFSET_CORRECT_16_32(x) ((0.031 * x) + 0.968)

#define LIMIT_CORRECT(x) ((-0.038 *x*x*x*x) + (1.768 * x*x*x) - (17.15 * x*x) + (68.68 * x) - 54.28) // 4 . derece polinom
//#define LIMIT_CORRECT(x) ( (9.506*x*x) - (75.66 * x) + 145.9 )// 2. derece polinom

/* Unloaded 'max' bg loops per xmS task */
#define BG_LOOPS_PER_TASK ( 1121400 / 2 )
#define AVERAGE_CNT 5

#define STORAGE_VOLTAGE_SIZE 4

volatile bool Measure_Refresh = false;
volatile bool Display_Refresh = false;
volatile bool Second_Procces = false;
volatile bool Minute_Procces = false;
volatile bool Display_Update = false;
volatile bool Display_Charge_Mode = false;
volatile bool Rotary_Button = false;
volatile bool Charge_Finish = false;
volatile bool Button_Pressed = false;
volatile bool Button_Released = false;
volatile bool Button_Long_Pressed = false;
volatile bool Mode_Lock_Flag = false;
volatile bool Rotary_Up_Flag = true;
bool Volt_Graph_Update_State = true;
bool Current_Graph_Update_State = true;
bool saved = false;

volatile byte Rotary_Fast_Mode = false;
volatile byte Rotary_VeryFast_Mode = false;
byte i = 0;
byte DB_Character = 0;
byte Button_Debounce = 100;
byte Time_Second = 0;
byte Time_Minute = 0;
byte Time_Hour = 0;
byte Analog_Power_State = 0;
byte Prev_Current_Mode = 0;
byte Set_Current_Mode = 0;
byte Second_Counter = 0;
byte CPU_util_pct = 0; /* 0 = 0% , 255 = 100% */
byte CPU_Load_Avr = 255;/* 0 = 0% , 255 = 100% */
byte CPU_util_array[AVERAGE_CNT];
byte  Storage_Voltage_Index;
byte StepDown_Step;

int Button_Counter = 0;
int Switch_Pin1 = 12;
int Switch_Pin2 = 11;
int StepDown_SetVoltage_Pin1 = 10;
int StepDown_SetVoltage_Pin2 = 9;
int StepDown_SetVoltage_Pin3 = 8;
int StepDown_SetVoltage_Pin4 = 7;
int Lcd_Power_Pin = 42;
int Charge_Voltage_Limit = 0;
int Current_Limit_Value = 0;
int Temperature_Array[Temperature_FILTER_SN] = {0};
int Temperature_Index = 0 ;
int Temperature_Temp = 0 ;
int Temperature = 0 ;
volatile int Rotary_Voltage = 0;
volatile int Rotary_Limit = 0;
volatile int Dac_Voltage = 0;
int Storage_Voltage[STORAGE_VOLTAGE_SIZE];

unsigned long Average_Current_Count = 0;
unsigned long Bg_Loop_Cnt = 0;

float Measure_Shunt_Voltage = 0;
float Measure_Bus_Voltage = 0;
float Measure_Current_mA = 0;
float Before_Current_mA = 0;
float Calculate_Current_mAh = 0;
float Measure_Load_Voltage = 0;
float Before_Load_Voltage = 0;
float Average_Current = 0;

double Average_Current_Total = 0;
double Current_Draw_Buffer[90];
double Voltage_Draw_Buffer[90];
double Current_Max_Value = 0;
double Voltage_Max_Value = 0;
double Current_Min_Value = 999;
double Voltage_Min_Value = 999;
double Measure_Value_Temp = 0;

unsigned long test = 0;

TFT_HX8357 tft = TFT_HX8357();

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void loop(void)
{
  //  while (Second_Procces==false);
  //  Second_Procces=false;
  //  test=0;
  //  while (Second_Procces==false)
  //  {
  //    test++; // cpu load calibration
  //  }

  Bg_Loop_Cnt++;

  wdt_reset();

  if (Button_Released)
  {
    Button_Released = false;
    //------------------------
    if (saved == false)
    {
      Storage_Voltage_Index++;
      if (Storage_Voltage_Index >= STORAGE_VOLTAGE_SIZE)
      {
        Storage_Voltage_Index = 0;
      }
      Rotary_Voltage = Storage_Voltage[Storage_Voltage_Index];
      Change_Voltage(Rotary_Voltage);
    }
    else
    {
      saved = false;
      tft.setTextDatum(TL_DATUM);
      tft.setTextColor(TFT_CYAN, TFT_BLACK);
      tft.drawString("         ", 72, 110, 2);
    }
    //------------------------
  }
  if (Button_Pressed)
  {
    Button_Pressed = false;
    //------------------------

    //------------------------
  }
  if (Button_Long_Pressed)
  {
    Button_Long_Pressed = false;
    //------------------------
    Storage_Voltage[Storage_Voltage_Index] = Rotary_Voltage;
    saved = true;
    //------------------------
  }
  //--------------------------------------------------------------
  if (Measure_Refresh)
  {
    Measure_Refresh = false;
    //--------------------------------------
    dac.setVoltage(Dac_Voltage, false);
    //--------------------------------------
    Current_Limit_Value = analogRead(A0) * 2.93255;// 1023/3;

    //     Current_Limit_Value = analogRead(A0) ;
    //     Current_Limit_Value = LIMIT_CORRECT( (double)analogRead(A0) / 46.5);

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
    Average_Current_Total += Measure_Current_mA;
    Average_Current_Count++;
    Display_Refresh = true;
  }
  //--------------------------------------------------------------
  if (Display_Refresh)
  {
    Display_Refresh = false;

    Display_Draw_Digits();

    if (Display_Update)
    {
      //////////////////////////////////////////////////////////////////////////////
      //############################################################################
      //////////////////////////////////////////////////////////////////////////////
      Measure_Value_Temp = (Voltage_Max_Value - Voltage_Min_Value) / 5;
      for (i = 0; i < 90; i += 1)
      {
        Trace(tft, i, Voltage_Draw_Buffer[i], 2, 80, 80, 440, 90, 0, 90, 5, Voltage_Min_Value, Voltage_Max_Value, Measure_Value_Temp, Volt_Graph_Update_State, TFT_BLACK , VOLTAGE_POZITION);
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
      for (i = 0; i < 90; i += 1)
      {
        Trace(tft, i, Voltage_Draw_Buffer[i], 2, 80, 80, 440, 90, 0, 90, 5, Voltage_Min_Value, Voltage_Max_Value, Measure_Value_Temp, Volt_Graph_Update_State, TFT_CYAN , VOLTAGE_POZITION);
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
      Measure_Value_Temp = (Current_Max_Value - Current_Min_Value) / 5;
      for (i = 0; i < 90; i += 1)
      {
        Trace(tft, i, Current_Draw_Buffer[i], DB_Character, 80, 80, 440, 90, 0, 90, 5, Current_Min_Value, Current_Max_Value, Measure_Value_Temp, Current_Graph_Update_State, TFT_BLACK , CURRENT_POZITION);
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
      for (i = 0; i < 90; i += 1)
      {
        Trace(tft, i, Current_Draw_Buffer[i], DB_Character, 80, 80, 440, 90, 0, 90, 5, Current_Min_Value, Current_Max_Value, Measure_Value_Temp, Current_Graph_Update_State, TFT_YELLOW , CURRENT_POZITION);
      }
      //////////////////////////////////////////////////////////////////////////////
      //############################################################################
      //////////////////////////////////////////////////////////////////////////////
    }
    Set_StepDown(Voltage_Max_Value);
    Display_Update = !Display_Update;
  }
  //--------------------------------------------------------------------------
  if (Second_Procces)
  {
    Second_Procces = false;

    Temperature_Array[Temperature_Index] = (analogRead(A1) * TEMPERATURE_CAL) - 273.15; // 5V referanslı adc ye göre LM335 için sıcaklık hesabı.
    Temperature_Index = (Temperature_Index + 1) % Temperature_FILTER_SN;
    Temperature_Temp = 0;
    for (int i = 0; i < Temperature_FILTER_SN; i++)
    {
      Temperature_Temp += Temperature_Array[i];
    }
    Temperature_Temp /= Temperature_FILTER_SN;
    Temperature = Temperature_Temp;

    if ((Display_Charge_Mode) && (Charge_Voltage_Limit <= Measure_Load_Voltage * 100))
    {
      Charge_Finish = true;
    }

    Calculate_Current_mAh += (Measure_Current_mA / 3600);

    Eeprom_Save();

    Prev_Current_Mode = 0; // refresh init ina219 periodicly

    //--------------------------
    Average_Current = Average_Current_Total / Average_Current_Count;
    //--------------------------
    if (0)
    {
      Serial.print("T: "); Serial.print(Time_Hour); Serial.print(':'); Serial.print(Time_Minute); Serial.print(':'); Serial.print(Time_Second); Serial.println(" Sn");
      Serial.print("V: "); Serial.print(Measure_Load_Voltage); Serial.println(" ");
      Serial.print("U: "); Serial.print(Measure_Current_mA); Serial.println(" ");
      Serial.print("L: "); Serial.print(Current_Limit_Value); Serial.println(" ");
    }
  }
  //--------------------------------------------------------------------------
  if (Minute_Procces)
  {
    Minute_Procces = false;

    tft.fillScreen(TFT_BLACK);

  }
  //--------------------------------------------------------------------------
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////




