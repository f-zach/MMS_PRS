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

// Assigns the unit to the sensor
//    input unit    →    MILLIBAR or 0 for millibars
//                       BAR or 1 for bars
//                       PSI or 2 for psi
void HoneywellPressureSensor::assign_unit(int unit)
{
    _unit = unit;
}

// Assigns a boolean value if the sensor is differential or not
//    input: output_unit   →    MILLIBAR or 0 for millibars
//                              BAR or 1 for bars
//                              PSI or 2 for psi
void HoneywellPressureSensor::assign_differential(bool differential)
{
    _differential = differential;
}

// Assigns th unit that the user wants to be output by the library

void HoneywellPressureSensor::assign_output_unit(int output_unit)
{
    _output_unit = output_unit;
}

void HoneywellPressureSensor::assign_channel(int channel_in)
{
    channel = channel_in;
}

void HoneywellPressureSensor::configure_sensor(int i2c_address, float range, int unit, int differential, int output_unit, int position)
{
    assign_I2C_address(i2c_address);
    assign_range(range);
    assign_unit(unit);
    assign_differential(differential);
    assign_output_unit(output_unit);
    assign_channel(position);
}

int HoneywellPressureSensor::read_data()
{
    Wire.requestFrom(_i2c_address, 2);
    _data = (Wire.read() << 8 | Wire.read()) & 0x3FFF;

    return _data;
}

float HoneywellPressureSensor::data2pressure()
{

    pressure = ((((_data - 1638) * (_range - (-_range * _differential))) / 13107) - (_range * _differential)) * _unitConversion[_unit][_output_unit];

    return pressure;
}

float HoneywellPressureSensor::read_pressure()
{
    read_data();
    data2pressure();

    return pressure;
}

void HoneywellPressureSensor::display_data()
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

void PRSmodule::config(byte sensorPositions, float PressureRange[8], byte diff, int UnitSensor[8], int UnitReq[8], byte sensor_i2c_address[8], bool checkFault)
{
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

void PRSmodule::switchToChannel(int ChannelNo)
{
    _channelByte = B00000001 << ChannelNo;

    Wire.beginTransmission(_i2cAddress);
    Wire.write(_channelByte);
    Wire.endTransmission();
}

void PRSmodule::closeAllChannels()
{
    Wire.beginTransmission(_i2cAddress);
    Wire.write(0);
    Wire.endTransmission();
}

float PRSmodule::readPressSingle(int sensor_no)
{
    switchToChannel(_Sensor[sensor_no].channel);

    PressureSingle = _Sensor[sensor_no].read_pressure();

    closeAllChannels();

    return PressureSingle;
}

void PRSmodule::readPressAll()
{
    for (int i = 0; i < SensorCount; i++)
    {
        Pressure[i] = readPressSingle(i);
    }
}
