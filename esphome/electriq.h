#include "esphome.h"
#define IR_RECEIVE_PIN 14   // D5
#define IR_SEND_PIN 12      // D6
#define APPLICATION_PIN 13  // D7
#define FEEDBACK_LED_IS_ACTIVE_LOW
#include <IRremote.hpp>
#include "IRCommand.h"

#include "esphome/components/sensor/sensor.h"

class ElectriqClimate : public Component, public Climate {
 public:
  void setup() override {
    // This will be called by App.setup()
    this->target_temperature = 16;
    this->current_temperature = 0;
    IrSender.begin();
    memset(this->ir_cmd.data, 0, sizeof(this->ir_cmd));
  }

  void register_temperature_sensor(esphome::sensor::Sensor *current_temperature)
  {
    current_temperature->add_on_state_callback([this](float state){ 
      this->current_temperature = state; 
      this->ir_cmd.temperature_feedback = (uint8_t) (this->current_temperature * 2);
      ESP_LOGD("electriq", "temperature updated: %0.1f", (float)(this->ir_cmd.temperature_feedback) / 2);

      this->ir_cmd.ifeel_update = true;
      calculate_checksum(&this->ir_cmd);
      ESP_LOGD("electriq", "sending command: 0x%llx 0x%llx", this->ir_cmd.data[0], this->ir_cmd.data[1]);
      IrSender.sendPulseDistanceWidthFromArray(38, 8400, 4250, 500, 1650, 500, 600, &ir_cmd.data[0], 120, PROTOCOL_IS_LSB_FIRST, 0, 0);
      this->ir_cmd.ifeel_update = false;
    });
  }

  void register_uvc_switch(esphome::template_::TemplateSwitch *uvc_switch)
  {
    uvc_switch->add_on_state_callback([this](bool state){ 
      this->ir_cmd.uvc = state;
      this->ir_cmd.ifeel_update = false;

      ESP_LOGD("electriq", "uvc is now ", ((state) ? "ENABLED" : "DISABLED"));
      
      calculate_checksum(&this->ir_cmd);
      ESP_LOGD("electriq", "sending command: 0x%llx 0x%llx", this->ir_cmd.data[0], this->ir_cmd.data[1]);
      IrSender.sendPulseDistanceWidthFromArray(38, 8400, 4250, 500, 1650, 500, 600, &this->ir_cmd.data[0], 120, PROTOCOL_IS_LSB_FIRST, 0, 0);
    });
  }

  void register_toggle_display(esphome::template_::TemplateButton *toggle_display)
  {
    toggle_display->add_on_press_callback([this](){ 
      this->ir_cmd.toggle_display = true;
      this->ir_cmd.ifeel_update = false;

      ESP_LOGD("electriq", "toggling display");
      
      calculate_checksum(&this->ir_cmd);
      ESP_LOGD("electriq", "sending command: 0x%llx 0x%llx", this->ir_cmd.data[0], this->ir_cmd.data[1]);
      IrSender.sendPulseDistanceWidthFromArray(38, 8400, 4250, 500, 1650, 500, 600, &this->ir_cmd.data[0], 120, PROTOCOL_IS_LSB_FIRST, 0, 0);
      this->ir_cmd.toggle_display = false;
    });
  }

  void control(const ClimateCall &call) override {
    bool updated_state = false;

    if (call.get_mode().has_value()) {
      // User requested mode change
      ClimateMode mode = *call.get_mode();
      // Send mode to hardware
      // ...
      this->ir_cmd.power = Power::ON;

      switch(mode)
      {
        case ClimateMode::CLIMATE_MODE_OFF:
          this->ir_cmd.power = Power::OFF;
          ESP_LOGD("electriq", "Power off");
          break;
        case ClimateMode::CLIMATE_MODE_COOL:
          this->ir_cmd.mode = Mode::MODE_COOL;
          ESP_LOGD("electriq", "Mode COOL");
          break;
        case ClimateMode::CLIMATE_MODE_HEAT:
          this->ir_cmd.mode = Mode::MODE_HEAT;
          ESP_LOGD("electriq", "Mode HEAT");
          break;
        case ClimateMode::CLIMATE_MODE_DRY:
          this->ir_cmd.mode = Mode::MODE_DRY;
          ESP_LOGD("electriq", "Mode DRY");
          break;
        case ClimateMode::CLIMATE_MODE_FAN_ONLY:
          this->ir_cmd.mode = Mode::MODE_FAN;
          ESP_LOGD("electriq", "Mode FAN");
          break;
        case ClimateMode::CLIMATE_MODE_AUTO:
          this->ir_cmd.mode = Mode::MODE_AUTO;
          ESP_LOGD("electriq", "Mode AUTO");
          break;
        default:
          ESP_LOGD("electriq", "Mode not supported");

      }

      updated_state = true;

      // Publish updated state
      this->mode = mode;
    }

    if (call.get_fan_mode().has_value()) {

      ClimateFanMode mode = *call.get_fan_mode();
      // Send mode to hardware
      // ...
      this->ir_cmd.silent = false;

      switch(mode)
      {
        case ClimateFanMode::CLIMATE_FAN_AUTO:
          this->ir_cmd.fan_speed = FanSpeed::FAN_AUTO;
          ESP_LOGD("electriq", "Fan speed set to AUTO");
          break;
        case ClimateFanMode::CLIMATE_FAN_LOW:
          this->ir_cmd.fan_speed = FanSpeed::FAN_LOW;
          ESP_LOGD("electriq", "Fan speed set to LOW");
          break;
        case ClimateFanMode::CLIMATE_FAN_MEDIUM:
          this->ir_cmd.fan_speed = FanSpeed::FAN_MID;
          ESP_LOGD("electriq", "Fan speed set to MID");
          break;
        case ClimateFanMode::CLIMATE_FAN_HIGH:
          this->ir_cmd.fan_speed = FanSpeed::FAN_HIGH;
          ESP_LOGD("electriq", "Fan speed set to HIGH");
          break;
        case ClimateFanMode::CLIMATE_FAN_QUIET:
          this->ir_cmd.silent = true;
          ESP_LOGD("electriq", "Fan speed set to QUIET");
          break;
        default:
          ESP_LOGD("electriq", "Mode not supported");
      }
      
      this->fan_mode = mode;
      updated_state = true;
    }

    if (call.get_swing_mode().has_value()) {

      ClimateSwingMode mode = *call.get_swing_mode();
      // Send mode to hardware
      // ...
      switch(mode)
      {
        case ClimateSwingMode::CLIMATE_SWING_OFF:
          this->ir_cmd.swing_horizontal = false;
          this->ir_cmd.swing_vertical = false;
          ESP_LOGD("electriq", "Swing mode set to OFF");
          break;
        case ClimateSwingMode::CLIMATE_SWING_BOTH:
          this->ir_cmd.swing_horizontal = true;
          this->ir_cmd.swing_vertical = true;
          ESP_LOGD("electriq", "Swing mode set to BOTH");
          break;
        case ClimateSwingMode::CLIMATE_SWING_VERTICAL:
          this->ir_cmd.swing_horizontal = false;
          this->ir_cmd.swing_vertical = true;
          ESP_LOGD("electriq", "Swing mode set to VERTICAL");
          break;
        case ClimateSwingMode::CLIMATE_SWING_HORIZONTAL:
          this->ir_cmd.swing_horizontal = true;
          this->ir_cmd.swing_vertical = false;
          ESP_LOGD("electriq", "Swing mode set to HORIZONTAL");
          break;
        default:
          ESP_LOGD("electriq", "Mode not supported");
      }

      this->swing_mode = mode;
      updated_state = true;
    }

    if (call.get_target_temperature().has_value()) {
      // User requested target temperature change
      // Send target temp to climate
      // ...
      this->target_temperature = *call.get_target_temperature();
      this->ir_cmd.temperature_set = static_cast<uint8_t>(this->target_temperature);
      ESP_LOGD("electriq", "set temperature to: %i", this->ir_cmd.temperature_set);
      
      updated_state = true;
    }

    if(updated_state)
    {
      calculate_checksum(&this->ir_cmd);
      ESP_LOGD("electriq", "sending command: 0x%llx 0x%llx", this->ir_cmd.data[0], this->ir_cmd.data[1]);
      IrSender.sendPulseDistanceWidthFromArray(38, 8400, 4250, 500, 1650, 500, 600, &this->ir_cmd.data[0], 120, PROTOCOL_IS_LSB_FIRST, 0, 0);
      this->publish_state();
    }
  }
  ClimateTraits traits() override {
    // The capabilities of the climate device
    auto traits = climate::ClimateTraits();
    traits.set_visual_min_temperature(16);
    traits.set_visual_max_temperature(32);
    traits.set_supports_current_temperature(false);
    traits.set_visual_temperature_step(1);

    traits.add_supported_mode(ClimateMode::CLIMATE_MODE_COOL);
    traits.add_supported_mode(ClimateMode::CLIMATE_MODE_HEAT);
    traits.add_supported_mode(ClimateMode::CLIMATE_MODE_DRY);
    traits.add_supported_mode(ClimateMode::CLIMATE_MODE_FAN_ONLY);
    traits.add_supported_mode(ClimateMode::CLIMATE_MODE_AUTO);
    
    traits.add_supported_fan_mode(ClimateFanMode::CLIMATE_FAN_AUTO);
    traits.add_supported_fan_mode(ClimateFanMode::CLIMATE_FAN_LOW);
    traits.add_supported_fan_mode(ClimateFanMode::CLIMATE_FAN_MEDIUM);
    traits.add_supported_fan_mode(ClimateFanMode::CLIMATE_FAN_HIGH);
    traits.add_supported_fan_mode(ClimateFanMode::CLIMATE_FAN_QUIET);

    traits.add_supported_swing_mode(ClimateSwingMode::CLIMATE_SWING_OFF);
    traits.add_supported_swing_mode(ClimateSwingMode::CLIMATE_SWING_VERTICAL);
    traits.add_supported_swing_mode(ClimateSwingMode::CLIMATE_SWING_HORIZONTAL);
    traits.add_supported_swing_mode(ClimateSwingMode::CLIMATE_SWING_BOTH);

    return traits;
  }

  private:
    IRCommand ir_cmd;
    float current_temperature;
};