#include "esphome/core/log.h"
#include "dew_point.h"

namespace esphome::dew_point {

static const char *const TAG = "dew_point.sensor";

void DewPointComponent::setup() {
  ESP_LOGCONFIG(TAG, "Running setup for '%s'", this->get_name().c_str());

  ESP_LOGD(TAG, "  Added callback for temperature '%s'", this->temperature_sensor_->get_name().c_str());
  this->temperature_sensor_->add_on_state_callback([this](float state) { this->temperature_callback_(state); });
  if (this->temperature_sensor_->has_state()) {
    this->temperature_callback_(this->temperature_sensor_->get_state());
  }

  ESP_LOGD(TAG, "  Added callback for relative humidity '%s'", this->humidity_sensor_->get_name().c_str());
  this->humidity_sensor_->add_on_state_callback([this](float state) { this->humidity_callback_(state); });
  if (this->humidity_sensor_->has_state()) {
    this->humidity_callback_(this->humidity_sensor_->get_state());
  }
}

void DewPointComponent::dump_config() {
  LOG_SENSOR("", "Dew Point", this);

  ESP_LOGCONFIG(TAG,
                "Sources\n"
                "  Temperature: '%s'\n"
                "  Relative Humidity: '%s'",
                this->temperature_sensor_->get_name().c_str(), this->humidity_sensor_->get_name().c_str());
}

float DewPointComponent::get_setup_priority() const { return setup_priority::DATA; }

void DewPointComponent::loop() {
  if (!this->next_update_) {
    // We only want to update when we have new data, so disable the loop until we get a callback from one of the source sensors
    this->disable_loop();
    return;
  }
  this->next_update_ = false;

  // Ensure we have source data
  const bool no_temperature = std::isnan(this->temperature_);
  const bool no_humidity = std::isnan(this->humidity_);
  if (no_temperature || no_humidity) {
    if (no_temperature) {
      ESP_LOGW(TAG, "No valid state from temperature sensor!");
    }
    if (no_humidity) {
      ESP_LOGW(TAG, "No valid state from temperature sensor!");
    }
    ESP_LOGW(TAG, "Unable to calculate absolute humidity.");
    this->publish_state(NAN);
    this->status_set_warning();
    return;
  }

  // Calculate dew point
  const float dew_point = calculateDewPointBolton();

  // Publish absolute humidity
  ESP_LOGD(TAG, "Publishing dew point %f ºC", dew_point);
  this->status_clear_warning();
  this->publish_state(dew_point);
}

// Calculate Dew Point using Bolton (1980) constants
float calculateDewPointBolton() {
  // Constants
  const float b = 17.67;
  const float c = 243.5;

  const float humidity = this->humidity_;
  const float celsius = this->temperature_;

  // Intermediate gamma calculation
  float gamma = logf(humidity / 100.0) + ((b * celsius) / (c + celsius));

  float dewPoint = (c * gamma) / (b - gamma);
  return dewPoint;
}

}  // namespace esphome::dew_point
