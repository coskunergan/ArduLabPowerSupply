////////////////////////////////////////////////////////////////////////////////
void CPU_Load_Calc( void )
{
  static unsigned long  prev_bg_loop_cnt = 0;
  unsigned long delta_cnt;
  static byte index = 0;
  byte idle_pct, i;

  delta_cnt = Bg_Loop_Cnt - prev_bg_loop_cnt;
  prev_bg_loop_cnt = Bg_Loop_Cnt;
  if ( delta_cnt > BG_LOOPS_PER_TASK )
  {
    delta_cnt = BG_LOOPS_PER_TASK;
  }
  idle_pct = (byte)( (255 * delta_cnt) / BG_LOOPS_PER_TASK );
  CPU_util_pct = 255 - idle_pct;
  CPU_util_array[index++] = CPU_util_pct;
  if (index == AVERAGE_CNT)
  {
    index = 0;
    delta_cnt = 0;
    for (i = 0; i < AVERAGE_CNT; i++)
    {
      delta_cnt += CPU_util_array[i];
    }
    CPU_Load_Avr = delta_cnt / AVERAGE_CNT;
  }
}
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
  if (!digitalRead(Switch_Pin2) || (Charge_Finish == true) || (Temperature >= MAX_Temperature))
  {
    Dac_Voltage = 0;
    Analog_Power_State = 0;
    Average_Current = Average_Current_Total = Average_Current_Count = 0;
  }
  else if (digitalRead(Switch_Pin1) && digitalRead(Switch_Pin2))
  {
    if (Analog_Power_State == 0 && (Temperature < (MAX_Temperature - 10)))
    {
      Change_Voltage(Rotary_Voltage);
      Analog_Power_State = 1;
    }
  }
  if (!digitalRead(Switch_Pin1))
  {
    if (Display_Charge_Mode == false)
    {
      Average_Current = Average_Current_Total = Average_Current_Count = 0;
    }
    Display_Charge_Mode = true;
  }
  else
  {
    if (Display_Charge_Mode == true)
    {
      Average_Current = Average_Current_Total = Average_Current_Count = 0;
    }
    Display_Charge_Mode = false;
    Charge_Finish = false;
  }
}
////////////////////////////////////////////////////////////////////////////////
