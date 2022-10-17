#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

int SoC = 0;


int StateofCharge(float volt_bat){
  SoC = 268.497 * pow(volt_bat,8) + -6879.270 * pow(volt_bat,7) + 76716.496 * pow(volt_bat,6) - 486365.673 * pow(volt_bat,5) + 1917287.171 * pow(volt_bat,4) - 4812471.066 * pow(volt_bat,3) + 7511312.301 * pow(volt_bat,2) - (6665390.783 * volt_bat) + 2574719.230;
   if (volt_bat <= 2.6){
  SoC = 0;
 }
 if (volt_bat >=4.0){
  SoC = 100;
 }
  return SoC;
}



void setup(void) 
{
  Serial.begin(115200);
  while (!Serial) {
      // will pause Zero, Leonardo, etc until serial console opens
      delay(1);
  }

  uint32_t currentFrequency;

  if (! ina219.begin()) {
   // Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }
}

void loop(void) 
{
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;
  float ind_voltage = 0;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  ind_voltage = busvoltage/2;
 
 Serial.print("SoC:");
 Serial.print(StateofCharge(ind_voltage));
 Serial.println();

 delay(1000);
}