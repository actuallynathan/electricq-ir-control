#pragma once
#include <algorithm>

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
    uint8_t : 8;                       // 0:7   [0]
    uint8_t temperature_set : 8;       // 8:1   [1]
    uint8_t : 8;                       // 16:23 [2]
    uint8_t silent : 1;                // 24    [3]
    uint8_t : 7;                       // 25:31 [3]
    FanSpeed fan_speed : 2;            // 32:33 [4]
    bool timer : 1;                    // 34    [4]
    uint8_t : 1;                       // 35    [4]
    Mode mode : 4;                     // 36:39 [4]
    bool swing_horizontal : 1;         // 40    [5]
    bool swing_vertical : 1;           // 41    [5]
    bool toggle_display : 1;           // 42    [5]
    uint8_t : 3;                       // 43:45 [5]
    Power power : 2;                   // 46:47 [5]
    uint8_t : 1;                       // 48    [6]
    bool uvc : 1;                      // 49    [6]
    uint8_t : 1;                       // 50    [6]
    bool sleep : 1;                    // 51    [6]
    uint8_t : 2;                       // 52:53 [6]
    bool heating_or_auto : 1;          // 54    [6]
    uint8_t : 1;                       // 55    [6]
    uint8_t temperature_feedback : 7;  // 56:62 [7]
    bool ifeel_update : 1;             // 63    [7]

    uint8_t : 7;            //
    bool turbo : 1;         //
    uint8_t : 8;            //
    uint8_t timer_val : 8;  //
    uint64_t : 24;          //
    uint8_t checksum : 8;   //
    uint8_t : 8;            //
  } __attribute__((packed));
  uint64_t data[2];
} IRCommand;


void calculate_checksum(IRCommand* cmd) {
  cmd->data[0] |= 0x56;
  // bound our temperature to 16-32, unless we're already in the range of 0x6C-0x7C
  if(cmd->temperature_set < 0x6C || cmd->temperature_set > 0x7C)
  {
    cmd->temperature_set = std::min(cmd->temperature_set, static_cast<uint8_t>(32));
    cmd->temperature_set = std::max(cmd->temperature_set, static_cast<uint8_t>(16));
    cmd->temperature_set += 0x5C;
  }

  cmd->heating_or_auto = (cmd->mode == Mode::MODE_HEAT || cmd->mode == Mode::MODE_AUTO);

  uint8_t checksum = 0;
  cmd->checksum = 0;
  uint8_t* ptr = (uint8_t*)&cmd->data[0];
  for (; ptr < (uint8_t*)(&cmd->data[1]) + sizeof(uint64_t); ptr++) {
    checksum += (*ptr & 0xF) + ((*ptr >> 4) & 0xF);
  }
  cmd->checksum = checksum;
}
