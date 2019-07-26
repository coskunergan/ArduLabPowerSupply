////////////////////////////////////////////////////////////////////////////////
void ISR_Time_Tick(void) // per 10mS
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
void ISR_doEncoder()
{
  if (Display_Charge_Mode == false)
  {
    if (digitalRead(encoder0PinA) == digitalRead(encoder0PinB))
    {
      if(Rotary_Voltage < 16384)
      {
        Rotary_Voltage++;
      }
      Rotary_Up_Flag = true;
    }
    else if (Rotary_Voltage)
    {
      Rotary_Voltage--;
      Rotary_Up_Flag = false;
    }      
    Change_Voltage(Rotary_Voltage);
  }
  else
  {
    if (digitalRead(encoder0PinA) == digitalRead(encoder0PinB))
    {
      if(Rotary_Limit < 12000)
      {
        Rotary_Limit++;
      }
      Rotary_Up_Flag = true;
    }
    else if(Rotary_Limit)
    {
      Rotary_Limit--;
      Rotary_Up_Flag = false;
    }     
    Change_Limit(Rotary_Limit);
  }  
}
////////////////////////////////////////////////////////////////////////////////
