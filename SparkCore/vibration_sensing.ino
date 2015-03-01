/*
 * This is some sample code for interacting with the MMA8452Q Accelerometer Device.
 * Please, read the README before trying to use this code.
 * The code in this file was writen to help you out, since it is a fairly complex problem
 * for programming beginners.
 * The accelerometer provides data in a certain and you must interpret it and convert it into
 * an accurate podometer, or step counter that outputs into the seven segment displays provided.
 *
 * I recommend taking your time working through the problem and actually understanding what
 * you are doing and how does a serial communication protocol such as I2C works.
 *
 * Good luck and have fun!
 *
 * Gabriel Hruskovec - ECE Pulse Competitions Director
 */

#include "Wire.h" // I2C Library for Arduino -- http://arduino.cc/en/reference/wire

#define BAUD	9600 // the console
#define MMA8452_ADDRESS 0x1D // address of slave device (accelerometer)

// define register addresses from datasheet
// https://d1b10bmlvqabco.cloudfront.net/attach/hrioh7n8sw44u/gxkzn61rklm20d/hrp4ssnzywag/Accelerometer_MMA8452Q_Datasheet.pdf
#define OUT_X_MSB 0x01 // x-axis data register [7:0] are 8 MSBs of 12-bit sample
#define OUT_X_LSB 0x02 // x-axis data register [7:4] are 4 LSBs of 12-bit sample (e.g. 1101 0000)
#define OUT_Y_MSB 0x03 // y-axis data register
#define OUT_Y_LSB 0x04 // y-axis data register
#define OUT_Z_MSB 0x05 // z-axis data register
#define OUT_Z_LSB 0x06 // z-axis data register

#define XYZ_DATA_CFG  0x0E
#define WHO_AM_I   0x0D
#define CTRL_REG1  0x2A
#define GSCALE 2
#define TRUE 1
#define FALSE 0


#define sq(x) ((x)*(x))


// declare Global Variables
int i = 0; // i counts the number of data points
int threshold = 120; // the threshold above which we count a step
int vectorMagnitude = 0;
boolean aboveThreshold = FALSE; // holds whether the current data point is above threshold
//byte commandByte = 0x67; // controls the program while running
                         // ASCII 0x67 "g" means go.  0x73 "s" means stop.  0x72 "r" means reset count

// Arduino requires a setup function that initializes states and protocols to be used
// It only runs when the Arduino is flashed or reset
void setup()
{
  // This is for using the serial port. It can be opened with CTRL+SHIFT+M or CMD+SHIFT+M (Mac)
  // Recommended for debugging - it can be used for printf statements
  //Serial.begin(BAUD);
  //Serial.println("Welcome to the ECE Pulse Intro to Embedded Competition!");
  Spark.function("getAccelData", getAccelData);
  Spark.variable("accel", &vectorMagnitude, INT);

    // Start I2C protocol as Master
  Wire.begin();

  // Initialize the Accelerometer
  initMMA84();
}

void loop()
{
  // Arduino requires a loop function that executes as the code runs
  // It is same in practice as a while(1)
  //if(commandByte == "s")
  //{
    int xData = readDataPoint(OUT_X_MSB);
    int yData = readDataPoint(OUT_Y_MSB);
    int zData = readDataPoint(OUT_Z_MSB);
    vectorMagnitude = sqrt( sq(xData) + sq(yData) + sq(zData) );

    /*
    if(vectorMagnitude > threshold)
      aboveThreshold = TRUE; // TRUE if above threshold
    else
      aboveThreshold = FALSE; // FALSE when not above threshold
      */
    i++;
    delay(100); // wait a bit between iterations
  //}



}

// function declaration
int getAccelData(String commandByte)
{
  // Your code here to read data from the Accelerometer
  // You should call readRegisters here
  if(commandByte == "s")
  {
    int xData = readDataPoint(OUT_X_MSB);
    int yData = readDataPoint(OUT_Y_MSB);
    int zData = readDataPoint(OUT_Z_MSB);
    vectorMagnitude = sqrt( sq(xData) + sq(yData) + sq(zData) );

    /*
    if(vectorMagnitude > threshold)
      aboveThreshold = TRUE; // TRUE if above threshold
    else
      aboveThreshold = FALSE; // FALSE when not above threshold
      */
    i++;
    delay(50); // wait a bit between iterations
  }

  //if(commandByte == 0x73) // if input is "s"...stop
  //{
  //	while(1){ /* infinite loop if stop */}
  //}

  return vectorMagnitude;
}

// Initialize the MMA8452 registers
// See the many application notes for more info on setting all of these registers:
// http://www.freescale.com/webapp/sps/site/prod_summary.jsp?code=MMA8452Q
// You shouldn't touch this code
void initMMA84()
{
  byte c = readRegister(WHO_AM_I);  // Read WHO_AM_I register
  if (c == 0x2A) // WHO_AM_I should always be 0x2A
  {
    //Serial.println("MMA8452Q is online...");
  }
  else
  {
    //Serial.print("Could not connect to MMA8452Q: 0x");
    //Serial.println(c, HEX);
    //while(1) ;
    // Loop forever if communication doesn't happen
  }

  // Must be in standby to change registers
  MMA8452Standby();

  // Set up the full scale range to 2, 4, or 8g.
  byte fsr = GSCALE;
  if(fsr > 8) fsr = 8;
  fsr >>= 2;
  writeRegister(XYZ_DATA_CFG, fsr);

  // Set to active to start reading
  MMA8452Active();
}

// Sets the MMA8452 to standby mode. It must be in standby to change most register settings
void MMA8452Standby()
{
  byte c = readRegister(CTRL_REG1); // maybe instead of ctrl_reg1, use 0x0B as system control reg
  writeRegister(CTRL_REG1, c & ~(0x01)); //Clear the active bit to go into standby
}

// Sets the MMA8452 to active mode. Needs to be in this mode to output data
void MMA8452Active()
{
  byte c = readRegister(CTRL_REG1);
  writeRegister(CTRL_REG1, c | 0x01); //Set the active bit to begin detection
}

// Read bytesToRead sequentially, starting at addressToRead into the dest byte array
void readRegisters(byte addressToRead, int bytesToRead, byte * dest)
{
  // Your code here to read multiple bytes

  for(int i = 0; i < bytesToRead; i++) // loop through each byte
  {
    dest[i] = readRegister(addressToRead); // read each byte and store it in dest[i]
    addressToRead++; // increment address
  }
}

// Function definition
// readDataPoint
// Reads top 8 bits (e.g. OUT_X_MSB) and bottom 4 bits (OUT_X_LSB) from accelerometer
// and combines them into one integer data point
// @param msbAddress - Address of register holding 8 most significant bits (e.g. OUT_X_MSB)
// @return - an integer containing a data point along one axis

int readDataPoint(byte msbAddress)
{
  int dataPoint = 0; // declare integer variable to hold combined data point
  byte msbData = readRegister(msbAddress); // reads top 8 bits into msbData
  byte lsbData = readRegister(msbAddress + 1); // reads bottom 4 bits into lsbData
  byte signCheck = 128; // 1000 0000 in binary

  if( (signCheck & msbData) == 128 ) // if sign bit is a 1
    dataPoint = -1; // fill dataPoint bits with 1

  // copy bits from msbData into dataPoint
  for(int j = 0; j < 8; j++)
  {
    dataPoint <<= 1; // make room for the next bit to come in
    if( (signCheck & msbData) == 128 ) // if most sig bit is 1
      dataPoint = dataPoint | 1; // set dataPoint lsb to 1
    msbData <<= 1; // shift bits one place
  }

  // copy bits from lsbData into dataPoint
  for(int j = 0; j < 4; j++)
  {
    dataPoint <<= 1; // make room for the next bit to come in
    if( (signCheck & lsbData) == 128 ) // if most sig bit is 1
      dataPoint = dataPoint | 1; // set dataPoint lsb to 1
    lsbData <<= 1; // shift bits one place
  }

  dataPoint = dataPoint / 10; // divide by 10 to avoid overflow

  return dataPoint;
}

// Function definition
// Read a single byte from addressToRead and return it as a byte
//    Writes from accelerometer to Arduino, then reads from Arduino?
byte readRegister(byte addressToRead)
{
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToRead);   // writes from accelerometer to Arduino?
  Wire.endTransmission(false); // endTransmission but keep the connection active

  Wire.requestFrom(MMA8452_ADDRESS, 1); // Ask for 1 byte, once done, bus is released by default

  while(!Wire.available()) ; // Wait for the data to come back
  return Wire.read(); // Return this one byte
}

// Writes a single byte (dataToWrite) into addressToWrite
void writeRegister(byte addressToWrite, byte dataToWrite)
{
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToWrite);
  Wire.write(dataToWrite);
  Wire.endTransmission();
}
