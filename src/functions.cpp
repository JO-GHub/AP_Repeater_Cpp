#include "functions.hpp"

/*------------------------ESP_Repeater Class ------------------------------*/

bool ESP_Repeater::BLEProvision() {
    	bool provisioned = false;

ESP_Repeater::ESP_Repeater() {}


    	WiFiProv.beginProvision(
    	    NETWORK_PROV_SCHEME_BLE,
    	    NETWORK_PROV_SCHEME_HANDLER_FREE_BLE,
    	    NETWORK_PROV_SECURITY_1,
    	    pop,
    	    service_name,
    	    service_key,
    	    nullptr,
    	    reset_provisioned
    	);

    	network_prov_mgr_is_wifi_provisioned(&provisioned);

    	startup_handler();

    	return true;
}
    
void ESP_Repeater::APStart() {
    	if (got_ip) {
    	    if (ap_started)
    	        return;

    	    if (WiFi.setAutoReconnect(true))
    	        Serial.println("Auto reconnect enabled");
    	    else
    	        Serial.println("Auto reconnect failed");

    	    if (WiFi.softAPConfig(ap_ip, ap_gw, ap_mask, ap_dhcpstart, ap_dns))
    	        Serial.println("AP configured");

    	    ch = WiFi.channel();

    	    if (ch == 0)
    	        ch = 1;

    	    Serial.printf("STA channel: %d\n", ch);

    	    if (WiFi.softAP(ap_ssid, ap_pass, ch, 0, 10))
    	        Serial.println("AP started");

    	    // Match TX power to band — 5 GHz gets the full 20 dBm,
    	    // 2.4 GHz is capped at 17 dBm
    	    if (ch >= 36) {
    	        WiFi.setTxPower(WIFI_POWER_20dBm);
    	        Serial.println("5GHz - 20dBm");
    	    } else {
    	        WiFi.setTxPower(WIFI_POWER_17dBm);
    	        Serial.println("2.4GHz - 17dBm");
    	    }

    	    if (!napt_enabled) {
    	        if (WiFi.AP.enableNAPT(true)) {
    	            napt_enabled = true;
    	            Serial.println("NAPT enabled");
    	        } else {
    	            Serial.println("NAPT failed");
    	        }
    	    }

    	    ap_started = true;
    	}
    }

void ESP_Repeater::post_provision(arduino_event_t *event) {
    	if (event->event_id == ARDUINO_EVENT_PROV_EXTENDER)
    	    prov_done = true;
    }

void ESP_Repeater::connect(arduino_event_t *event) {
	if (prov_done == true) {
	    if (event->event_id == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
	        vTaskDelay(pdMS_TO_TICKS(5000));
	        got_ip = true;
	    }
	} else {
	    if (WiFi.STA.hasIP() && WiFi.STA.connected()) {
	        vTaskDelay(pdMS_TO_TICKS(5000));
	        got_ip = true;
	    }
	}
}

bool ESP_Repeater::is_sta_active() const {
	return got_ip;
}

bool ESP_Repeater::is_ap_active() const {
	return ap_started;
}

bool ESP_Repeater::is_provisioned() const {
	return prov_done;
}


/*------------------------Button Class ------------------------------*/

Button::Button(int8_t pin) : button_num{pin} {
	pinMode(pin, INPUT_PULLUP);
}

Button::Press_Type Button::get_event() {
	if (digitalRead(button_num) == 0 && !is_pressed) {
            press_timer_start = millis();
            is_pressed = true;
        }

    if (digitalRead(button_num) == 1 && is_pressed) {
        unsigned long duration = millis() - press_timer_start;
        is_pressed = false;

        if (duration < 100) {
        	return Press_Type::NONE;
        }
        else if (duration > 3000) {
            return Press_Type::LONG_PRESS;
		}
		else {
			return Press_Type::SHORT_PRESS;
		}
	}

	return Press_Type::NONE;
}

/*------------------------Oled Class ------------------------------*/

Oled::Oled() {
	Wire.setPins(sensor_sda, sensor_scl);
	Wire.begin();

	uint8_t address{0};
	bool is_started{false};

	// Scan addresses to find the display
	for (int i = 0; i <= 127; i++) {
	    Wire.beginTransmission(i);

		// if something answers back, we found it
	    if (Wire.endTransmission() == 0) {
	    	address = i;
	        Serial.printf("OLED address 0x%02x\n", address);

	        if (display.begin(SSD1306_SWITCHCAPVCC, address)) {
	            Serial.println("Display is live");
	            Serial.printf(
	                "Bus %d\nClock %luHz\nTimeout %dms\n",
	                Wire.getBusNum(),
	                Wire.getClock(),
	                Wire.getTimeOut()
	            );
	            is_started = true;
         	    break;								// stop the loop since we found the display
         	}
	    }
	}

	if (!is_started) Serial.println("No device found.");
}

void Oled::welcome_msg() {
// Welcome screen shown at boot
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.write("WELCOME");
    display.setCursor(0, 16);
    display.write("USER");
    display.setTextSize(1);
    display.setCursor(0, 48);
    display.write("Joe's WiFi Repeater");
    display.display();
    
}

void Oled::credential_check_msg() {
// Shown while stored credentials are being verified in the background
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.write("CREDENTIAL");
    display.setTextSize(1);
    display.setCursor(0, 16);
    display.write("Please wait while");
    display.setCursor(0, 32);
    display.write("the system does a");
    display.setCursor(0, 48);
    display.write("background check");
    display.display();
}

void Oled::reset_provision_msg() {
// Shown briefly before the device wipes credentials and reboots
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.write("PROVISION");
    display.setTextSize(1);
    display.setCursor(0, 16);
    display.write("Clearing all settings");
    display.setCursor(0, 32);
    display.write("and restarting...");
    display.display();
}
    
void Oled::provisioning_msg() {
// Instructs the user to open the ESP BLE app and provision the device
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.write("PROVISION");
    display.setTextSize(1);
    display.setCursor(0, 16);
    display.write("Use the ESP BLE App");
    display.setCursor(0, 32);
    display.write("and provision");
    display.setCursor(0, 48);
    display.write("your device");
    display.display();
    
}

void Oled::sta_name_msg() {
// Shows the SSID the device is connected to as a station,
// scaling text to fit
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.write("STATION");

    String ssid = WiFi.SSID();

    if (ssid.length() <= 7)
        display.setTextSize(3);
    else if (ssid.length() <= 10)
        display.setTextSize(2);
    else
        display.setTextSize(1);

    display.setCursor(0, 16);
    display.write(ssid.c_str());
    display.display();
}
    
void Oled::ap_name_msg() {
// Shows the AP network name the device is broadcasting
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.write("AP NETWORK");
    display.setTextSize(2);
    display.write(WiFi.softAPSSID().c_str());
    display.display();
}

void Oled::sta_rssi_msg() {
// Shows the current RSSI of the upstream STA connection
    char buf[16];

    sprintf(buf, "%d dBm", WiFi.RSSI());

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.write("SIGNAL");

    if (strlen(buf) <= 7)
        display.setTextSize(3);
    else
        display.setTextSize(2);

    display.setCursor(0, 16);
    display.write(buf);
    display.display();
}

void Oled::socket_read_msg() {
// Prompts the user to press or hold the button
// to record or skip the survey
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.write("SURVEY");
    display.setTextSize(1);
    display.setCursor(0, 16);
    display.write("Press button");
    display.setCursor(0, 24);
    display.write("to record RSSI");
    display.setCursor(0, 32);
    display.write("Hold button");
    display.setCursor(0, 48);
    display.write("to skip survey");
    display.display();
}

void Oled::socket_write_msg(const std::vector<uint8_t>& socket_num, const std::vector<int16_t>& rssi) {
// Confirms a reading was saved,
// showing the socket number and RSSI value
    char line1[12], line2[16];

	if (!socket_num.empty() && !rssi.empty()) {
	    snprintf(line1, sizeof(line1), "Socket: %u", socket_num[0]);
	    snprintf(line2, sizeof(line2), "RSSI: %i dBm", rssi[0]);

	    display.clearDisplay();
	    display.setTextColor(SSD1306_WHITE);
	    display.setTextSize(2);
	    display.setCursor(0, 0);
	    display.write("SAVED!");
	    display.setCursor(0, 16);
	    display.write(line1);
	    display.setTextSize(1);
	    display.setCursor(0, 32);
	    display.write(line2);
	    display.display();
    }
}

void Oled::user_prompt_msg() {
// Tells the user they can reprovision with a short press
// or skip with a long hold
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.write("STARTUP");
    display.setTextSize(1);
    display.setCursor(0, 16);
    display.write("Press the button");
    display.setCursor(0, 24);
    display.write("to reprovision");
    display.setCursor(0, 48);
    display.write("Hold the button");
    display.setCursor(0, 56);
    display.write("for 10s to skip");
    display.display();    
}

void Oled::best_socket_msg(const uint8_t median_socket, const int16_t median_rssi) {
// Shows the recommended socket — the one with
// the median RSSI across all saved readings
    char line1[12], line2[16];

    snprintf(line1, sizeof(line1), "Socket: %u", median_socket);
    snprintf(line2, sizeof(line2),"RSSI: %i dBm", median_rssi);

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.write("SUGGESTION");
    display.setCursor(0, 16);
    display.write(line1);
    display.setTextSize(1);
    display.setCursor(0, 32);
    display.write(line2);
    display.display();
}

void Oled::no_readings_msg() {
// Shown when no RSSI readings have been saved to NVS yet
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.write("NO DATA");
    display.setTextSize(1);
    display.setCursor(0, 16);
    display.write("No socket readings");
    display.setCursor(0, 32);
    display.write("have been saved.");
    display.display();
}

void Oled::connecting_msg() {
// Shown while the device is connecting to the provisioned network
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.write("CONNECTING");

    String ssid = WiFi.SSID();

    if (ssid.length() <= 7)
        display.setTextSize(3);
    else if (ssid.length() <= 10)
        display.setTextSize(2);
    else
        display.setTextSize(1);

    display.setCursor(0, 16);
    display.write(ssid.c_str());
    display.display();
}
void Oled::survey_prompt_msg() {
// Prompts the user to press to record RSSI
// or hold to skip straight to repeater mode
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.write("SURVEY");
    display.setTextSize(1);
    display.setCursor(0, 16);
    display.write("Press the button");
    display.setCursor(0, 24);
    display.write("to record RSSI");
    display.setCursor(0, 48);
    display.write("Hold the button");
    display.setCursor(0, 56);
    display.write("for 10s to skip");
    display.display();
}
 
void Oled::repeater_mode_msg() {
// Shown when the device is transitioning into AP repeater mode
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.write("MODE");
    display.setTextSize(2);
    display.setCursor(0, 16);
    display.write("Switching");
    display.setCursor(0, 32);
    display.write("to AP");
    display.setCursor(0, 48);
    display.write("mode...");
    display.display();
}

/*------------------------NVS Class ------------------------------*/


NVS::NVS(vector<uint8_t>& socket_count, vector<int16_t>& rssi_readings) : socket_num(socket_count), rssi(rssi_readings) {
	    nvs.begin("RSSI", false);
}

void NVS::write() {
	if (!napt_enabled)
	    return;

	digitalWrite(LED, HIGH);

	int socket_num = 0;
	int8_t reading = 0;

	for (int i = 1; i <= 9; i++) {
	    char key[2] = {(char)(i + 48), '\0'};

	    if (!nvs.isKey(key)) {
	        socket_num = i;
	        reading = (int8_t)WiFi.STA.RSSI();

	        nvs.putChar(key, reading);

	        Serial.printf("Key: %s, RSSI: %d\n", key, reading);
	        break;
	    }
	}

	if (nvs.freeEntries() == 0) {
	    Serial.println("NVS full, clearing");
	    nvs.clear();
	}

	digitalWrite(LED, LOW);

	Socket_saved_msg(socket_num, reading);
	vTaskDelay(pdMS_TO_TICKS(2000));
	Socket_msg();
}

void NVS::read() {
    int8_t readings[9];
    int sockets[9];
    int count = 0;

    for (int i = 1; i <= 9; i++) {
        char key[2] = {(char)(i + 48), '\0'};

        if (!nvs.isKey(key))
            break;

        readings[count] = nvs.getChar(key, -128);
        sockets[count] = i;
        count++;
    }

    if (count == 0) {
        No_readings_msg();
        vTaskDelay(pdMS_TO_TICKS(4000));
        return;
    }
}

void NVS::socket_algo() {
    // Bubble sort readings ascending so the median index lands in the middle
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - 1 - i; j++) {
            if (readings[j] > readings[j + 1]) {
                int8_t temp_rssi = readings[j];
                readings[j] = readings[j + 1];
                readings[j + 1] = temp_rssi;

                int temp_socket = sockets[j];
                sockets[j] = sockets[j + 1];
                sockets[j + 1] = temp_socket;
            }
        }
    }

    int mid = count / 2;

    Serial.printf(
        "Key: %d, RSSI: %d\n",
        sockets[mid],
        readings[mid]
    );

    Best_socket_msg(sockets[mid], readings[mid]);
    vTaskDelay(pdMS_TO_TICKS(4000));
}

/*------------------------TB Class ------------------------------*/

TB::TB() {
	if (!tb.connected()) {
	    if (!tb.connect(tb_host, access_token))
	        return false;

	    vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void TB::send_telemetry() {
	tb.sendAttributeData("STA_RSSI", WiFi.STA.RSSI());
	tb.sendAttributeData("STA_MAC", WiFi.STA.BSSIDstr().c_str());
	tb.sendAttributeData("STA_IP", WiFi.STA.localIP().toString().c_str());
	tb.sendAttributeData("STA_SSID", WiFi.SSID().c_str());
	tb.sendAttributeData("Channel", WiFi.channel());
	tb.sendAttributeData("AP_MAC", WiFi.softAPmacAddress().c_str());
	tb.sendAttributeData("AP_IP", WiFi.softAPIP().toString().c_str());
	tb.sendAttributeData("AP_SSID", WiFi.softAPSSID().c_str());

	if (ch >= 36)
	    tb.sendAttributeData("AP_TxPower", "20dBm+3dBi");
	else
	    tb.sendAttributeData("AP_TxPower", "17dBm+3dBi");
}
