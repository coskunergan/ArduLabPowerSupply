////////////////////////////////////////////////////////////////////////////////
void ISR_Time_Tick(void) // per 10mS
{
  //----------------------
  // Per 10mS region
  Button_Procces();
  Rotary_VeryFast_Mode = 0;
  //----------------------
  // Per 100mS region
  if ((Second_Counter % 6) == 0)
  {
    Measure_Refresh = true;
    Rotary_Fast_Mode = 0;
  }
  //----------------------
  // Per second region
  if (Second_Counter >= 100)
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
  // Per 500mS region
  if ((Second_Counter % 50) == 0)
  {
    CPU_Load_Calc();
  }
  //----------------------
  Second_Counter++;
  //----------------------
}
////////////////////////////////////////////////////////////////////////////////
void Change_Limit(int val)
{
  Charge_Voltage_Limit = (val / 2.5);
}
////////////////////////////////////////////////////////////////////////////////
void Change_Voltage(int val)
{
  Dac_Voltage = (val / 2.5);
}
////////////////////////////////////////////////////////////////////////////////
byte Rotary_Speed_Check(void)
{
  Rotary_VeryFast_Mode++;
  Rotary_Fast_Mode++;
  if (Rotary_VeryFast_Mode > 3)
  {
    return 32;
  }
  //--------------
  if (Rotary_Fast_Mode > 3)
  {
    return 8;
  }
  //----------------
  return 1;
}
////////////////////////////////////////////////////////////////////////////////
void ISR_doEncoder()
{
  byte Rotary_Turn_x = Rotary_Speed_Check();
  //----------------
  if (Display_Charge_Mode == false)
  {
    if (digitalRead(encoder0PinA) == digitalRead(encoder0PinB))
    {
      if (Rotary_Voltage < (10240 - Rotary_Turn_x))
      {
        Rotary_Voltage += Rotary_Turn_x;
      }
    }
    else if ((Rotary_Voltage - Rotary_Turn_x) >= 0)
    {
      Rotary_Voltage -= Rotary_Turn_x;
    }
    Change_Voltage(Rotary_Voltage);
  }
  else
  {
    if (digitalRead(encoder0PinA) == digitalRead(encoder0PinB))
    {
      if (Rotary_Limit < (7500  - Rotary_Turn_x))
      {
        Rotary_Limit += Rotary_Turn_x;
      }
    }
    else if ((Rotary_Limit - Rotary_Turn_x) >= 0)
    {
      Rotary_Limit -= Rotary_Turn_x;
    }
    Change_Limit(Rotary_Limit);
  }
}
////////////////////////////////////////////////////////////////////////////////
