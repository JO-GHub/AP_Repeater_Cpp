#pragma once

#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <Network.h>
#include <WiFiProv.h>
#include <Preferences.h>
#include <ThingsBoard.h>
#include <Arduino_MQTT_Client.h>
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <vector> 

class WiFiSetup {
private:
	const char *ap_ssid = "ESP32-C5";
	const char *ap_pass = "12345678";
	const char *pop = "abcd1234";
	const char *service_name = "C5";
	const char *service_key = NULL;
	const bool reset_provisioned = false;

public:
    WiFiSetup();
    
    bool BLEProvision();
    
    void APStart();

    void post_provision(arduino_event_t *event);

    void connect(arduino_event_t *event);

    bool is_online() const;

    bool is_ap_active const;

    bool is_provisioned const;
};

class NVS {
private:
public:
    NVS();

    void start();

    void write();

    void read() const;

    bool socket_algo(std::vector<int8_t>& socket_num, std::vector<int8_t>& rssi);
};

class Button {
private:
    int8_t button_num;
public:
    Button(int8_t button_num);
};

class TB {
private:
public:
    TB();
};

class Oled {
private:
    const int8_t sda;
    const int8_t scl;
    const int8_t height;
    const int8_t width;
    const int8_t LED;

public:
    Oled(int8_t sda_pin, int8_t scl_pin, int8_t h, int8_t w, int8_t led_pin);

    bool welcome_msg() const;

    bool credential_check_msg() const;

    bool reset_provision_msg() const;

    bool provisioning_msg() const;

    bool sta_name_msg() const;

    bool sta_rssi_msg() const;

    bool ap_name_msg() const;

    bool socket_read_msg() const;

    bool socket_write_msg(const std::vector<int8_t>& socket_num, const std::vector<int8_t>& rssi) const;

    bool user_prompt_msg() const;

    bool no_readings_msg() const;

    bool connecting_msg() const;

    bool survey_prompt_msg() const;

    bool repeater_mode_msg() const;
};
