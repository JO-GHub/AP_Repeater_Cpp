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

class ESP_Repeater {
private:
	const char* ap_ssid{"ESP32-C5"};
	const char* ap_pass{"12345678"};
	const char* pop{"abcd1234"};
	const char* service_name{"C5"};
	const char* service_key{nullptr};			// dedicated keyword for preventing type errors
	bool reset_provisioned{false};
	bool prov_done{false};
	bool got_ip{false};
	bool ap_started{false};
	bool napt_enabled{false};
	int ch{0};                                
	const IPAddress ap_ip{192, 168, 10, 1};
	const IPAddress ap_gw{192, 168, 10, 1};
	const IPAddress ap_mask{255, 255, 255, 0};
	const IPAddress ap_dhcpstart{192, 168, 10, 2};
	const IPAddress ap_dns{8, 8, 8, 8};

public:
    ESP_Repeater();
    
    bool BLEProvision();
    
    void APStart();

    void post_provision(arduino_event_t *event);

    void connect(arduino_event_t *event);

    bool is_sta_active() const;

    bool is_ap_active() const;

    bool is_provisioned() const;
};

class Button {
private:
    int8_t button_num{0};
	bool is_pressed{false};
	unsigned long press_timer_start{0};

   	public:
    enum class Press_Type {
    	NONE,
    	SHORT_PRESS,
    	LONG_PRESS
    };

   	Press_Type get_event();			// read buttton press type and return what type of press occured
   	
    Button(int8_t pin);
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
