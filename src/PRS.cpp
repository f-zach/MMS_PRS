#include "PRS.h"
HoneywellPressureSensor::HoneywellPressureSensor()
{
}

// Assigns the I2C address to the sensor
void HoneywellPressureSensor::assign_I2C_address(int i2c_address)
{
    _i2c_address = i2c_address;
}

// Assigns the measurement range value to the sensor (for differential sensors only the upper limit)
void HoneywellPressureSensor::assign_range(float range)
{
    _range = range;
}

/*
    Assigns the unit to the sensor
    input unit    →    MILLIBAR or 0 for millibars
                       BAR or 1 for bars
                       PSI or 2 for psi
*/
void HoneywellPressureSensor::assign_unit(int unit)
{
    _unit = unit;
}

// Assigns a boolean value if the sensor is differential or not
void HoneywellPressureSensor::assign_differential(bool differential)
{
    _differential = differential;
}

// Assigns th unit that the user wants to be output by the library

//    output_unit:  MILLIBAR or 0 for millibars, BAR or 1 for bars, PSI or 2 for psi
void HoneywellPressureSensor::assign_output_unit(int output_unit)
{
    _output_unit = output_unit;
}

/*
 Assigns a channel number to the sensor (for use with TCA9548 I²C-multiplexer)

 channel_in: Channel number from 0 to 7
*/
void HoneywellPressureSensor::assign_channel(int channel_in)
{
    channel = channel_in;
}

/*
    Assigns values to all variables required for sensor opertaion

    i2c_address:    Sensor I²C-address
    range:          Sensor measurement range
    unit:           Sensor output unit (MILLIBAR or 0 for millibar; BAR or 1 for bar; PSI or 2 for PSI)
    differential:   Sensor measurement method (false: absolute,relative; true: differential)
    output_unit:    User reqired output unit for output pressure values (MILLIBAR or 0 for millibar; BAR or 1 for bar; PSI or 2 for PSI)
    channel:        I²C-multiplexer channel (values 0 to 7)
*/
void HoneywellPressureSensor::configure_sensor(int i2c_address, float range, int unit, int differential, int output_unit, int channel)
{
    assign_I2C_address(i2c_address);
    assign_range(range);
    assign_unit(unit);
    assign_differential(differential);
    assign_output_unit(output_unit);
    assign_channel(channel);
}

// Read the unconverted digital pressure data from the sensor
int HoneywellPressureSensor::read_data()
{
    Wire.requestFrom(_i2c_address, 2);

    // The data is read and the first two bits of the MSB are set to zero, since they can contain unwanted data
    _data = (Wire.read() << 8 | Wire.read()) & 0x3FFF;

    return _data;
}

// Converts the digital pressure data to a float pressure value. The output pressure unit ist defined by the user in the config function or with assign_output_unit
float HoneywellPressureSensor::data2pressure()
{
    pressure = ((((_data - 1638) * (_range - (-_range * _differential))) / 13107) - (_range * _differential)) * _unitConversion[_unit][_output_unit];

    return pressure;
}

// Reads the data from the sensor and converts it
float HoneywellPressureSensor::read_pressure()
{
    read_data();
    data2pressure();

    return pressure;
}

// Displays the configuration data of the sensor via Serial
void HoneywellPressureSensor::display_config_data()
{
    Serial.print("I2C-Adresse: ");
    Serial.println(_i2c_address, HEX);
    Serial.print("Range: ");
    Serial.println(_range);
    Serial.print("Unit: ");
    Serial.println(_unit);
    Serial.print("Differential: ");
    Serial.println(_differential);
}

PRSmodule::PRSmodule(int i2cAddress)
    : _i2cAddress(i2cAddress)
{
}




/*
Configures the PRS module

Inputs:     sensorPositions         Byte containing the sensor positions on the module
                                    0 → not occupied, 1 → occupied
                                    e.g.: 10011100 → Sensor positions 3,4,5 and 8 are occupied
            
            PressureRange           Array with a size of eight containing the unitless value of the pressure range of the sensors (for differential the absolute amount).
                                    The values are put into the corresponding place of the array, e.g. PressureRange[5] = 100 → The unitless pressure range of the sensor in 
                                    position 6 (array indexing!) is 100.

            diff                    Byte containing information, if the sensor in a certain slot is a differential or a absolute/relative sensor
                                    0 → absolute/relative Sensor, 1 → differential sensor
                                    e.g.: 00001111 → sensors 1,2,3,4 are differential

            unit sensors            
                                            
*/
void PRSmodule::config(byte sensorPositions, float PressureRange[8], byte diff, int UnitSensor[8], int UnitReq[8], byte sensor_i2c_address[8], bool checkFault)
{

    _sensorCount = 0;

    // Create the sensor array containing
    for (_i = 0; _i < 8; _i++)
    {
        if (bitRead(sensorPositions, _i))
        {
            _Sensor[_sensorCount].configure_sensor(sensor_i2c_address[_i], PressureRange[_i], UnitSensor[_i], bitRead(diff, _i), UnitReq[_i], _i);

            _sensorCount++;
        }
    }

    SensorCount = _sensorCount;
}

/*
    Switch the I²C-switch of the PRS module to a desired channel

    Inputs:     ChannelNo           Number of the channel to switch to (0-7)
*/
void PRSmodule::switchToChannel(int ChannelNo)
{
    _channelByte = B00000001 << ChannelNo;

    Wire.beginTransmission(_i2cAddress);
    Wire.write(_channelByte);
    Wire.endTransmission();
}

/*
    Close all channels of the I²C-switch
*/
void PRSmodule::closeAllChannels()
{
    Wire.beginTransmission(_i2cAddress);
    Wire.write(0);
    Wire.endTransmission();
}

/*
    Read the pressure value of a single pressure sensor 

    Inputs:     sensorNo            Number of the desired sensor to read

    Outputs:    PressureSingle      Pressure value read and converted from the sensor
*/
float PRSmodule::readPressSingle(int sensorNo)
{
    switchToChannel(_Sensor[sensorNo].channel);

    PressureSingle = _Sensor[sensorNo].read_pressure();

    closeAllChannels();

    return PressureSingle;
}

/*
    Read all sensors installed on the module. This function will automatically read all sensors which were configured with the config function
*/
void PRSmodule::readPressAll()
{
    for (int i = 0; i < SensorCount; i++)
    {
        Pressure[i] = readPressSingle(i);
    }
}
