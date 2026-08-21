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

using namespace std;

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

class Oled {
private:
    const uint8_t sensor_sda{6};
    const uint8_t sensor_scl{7};
    const uint8_t height{64};
    const uint8_t width{128};
    const uint8_t LED{1};
    Adafruit_SSD1306 display{width, height};
    

public:
    Oled();

    void welcome_msg();

    void credential_check_msg();

    void reset_provision_msg();

    void provisioning_msg();

    void sta_name_msg();

    void sta_rssi_msg();

    void ap_name_msg();

    void socket_read_msg();

    void socket_write_msg(const vector<uint8_t>& socket_num, const vector<int16_t>& rssi);

	void best_socket_msg(const vector<uint8_t>& median_socket, const vector<uint8_t>& median_rssi);

    void user_prompt_msg();

    void no_readings_msg();

    void connecting_msg();

    void survey_prompt_msg();

    void repeater_mode_msg();
};


class NVS {
private:
	vector<uint8_t>& socket_num;
	vector<int16_t>& rssi;
	Preferences nvs;
	
public:
    NVS(vector<uint8_t>& socket_count, vector<int16_t>& rssi_readings);

    void write();

    void read();

    void socket_algo();
};


class TB {
private:
	WiFiClient esp32;
	Arduino_MQTT_Client mqttClient(esp32);
	ThingsBoardSized tb(mqttClient);

	const char tb_host[16] = "192.168.213.10";
	const char access_token[24] = "W7OO3uuv3BZ5ZyXOjETZ";
	
public:
    TB();

    void send_telemetry();
};


