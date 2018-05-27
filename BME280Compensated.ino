#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>
#include <SparkFunBME280.h>
#include <SparkFunCCS811.h>

/* CvdO 2018
 *  Combinatie van:
 *  Arduino
 *  CJMCU-811 air sensor (3.3V) I2C
 *  GY-BME280 temp and pressure sensor (3.3V) I2C
 *  ST7735 color TFT display 128x128 pixels
 *  BME280Compensated sketch from Sparkfun
 *  ST7735 graphictest sketch from Adafruit
 */

/******************************************************************************
  BME280Compensated.ino

  Marshall Taylor @ SparkFun Electronics

  April 4, 2017

  https://github.com/sparkfun/CCS811_Air_Quality_Breakout
  https://github.com/sparkfun/SparkFun_CCS811_Arduino_Library

  This example uses a BME280 to gather environmental data that is then used
  to compensate the CCS811.

  Hardware Connections (Breakoutboard to Arduino):
  3.3V to 3.3V pin
  GND to GND pin
  SDA to A4
  SCL to A5

  Resources:
  Uses Wire.h for i2c operation

  Development environment specifics:
  Arduino IDE 1.8.1

  This code is released under the [MIT License](http://opensource.org/licenses/MIT).

  Please review the LICENSE.md file included with this example. If you have any questions
  or concerns with licensing, please contact techsupport@sparkfun.com.

  Distributed as-is; no warranty is given.
******************************************************************************/

//#define CCS811_ADDR 0x5B //Default I2C Address
#define CCS811_ADDR 0x5A //Alternate I2C Address

#define PIN_NOT_WAKE 5

// Color definitions for the tft display
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0  
#define WHITE   0xFFFF


//The BME280 and the CJMCU811 are both on the I2C bus, in parallel
//use the hardware pins for clock (SCK) (arduino pin 13) and data (SDA) (arduino pin 11)
#define TFT_CS     10
#define TFT_RST    9  
#define TFT_DC     8 //DC is DATA/COMMAND or A0

//Global objects for sensors and tft
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);
CCS811 myCCS811(CCS811_ADDR);
BME280 myBME280;

int time=0;

void setup()
{
  Serial.begin(9600);
  Serial.println();
  tft.initR(INITR_144GREENTAB);   // initialize a ST7735S chip, green tab
  tft.setRotation(1);
  tft.fillScreen(YELLOW);
  tft.fillRect(1, 1, 126, 126, BLACK);
  tft.setTextWrap(false);
  tft.setTextColor(ST7735_YELLOW);
  tft.setTextSize(2);

  Serial.println("Apply BME280 data to CCS811 for compensation.");

  //This begins the CCS811 sensor and prints error status of .begin()
  CCS811Core::status returnCode = myCCS811.begin();
  Serial.print("CCS811 begin exited with: ");
  //Pass the error code to a function to print the results
  printDriverError( returnCode );
  Serial.println();

  //For I2C, enable the following and disable the SPI section
  myBME280.settings.commInterface = I2C_MODE;
  myBME280.settings.I2CAddress = 0x77;

  //Initialize BME280
  //For I2C, enable the following and disable the SPI section
  myBME280.settings.commInterface = I2C_MODE;
  myBME280.settings.I2CAddress = 0x77;
  myBME280.settings.runMode = 3; //Normal mode
  myBME280.settings.tStandby = 0;
  myBME280.settings.filter = 4;
  myBME280.settings.tempOverSample = 5;
  myBME280.settings.pressOverSample = 5;
  myBME280.settings.humidOverSample = 5;

  delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.

  //Calling .begin() causes the settings to be loaded
  myBME280.begin();


}
//---------------------------------------------------------------
void loop()
{
  //Check to see if data is available
  if (myCCS811.dataAvailable())
  {
    //Calling this function updates the global tVOC and eCO2 variables
    myCCS811.readAlgorithmResults();
    //printInfoSerial fetches the values of tVOC and eCO2
    //printInfoSerial();
    if(time%20==0) Serial.println("time(s)\ttemp(C)\tpressure(Pa)\thum(%)\tCO2(ppm)TVOC(ppb)");

    Serial.print(time);
    Serial.print("\t");
    Serial.print(myBME280.readTempC(), 2);
    Serial.print("\t");
    Serial.print(myBME280.readFloatPressure(), 2);
    Serial.print("\t");
    Serial.print(myBME280.readFloatHumidity(), 2);
    Serial.print("\t");
    Serial.print(myCCS811.getCO2());
    Serial.print("\t");
    Serial.println(myCCS811.getTVOC());

    const int x=8;
    int y=12;
    const int delty=21;
    //tft.fillRect(1, 1, 126, 126, BLACK);
    tft.setCursor(x, y);
    tft.fillRect(x-1, y, 48, delty-6, BLACK);
    tft.print(myBME280.readTempC(),1);
    tft.println(" C");
    y+=delty;

    tft.setCursor(x, y);
    tft.fillRect(x-1, y, 48, delty-6, BLACK);
    tft.print(myBME280.readFloatPressure()/100,0);
    tft.println(" mbar");
    y+=delty;

    tft.setCursor(x, y);
    tft.fillRect(x-1, y, 48, delty-6, BLACK);
    tft.print(myBME280.readFloatHumidity(), 1);
    tft.println("% hum");
    y+=delty;
    
    tft.setCursor(x, y);
    tft.fillRect(x-1, y, 48, delty-6, BLACK);
    tft.print(myCCS811.getCO2());
    tft.setCursor(x+48, y);
    tft.println("ppmCO2");
    y+=delty;
    
    tft.setCursor(x, y);
    tft.fillRect(x-1, y, 48, delty-6, BLACK);
    tft.print(myCCS811.getTVOC());
    tft.setCursor(x+36, y);
    tft.println("ppbTVOC");
  

    float BMEtempC = myBME280.readTempC();
    float BMEhumid = myBME280.readFloatHumidity();

/*
    Serial.print("Applying new values (deg C, %): ");
    Serial.print(BMEtempC);
    Serial.print(",");
    Serial.println(BMEhumid);
    Serial.println();
*/
    //This sends the temperature data to the CCS811
    myCCS811.setEnvironmentalData(BMEhumid, BMEtempC);
  }
  else if (myCCS811.checkForStatusError())
  {
    //If the CCS811 found an internal error, print it.
    printSensorError();
  }

  delay(2000); //Wait for next reading
  time+=2;
}

//---------------------------------------------------------------
void printInfoSerial()
{
  //getCO2() gets the previously read data from the library
  Serial.println("CCS811 data:");
  Serial.print(" CO2 concentration : ");
  Serial.print(myCCS811.getCO2());
  Serial.println(" ppm");

  //getTVOC() gets the previously read data from the library
  Serial.print(" TVOC concentration : ");
  Serial.print(myCCS811.getTVOC());
  Serial.println(" ppb");

  Serial.println("BME280 data:");
  Serial.print(" Temperature: ");
  Serial.print(myBME280.readTempC(), 2);
  Serial.println(" degrees C");

  Serial.print(" Temperature: ");
  Serial.print(myBME280.readTempF(), 2);
  Serial.println(" degrees F");

  Serial.print(" Pressure: ");
  Serial.print(myBME280.readFloatPressure(), 2);
  Serial.println(" Pa");

  Serial.print(" Pressure: ");
  Serial.print((myBME280.readFloatPressure() * 0.0002953), 2);
  Serial.println(" InHg");

  Serial.print(" Altitude: ");
  Serial.print(myBME280.readFloatAltitudeMeters(), 2);
  Serial.println("m");

  Serial.print(" Altitude: ");
  Serial.print(myBME280.readFloatAltitudeFeet(), 2);
  Serial.println("ft");

  Serial.print(" %RH: ");
  Serial.print(myBME280.readFloatHumidity(), 2);
  Serial.println(" %");

  Serial.println();


}

//printDriverError decodes the CCS811Core::status type and prints the
//type of error to the serial terminal.
//
//Save the return value of any function of type CCS811Core::status, then pass
//to this function to see what the output was.
void printDriverError( CCS811Core::status errorCode )
{
  switch ( errorCode )
  {
    case CCS811Core::SENSOR_SUCCESS:
      Serial.print("SUCCESS");
      break;
    case CCS811Core::SENSOR_ID_ERROR:
      Serial.print("ID_ERROR");
      break;
    case CCS811Core::SENSOR_I2C_ERROR:
      Serial.print("I2C_ERROR");
      break;
    case CCS811Core::SENSOR_INTERNAL_ERROR:
      Serial.print("INTERNAL_ERROR");
      break;
    case CCS811Core::SENSOR_GENERIC_ERROR:
      Serial.print("GENERIC_ERROR");
      break;
    default:
      Serial.print("Unspecified error.");
  }
}

//printSensorError gets, clears, then prints the errors
//saved within the error register.
void printSensorError()
{
  uint8_t error = myCCS811.getErrorRegister();

  if ( error == 0xFF ) //comm error
  {
    Serial.println("Failed to get ERROR_ID register.");
  }
  else
  {
    Serial.print("Error: ");
    if (error & 1 << 5) Serial.print("HeaterSupply");
    if (error & 1 << 4) Serial.print("HeaterFault");
    if (error & 1 << 3) Serial.print("MaxResistance");
    if (error & 1 << 2) Serial.print("MeasModeInvalid");
    if (error & 1 << 1) Serial.print("ReadRegInvalid");
    if (error & 1 << 0) Serial.print("MsgInvalid");
    Serial.println();
  }
}
