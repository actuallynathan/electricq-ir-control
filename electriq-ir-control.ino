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
  cmd.toggle_display = true;
  calculate_checksum(&cmd);
  Serial.printf("command: 0x%llx 0x%llx \r\n", cmd.data[0], cmd.data[1]);

  IrSender.sendPulseDistanceWidthFromArray(38, 8400, 4250, 500, 1650, 500, 600, &cmd.data[0], 120, PROTOCOL_IS_LSB_FIRST, 0, 0);

  delay(10000);
}
