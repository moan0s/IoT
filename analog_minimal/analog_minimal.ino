const int Analog_channel_pin= 4; //15
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
voltage_value = (ADC_VALUE * 3.3 ) / (4095);
Serial.print("Voltage = ");
Serial.print(voltage_value);
Serial.println("volts");
delay(100);
}
