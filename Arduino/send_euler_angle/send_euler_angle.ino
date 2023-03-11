#include "MPU9250.h"
#include "eeprom_utils.h"

#define FRAME_START     0xAA
#define FRAME_END       0xDD
#define FRAME_MAX_SIZE  16
#define DEG_2_RAD       0.01745329251994329576923690768489
#define DEAD_ZONE       3

typedef struct _data
{
  float euler_x;
  float euler_y;
  float euler_z;
  bool  ismove;
  bool  button;
} DATA;

const int buttonPin = 7; 

MPU9250 mpu;

void setup() {
    pinMode(buttonPin, INPUT);
    Serial.begin(115200);
    Wire.begin();
    delay(2000);

    mpu.setMagneticDeclination(-8.53);
    mpu.selectFilter(QuatFilterSel::MAHONY);
//    mpu.setFilterIterations(5);

    if (!mpu.setup(0x68)) {  // change to your own address
        while (1) {
            Serial.println("MPU connection failed. Please check your connection with `connection_check` example.");
            delay(5000);
        }
    }

#if defined(ESP_PLATFORM) || defined(ESP8266)
    EEPROM.begin(0x80);
#endif
    // load from eeprom
    loadCalibration();
}

DATA data;
byte buffer[FRAME_MAX_SIZE];

void loop() {
    if (mpu.update()) {
        static uint32_t prev_ms = millis();
        if (millis() > prev_ms + 1) {
            data.euler_x = DEG_2_RAD * mpu.getEulerX();
            data.euler_y = DEG_2_RAD * mpu.getEulerY();
            data.euler_z = -DEG_2_RAD * mpu.getEulerZ();
            data.ismove = is_move();
            data.button = digitalRead(buttonPin);
            write_bytes(buffer, &data);
            Serial.write(buffer, FRAME_MAX_SIZE);
            Serial.flush();
            prev_ms = millis();
        }
    }
}

bool is_move() {
    int temp;
    temp = fabs(mpu.getGyroX()) + fabs(mpu.getGyroY()) + fabs(mpu.getGyroZ());
    if (temp > DEAD_ZONE)
        return true;
    else
        return false;
}

void write_bytes(byte *buffer, DATA *data){
    buffer[0] = FRAME_START;
    memcpy(buffer + 1, &data->euler_x, 4);
    memcpy(buffer + 5, &data->euler_y, 4);
    memcpy(buffer + 9, &data->euler_z, 4);
    buffer[13] = data->ismove;
    buffer[14] = data->button;
    buffer[15] = FRAME_END;
}
