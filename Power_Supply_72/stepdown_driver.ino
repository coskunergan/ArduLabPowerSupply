#define VOLT_TRACE_LIMIT  3.0
//////////////////////////////////////////////////////////////////////////////
void Set_StepDown(float voltage)
{
  StepDown_Step = voltage + VOLT_TRACE_LIMIT;

  StepDown_Step /= 2;
  
  if (StepDown_Step > 15)
  {
    StepDown_Step = 15;
  }
  StepDown_Step ^= 0xF;

  if ((StepDown_Step & 8) == 8)
  {
    digitalWrite(StepDown_SetVoltage_Pin1, HIGH);    // 8K
  }
  else
  {
    digitalWrite(StepDown_SetVoltage_Pin1, LOW);    // 8K
  }
  if ((StepDown_Step & 4) == 4)
  {
    digitalWrite(StepDown_SetVoltage_Pin2, HIGH);    // 4K
  }
  else
  {
    digitalWrite(StepDown_SetVoltage_Pin2, LOW);    // 4K
  }
  if ((StepDown_Step & 2) == 2)
  {
    digitalWrite(StepDown_SetVoltage_Pin3, HIGH);    // 2K
  }
  else
  {
    digitalWrite(StepDown_SetVoltage_Pin3, LOW);    // 2K
  }
  if ((StepDown_Step & 1) == 1)
  {
    digitalWrite(StepDown_SetVoltage_Pin4, HIGH);    // 1K
  }
  else
  {
    digitalWrite(StepDown_SetVoltage_Pin4, LOW);    // 1K
  }
}
//////////////////////////////////////////////////////////////////////////////
