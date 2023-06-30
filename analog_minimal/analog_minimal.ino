const int Analog_channel_pin= 15;
int ADC_VALUE = 0;
int voltage_value = 0; 
void setup() 
{
Serial.begin(115200);
}
void loop() 
{
ADC_VALUE = analogRead(Analog_channel_pin);
Serial.print("ADC VALUE = ");
Serial.println(ADC_VALUE);
delay(1000);
voltage_value = (ADC_VALUE * 3.3 ) / (4095);
Serial.print("Voltage = ");
Serial.print(voltage_value);
Serial.print("volts");
delay(1000);
}
