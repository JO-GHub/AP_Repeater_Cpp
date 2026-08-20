#include "functions.hpp"

/*------------------------ESP_Repeater Class ------------------------------*/


ESP_Repeater::ESP_Repeater() {}

bool ESP_Repeater::BLEProvision() {
    	bool provisioned = false;

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
	pinMO
}
