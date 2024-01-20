#define IR_RECEIVE_PIN 14   // D5
#define IR_SEND_PIN 12      // D6
#define APPLICATION_PIN 13  // D7
#define FEEDBACK_LED_IS_ACTIVE_LOW
#include <IRremote.hpp>
#include "IRCommand.h"


void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  IrSender.begin();
  delay(500);
}


bool TEST(uint64_t* data, uint64_t a, uint64_t b) {
  return memcmp(data, &a, sizeof(a)) == 0 && memcmp(data + 1, &b, sizeof(b)) == 0;
}

void loop() {
  IRCommand cmd;

  // Some test cases
  memset(&cmd, 0, sizeof(IRCommand));
  cmd.mode = Mode::MODE_HEAT;
  cmd.temperature_set = 18;
  cmd.fan_speed = FanSpeed::FAN_AUTO;
  cmd.uvc = true;
  calculate_checksum(&cmd);
  Serial.printf("command: 0x%llx 0x%llx - pass: %i \r\n", cmd.data[0], cmd.data[1], TEST(cmd.data, 0x42001000006E56, 0x26000000000000));


  memset(&cmd, 0, sizeof(IRCommand));
  cmd.mode = Mode::MODE_HEAT;
  cmd.temperature_set = 19;
  cmd.fan_speed = FanSpeed::FAN_LOW;
  cmd.uvc = true;
  calculate_checksum(&cmd);
  Serial.printf("command: 0x%llx 0x%llx - pass: %i \r\n", cmd.data[0], cmd.data[1], TEST(cmd.data, 0x42001200006F56, 0x29000000000000));

  memset(&cmd, 0, sizeof(IRCommand));
  cmd.mode = Mode::MODE_DRY;
  cmd.temperature_set = 16;
  cmd.fan_speed = FanSpeed::FAN_AUTO;
  cmd.uvc = false;
  calculate_checksum(&cmd);
  Serial.printf("command: 0x%llx 0x%llx - pass: %i \r\n", cmd.data[0], cmd.data[1], TEST(cmd.data, 0x3000006C56, 0x20000000000000));

  memset(&cmd, 0, sizeof(IRCommand));
  cmd.mode = Mode::MODE_COOL;
  cmd.temperature_set = 24;
  cmd.silent = true;
  cmd.fan_speed = FanSpeed::FAN_AUTO;
  calculate_checksum(&cmd);
  Serial.printf("command: 0x%llx 0x%llx - pass: %i \r\n", cmd.data[0], cmd.data[1], TEST(cmd.data, 0x2001007456, 0x19000000000000));

  memset(&cmd, 0, sizeof(IRCommand));
  cmd.mode = Mode::MODE_COOL;
  cmd.temperature_set = 24;
  cmd.silent = true;
  cmd.sleep = true;
  cmd.fan_speed = FanSpeed::FAN_AUTO;
  calculate_checksum(&cmd);
  Serial.printf("command: 0x%llx 0x%llx - pass: %i \r\n", cmd.data[0], cmd.data[1], TEST(cmd.data, 0x8002001007456, 0x21000000000000));

  memset(&cmd, 0, sizeof(IRCommand));
  cmd.mode = Mode::MODE_DRY;
  cmd.temperature_set = 20;
  cmd.toggle_display = true;
  cmd.fan_speed = FanSpeed::FAN_MID;
  calculate_checksum(&cmd);
  Serial.printf("command: 0x%llx 0x%llx - pass: %i \r\n", cmd.data[0], cmd.data[1], TEST(cmd.data, 0x43300007056, 0x1C000000000000));

  memset(&cmd, 0, sizeof(IRCommand));
  cmd.mode = Mode::MODE_FAN;
  cmd.temperature_set = 24;
  cmd.silent = true;
  cmd.timer = true;
  cmd.timer_val = 1;
  cmd.fan_speed = FanSpeed::FAN_HIGH;
  calculate_checksum(&cmd);
  Serial.printf("command: 0x%llx 0x%llx - pass: %i \r\n", cmd.data[0], cmd.data[1], TEST(cmd.data, 0x5501007456, 0x22000000010000));

  memset(&cmd, 0, sizeof(IRCommand));
  cmd.mode = Mode::MODE_FAN;
  cmd.temperature_set = 24;
  cmd.silent = true;
  cmd.timer = true;
  cmd.timer_val = 3;
  cmd.fan_speed = FanSpeed::FAN_HIGH;
  calculate_checksum(&cmd);
  Serial.printf("command: 0x%llx 0x%llx - pass: %i \r\n", cmd.data[0], cmd.data[1], TEST(cmd.data, 0x5501007456, 0x24000000030000));


  memset(&cmd, 0, sizeof(IRCommand));
  cmd.mode = Mode::MODE_HEAT;
  cmd.temperature_set = 32;
  cmd.silent = true;
  cmd.swing_horizontal = true;
  cmd.swing_vertical = true;
  cmd.fan_speed = FanSpeed::FAN_AUTO;
  calculate_checksum(&cmd);
  Serial.printf("command: 0x%llx 0x%llx - pass: %i \r\n", cmd.data[0], cmd.data[1], TEST(cmd.data, 0x40031001007C56, 0x27000000000000));


  memset(&cmd, 0, sizeof(IRCommand));
  cmd.mode = Mode::MODE_HEAT;
  cmd.temperature_set = 16;
  cmd.fan_speed = FanSpeed::FAN_LOW;
  cmd.turbo = true;
  cmd.uvc = true;
  calculate_checksum(&cmd);
  Serial.printf("command: 0x%llx 0x%llx - pass: %i \r\n", cmd.data[0], cmd.data[1], TEST(cmd.data, 0x42001200006c56, 0x2E000000000080));


  delay(10000);
}
