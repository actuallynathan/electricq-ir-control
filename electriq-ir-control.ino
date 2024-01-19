#define IR_RECEIVE_PIN 14   // D5
#define IR_SEND_PIN 12      // D6
#define APPLICATION_PIN 13  // D7
#define FEEDBACK_LED_IS_ACTIVE_LOW
#include <IRremote.hpp>

enum class Power : uint8_t {
  OFF = 0x3,
  ON = 0x0
};

enum class Mode : uint8_t {
  MODE_HEAT = 0x1,
  MODE_COOL = 0x2,
  MODE_DRY = 0x3,
  MODE_AUTO = 0x4,
  MODE_FAN = 0x5
};

enum class FanSpeed : uint8_t {
  FAN_AUTO = 0x0,
  FAN_HIGH = 0x1,
  FAN_LOW = 0x2,
  FAN_MID = 0x3
};

typedef union {
  struct {
    uint8_t unknown_val : 8;           // 57:64 [8]
    uint8_t temperature_set : 8;       // 49:56 [8]
    uint8_t : 8;                       // 41:48 [8]
    uint8_t silent : 1;                // 40 [8]
    uint8_t : 3;                       // 36:39 [8]
    uint8_t : 4;                       // 32:35 [8]
    FanSpeed fan_speed : 2;            // 29:31 [8]
    bool timer : 1;                    // 28 [8]
    uint8_t : 1;                       // 28 [8]
    Mode mode : 4;                     // 24:27 [8]
    bool swing_horizontal : 1;         // 23 [8]
    bool swing_vertical : 1;           // 22 [8]
    bool toggle_display : 1;           // 21 [8]
    uint8_t : 1;                       // 20 [8]
    uint8_t : 2;                       // 18:19 [8]
    Power power : 2;                   // 16:17 [8]
    uint8_t : 1;                       // 15 [8]
    bool uvc : 1;                      // 14 [8]
    uint8_t : 1;                       // 13 [8]
    bool sleep : 1;                    // 13 [8]
    uint8_t : 2;                       // 10:11 [8]
    bool heating_or_auto : 1;          // 9
    uint8_t : 1;                       // 8
    uint8_t temperature_feedback : 7;  // 1:7
    bool ifeel_update : 1;             // 0

    uint8_t : 7;     //
    bool turbo : 1;  //
    uint8_t : 8;     //
    uint8_t timer_val : 8;  //
    uint64_t : 24;          //
    uint8_t checksum : 8;   //
    uint8_t : 8;            //
  } __attribute__((packed));
  uint64_t data[2];
} IRCommand;


void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  IrSender.begin();
  delay(500);
}

void sanitise_command(IRCommand* cmd) {
  cmd->unknown_val = 0x56;
  // bound our temperature to 16-32
  cmd->temperature_set = std::min(cmd->temperature_set, static_cast<uint8_t>(32));
  cmd->temperature_set = std::max(cmd->temperature_set, static_cast<uint8_t>(16));
  cmd->temperature_set += 0x5C;

  cmd->heating_or_auto = (cmd->mode == Mode::MODE_HEAT || cmd->mode == Mode::MODE_AUTO);

  uint8_t checksum = 0;
  cmd->checksum = 0;
  uint8_t* ptr = (uint8_t*)&cmd->data[0];
  for (; ptr < (uint8_t*)(&cmd->data[1]) + sizeof(uint64_t); ptr++) {
    checksum += (*ptr & 0xF) + ((*ptr >> 4) & 0xF);
  }
  cmd->checksum = checksum;
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
  sanitise_command(&cmd);
  Serial.printf("command: 0x%llx 0x%llx - pass: %i \n", cmd.data[0], cmd.data[1], TEST(cmd.data, 0x42001000006E56, 0x26000000000000));


  memset(&cmd, 0, sizeof(IRCommand));
  cmd.mode = Mode::MODE_HEAT;
  cmd.temperature_set = 19;
  cmd.fan_speed = FanSpeed::FAN_LOW;
  cmd.uvc = true;
  sanitise_command(&cmd);
  Serial.printf("command: 0x%llx 0x%llx - pass: %i \n", cmd.data[0], cmd.data[1], TEST(cmd.data, 0x42001200006F56, 0x29000000000000));

  memset(&cmd, 0, sizeof(IRCommand));
  cmd.mode = Mode::MODE_DRY;
  cmd.temperature_set = 16;
  cmd.fan_speed = FanSpeed::FAN_AUTO;
  cmd.uvc = false;
  sanitise_command(&cmd);
  Serial.printf("command: 0x%llx 0x%llx - pass: %i \n", cmd.data[0], cmd.data[1], TEST(cmd.data, 0x3000006C56, 0x20000000000000));

  memset(&cmd, 0, sizeof(IRCommand));
  cmd.mode = Mode::MODE_COOL;
  cmd.temperature_set = 24;
  cmd.silent = true;
  cmd.fan_speed = FanSpeed::FAN_AUTO;
  sanitise_command(&cmd);
  Serial.printf("command: 0x%llx 0x%llx - pass: %i \n", cmd.data[0], cmd.data[1], TEST(cmd.data, 0x2001007456, 0x19000000000000));

  memset(&cmd, 0, sizeof(IRCommand));
  cmd.mode = Mode::MODE_COOL;
  cmd.temperature_set = 24;
  cmd.silent = true;
  cmd.sleep = true;
  cmd.fan_speed = FanSpeed::FAN_AUTO;
  sanitise_command(&cmd);
  Serial.printf("command: 0x%llx 0x%llx - pass: %i \n", cmd.data[0], cmd.data[1], TEST(cmd.data, 0x8002001007456, 0x21000000000000));

  memset(&cmd, 0, sizeof(IRCommand));
  cmd.mode = Mode::MODE_DRY;
  cmd.temperature_set = 20;
  cmd.toggle_display = true;
  cmd.fan_speed = FanSpeed::FAN_MID;
  sanitise_command(&cmd);
  Serial.printf("command: 0x%llx 0x%llx - pass: %i \n", cmd.data[0], cmd.data[1], TEST(cmd.data, 0x43300007056, 0x1C000000000000));

  memset(&cmd, 0, sizeof(IRCommand));
  cmd.mode = Mode::MODE_FAN;
  cmd.temperature_set = 24;
  cmd.silent = true;
  cmd.timer = true;
  cmd.timer_val = 1;
  cmd.fan_speed = FanSpeed::FAN_HIGH;
  sanitise_command(&cmd);
  Serial.printf("command: 0x%llx 0x%llx - pass: %i \n", cmd.data[0], cmd.data[1], TEST(cmd.data, 0x5501007456, 0x22000000010000));

  memset(&cmd, 0, sizeof(IRCommand));
  cmd.mode = Mode::MODE_FAN;
  cmd.temperature_set = 24;
  cmd.silent = true;
  cmd.timer = true;
  cmd.timer_val = 3;
  cmd.fan_speed = FanSpeed::FAN_HIGH;
  sanitise_command(&cmd);
  Serial.printf("command: 0x%llx 0x%llx - pass: %i \n", cmd.data[0], cmd.data[1], TEST(cmd.data, 0x5501007456, 0x24000000030000));


  memset(&cmd, 0, sizeof(IRCommand));
  cmd.mode = Mode::MODE_HEAT;
  cmd.temperature_set = 32;
  cmd.silent = true;
  cmd.swing_horizontal = true;
  cmd.swing_vertical = true;
  cmd.fan_speed = FanSpeed::FAN_AUTO;
  sanitise_command(&cmd);
  Serial.printf("command: 0x%llx 0x%llx - pass: %i \n", cmd.data[0], cmd.data[1], TEST(cmd.data, 0x40031001007C56, 0x27000000000000));


  memset(&cmd, 0, sizeof(IRCommand));
  cmd.mode = Mode::MODE_HEAT;
  cmd.temperature_set = 16;
  cmd.fan_speed = FanSpeed::FAN_LOW;
  cmd.turbo = true;
  cmd.uvc = true;
  sanitise_command(&cmd);
  Serial.printf("command: 0x%llx 0x%llx - pass: %i \n", cmd.data[0], cmd.data[1], TEST(cmd.data, 0x42001200006c56, 0x2E000000000080));


  delay(10000);
}
