#include <Arduino.h>
#include <i2c_t3.h>

#define MILLIBAR 0
#define BAR 1
#define PSI 2 

class HoneywellPressureSensor
{
    private:
        int _i2c_address;
        float _range;
        int _unit;
        int _output_unit;
        bool _differential;
        int _data;

    public:
        HoneywellPressureSensor();
        void configure_sensor(int i2c_address, float range, int unit, int differential, int output_unit, int position);
        void assign_I2C_address(int i2c_adddress);
        void assign_range(float range);
        void assign_unit(int unit);
        void assign_output_unit(int output_unit);
        void assign_differential(bool differential);
        void assign_channel(int channel_in);
        int read_data();
        float data2pressure();
        float read_pressure();
        void display_data();
        int channel;
        float pressure;  
        const float _unitConversion[3][3] = {{1,0.001,68.947572932},{1000,1,0.0689475729},{14.503773773,0.0145037738,1}};  
};

class PRSmodule
{
private:
    int _i2cAddress;
    int _i;
    int _sensorCount;
    int _ChannelNo[8];
    HoneywellPressureSensor _Sensor[8];
    byte _channelByte;
    int _rawData[8];
    byte _sensor_i2c_address[8] = {0,0,0,0,0,0,0,0};

public:
    PRSmodule(int i2cAddress);
    void config(byte sensorPositions, float PressureRange[8], byte diff, int UnitSensor[8], int UnitReq[8], byte sensor_i2c_address[8], bool checkFault = false);
    void switchToChannel(int ChannelNo);
    void closeAllChannels();
    float readPressSingle(int ChannelNo);  
    void readPressAll();
    int SensorCount;
    int rawData[8];
    int rawDataSingle;
    float Pressure[8];
    float PressureSingle;
    byte default_i2c_address[8] = {0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28};
    

};




