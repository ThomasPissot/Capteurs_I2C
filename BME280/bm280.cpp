#include "bm280.h"

using namespace std;


bm280::bm280(int i2cAddress)  	// Le constructeur
{
    fd = wiringPiI2CSetup(i2cAddress);
    if(fd < 0) {
       cout << "Device not found" << endl;
    }

  readCalibrationData();

  wiringPiI2CWriteReg8(fd, 0xf2, 0x01);   // humidity oversampling x 1
  wiringPiI2CWriteReg8(fd, 0xf4, 0x25);   // pressure and temperature oversampling x 1, mode normal


}

void bm280::readCalibrationData() {
  cal.dig_T1 = (uint16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_T1);
  cal.dig_T2 = (int16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_T2);
  cal.dig_T3 = (int16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_T3);

  cal.dig_P1 = (uint16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_P1);
  cal.dig_P2 = (int16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_P2);
  cal.dig_P3 = (int16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_P3);
  cal.dig_P4 = (int16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_P4);
  cal.dig_P5 = (int16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_P5);
  cal.dig_P6 = (int16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_P6);
  cal.dig_P7 = (int16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_P7);
  cal.dig_P8 = (int16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_P8);
  cal.dig_P9 = (int16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_P9);

  cal.dig_H1 = (uint8_t)wiringPiI2CReadReg8(fd, BME280_REGISTER_DIG_H1);
  cal.dig_H2 = (int16_t)wiringPiI2CReadReg16(fd, BME280_REGISTER_DIG_H2);
  cal.dig_H3 = (uint8_t)wiringPiI2CReadReg8(fd, BME280_REGISTER_DIG_H3);
  cal.dig_H4 = (wiringPiI2CReadReg8(fd, BME280_REGISTER_DIG_H4) << 4) | (wiringPiI2CReadReg8(fd, BME280_REGISTER_DIG_H4+1) & 0xF);
  cal.dig_H5 = (wiringPiI2CReadReg8(fd, BME280_REGISTER_DIG_H5+1) << 4) | (wiringPiI2CReadReg8(fd, BME280_REGISTER_DIG_H5) >> 4);
  cal.dig_H6 = (int8_t)wiringPiI2CReadReg8(fd, BME280_REGISTER_DIG_H6);
}

void bm280::getRawData() {
  wiringPiI2CWrite(fd, 0xf7);

  raw.pmsb = wiringPiI2CRead(fd);
  raw.plsb = wiringPiI2CRead(fd);
  raw.pxsb = wiringPiI2CRead(fd);

  raw.tmsb = wiringPiI2CRead(fd);
  raw.tlsb = wiringPiI2CRead(fd);
  raw.txsb = wiringPiI2CRead(fd);

  raw.hmsb = wiringPiI2CRead(fd);
  raw.hlsb = wiringPiI2CRead(fd);

  raw.temperature = 0;
  raw.temperature = (raw.temperature | raw.tmsb) << 8;
  raw.temperature = (raw.temperature | raw.tlsb) << 8;
  raw.temperature = (raw.temperature | raw.txsb) >> 4;

  raw.pressure = 0;
  raw.pressure = (raw.pressure | raw.pmsb) << 8;
  raw.pressure = (raw.pressure | raw.plsb) << 8;
  raw.pressure = (raw.pressure | raw.pxsb) >> 4;

  raw.humidity = 0;
  raw.humidity = (raw.humidity | raw.hmsb) << 8;
  raw.humidity = (raw.humidity | raw.hlsb);
}

int32_t bm280::getTemperatureCalibration()
{
  getRawData();

  int32_t var1  = ((((raw.temperature>>3) - ((int32_t)cal.dig_T1 <<1))) * ((int32_t)cal.dig_T2)) >> 11;

  int32_t var2  = (((((raw.temperature>>4) - ((int32_t)cal.dig_T1)) * ((raw.temperature>>4) - ((int32_t)cal.dig_T1))) >> 12) * ((int32_t)cal.dig_T3)) >> 14;

  return var1 + var2;
}

float bm280::obtenirTemperatureEnC()
{
  int32_t t_fine = getTemperatureCalibration();
  float T  = (t_fine * 5 + 128) >> 8;
  return T/100;
}

float bm280::obtenirTemperatureEnF()
{
	float output = obtenirTemperatureEnC();
	output = (output * 9) / 5 + 32;
	return output;
}




// retourne la pression en hPa
// Le capteur retourne la pression en Pa sur unsigned 32 bit integer avec le format Q24.8  (24 bits pour la partie entière et 8 bits pour la partie fractionnaire).
// 24674867 represente 24674867/256 = 96386.2 Pa ou 24674867/25600 963.862 hPa

float bm280::obtenirPression()
{
  int32_t t_fine = getTemperatureCalibration();
  int64_t var1, var2, p;

  var1 = ((int64_t)t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)cal.dig_P6;
  var2 = var2 + ((var1*(int64_t)cal.dig_P5)<<17);
  var2 = var2 + (((int64_t)cal.dig_P4)<<35);
  var1 = ((var1 * var1 * (int64_t)cal.dig_P3)>>8) + ((var1 * (int64_t)cal.dig_P2)<<12);
  var1 = (((((int64_t)1)<<47)+var1))*((int64_t)cal.dig_P1)>>33;

  if (var1 == 0) {
    return 0;  // exception provoquée par division par zéro
  }
  p = 1048576 - raw.pressure;
  p = (((p<<31) - var2)*3125) / var1;
  var1 = (((int64_t)cal.dig_P9) * (p>>13) * (p>>13)) >> 25;
  var2 = (((int64_t)cal.dig_P8) * p) >> 19;

  p = ((p + var1 + var2) >> 8) + (((int64_t)cal.dig_P7)<<4);
  return (float)p/25600;

}

// retourne le taux d'humidité relative en %

float bm280::obtenirHumidite()
{
  int32_t v_x1_u32r;
  int32_t t_fine = getTemperatureCalibration();


  v_x1_u32r = (t_fine - ((int32_t)76800));

  v_x1_u32r = (((((raw.humidity << 14) - (((int32_t)cal.dig_H4) << 20) - (((int32_t)cal.dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) *
              (((((((v_x1_u32r * ((int32_t)cal.dig_H6)) >> 10) *
              (((v_x1_u32r * ((int32_t)cal.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
              ((int32_t)2097152)) * ((int32_t)cal.dig_H2) + 8192) >> 14));

  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
              ((int32_t)cal.dig_H1)) >> 4));

  v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
  v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
  float h = (v_x1_u32r>>12);
  return  h / 1024.0;
}

// retourne l'altitude en métres

float bm280::obtenirAltitudeEnMetres(){
    //  Equation obtenue à partir du datasheet BMP180 (page 16):
    //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf
    //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064
    float pression =  obtenirPression();
    return 44330.0 * (1.0 - pow(pression / MEAN_SEA_LEVEL_PRESSURE, 0.190294957));
}

// retourne l'altitude en pieds

float bm280::obtenirAltitudeEnPieds(){
    float metre =  obtenirAltitudeEnMetres();
    return metre * 3.28084;

}
