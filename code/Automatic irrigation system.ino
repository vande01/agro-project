// Include required libraries
#include <Adafruit_NAU7802.h>  // Library for NAU7802 ADC weight sensor
#include <U8g2lib.h>           // Library for OLED display
#include <Wire.h>              // I2C communication library
#include <WiFi.h>              // WiFi connectivity library
#include <PubSubClient.h>      // MQTT client library

// Create instances of required objects
Adafruit_NAU7802 nau;         // Create NAU7802 ADC object
// Initialize OLED display with I2C interface, no reset pin
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// WiFi network credentials
const char* ssid = "agrotech";           // Network name
const char* password = "1Afuna2gezer";    // Network password

// MQTT broker configuration
const char* mqtt_server = "192.168.0.102";    // MQTT broker IP address (Home Assistant)
const int mqtt_port = 1883;                   // MQTT broker port
const char* mqtt_user = "mqtt-user";          // MQTT username for authentication
const char* mqtt_password = "1234";           // MQTT password for authentication
const char* mqtt_topic = "/greenhouse/outside/irrigation/solenoid5"; // Topic to publish messages to

// Define touch sensor pin
#define TOUCH_PIN A4  // Analog pin A4 used for touch sensing

// Initialize WiFi and MQTT clients
WiFiClient espClient;           // Create WiFi client instance
PubSubClient client(espClient); // Create MQTT client using WiFi client

// State variable for message control
bool send_message = false;      // Flag to control message sending

// Function to establish WiFi connection
void setup_wifi() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);  // Start WiFi connection

    // Wait until connected
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    // Print connection success and IP address
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

// Function to handle MQTT connection and reconnection
void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Try to connect with specified credentials
        if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
            Serial.println("connected");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" Retrying in 5 seconds...");
            delay(5000);
        }
    }
}

// Initial setup function
void setup() {
    Serial.begin(115200);      // Initialize serial communication
    
    setup_wifi();              // Connect to WiFi
    client.setServer(mqtt_server, mqtt_port);  // Configure MQTT server
    reconnect();               // Ensure MQTT connection
    
    // Initialize OLED display
    u8g2.begin();
    
    // Display startup message
    u8g2.clearBuffer();        // Clear display buffer
    u8g2.setFont(u8g2_font_6x10_tr);  // Set font
    u8g2.drawStr(3, 10, "our measuring project");  // Draw text
    u8g2.sendBuffer();         // Update display
    
    delay(5000);               // Wait 5 seconds
    
    // Initialize NAU7802 sensor
    Serial.println("NAU7802");
    if (!nau.begin()) {
        Serial.println("Failed to find NAU7802");
        while (1) delay(10);   // Halt if sensor not found
    }
    Serial.println("Found NAU7802");

    // Configure NAU7802 settings
    nau.setLDO(NAU7802_3V0);  // Set voltage to 3.0V
    // Print LDO voltage setting
    Serial.print("LDO voltage set to ");
    switch (nau.getLDO()) {
        case NAU7802_4V5:  Serial.println("4.5V"); break;
        case NAU7802_4V2:  Serial.println("4.2V"); break;
        case NAU7802_3V9:  Serial.println("3.9V"); break;
        case NAU7802_3V6:  Serial.println("3.6V"); break;
        case NAU7802_3V3:  Serial.println("3.3V"); break;
        case NAU7802_3V0:  Serial.println("3.0V"); break;
        case NAU7802_2V7:  Serial.println("2.7V"); break;
        case NAU7802_2V4:  Serial.println("2.4V"); break;
        case NAU7802_EXTERNAL:  Serial.println("External"); break;
    }

    // Set and print gain settings
    nau.setGain(NAU7802_GAIN_128);  // Set gain to 128x
    Serial.print("Gain set to ");
    switch (nau.getGain()) {
        case NAU7802_GAIN_1:   Serial.println("1x"); break;
        case NAU7802_GAIN_2:   Serial.println("2x"); break;
        case NAU7802_GAIN_4:   Serial.println("4x"); break;
        case NAU7802_GAIN_8:   Serial.println("8x"); break;
        case NAU7802_GAIN_16:  Serial.println("16x"); break;
        case NAU7802_GAIN_32:  Serial.println("32x"); break;
        case NAU7802_GAIN_64:  Serial.println("64x"); break;
        case NAU7802_GAIN_128: Serial.println("128x"); break;
    }

    // Set and print sample rate
    nau.setRate(NAU7802_RATE_10SPS);  // Set to 10 samples per second
    Serial.print("Conversion rate set to ");
    switch (nau.getRate()) {
        case NAU7802_RATE_10SPS:  Serial.println("10 SPS"); break;
        case NAU7802_RATE_20SPS:  Serial.println("20 SPS"); break;
        case NAU7802_RATE_40SPS:  Serial.println("40 SPS"); break;
        case NAU7802_RATE_80SPS:  Serial.println("80 SPS"); break;
        case NAU7802_RATE_320SPS: Serial.println("320 SPS"); break;
    }

    // Take initial readings to stabilize sensor
    for (uint8_t i=0; i<10; i++) {
        while (!nau.available()) delay(1);
        nau.read();
    }

    // Perform internal calibration
    while (!nau.calibrate(NAU7802_CALMOD_INTERNAL)) {
        Serial.println("Failed to calibrate internal offset, retrying!");
        delay(1000);
    }
    Serial.println("Calibrated internal offset");

    // Perform system offset calibration
    while (!nau.calibrate(NAU7802_CALMOD_OFFSET)) {
        Serial.println("Failed to calibrate system offset, retrying!");
        delay(1000);
    }
    Serial.println("Calibrated system offset");
}

// Main loop function
void loop() {
    delay(1000);  // Wait 1 second between readings
    
    // Wait for new reading to be available
    while (!nau.available()) {
        delay(1);
    }
    
    // Read and convert weight measurement
    int32_t val = nau.read();
    float weight_gram;
    weight_gram = val / 108;    // Apply calibration factor
    weight_gram = weight_gram - 484;  // Apply offset correction
    Serial.print("Read "); 
    Serial.println(weight_gram);

    // Update OLED display
    static int counter = 0;
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(3, 16, "weight:");
    u8g2.drawStr(73, 16, String(weight_gram).c_str());
    u8g2.sendBuffer();
    
    counter++;  // Increment display counter
    
    // Check and reconnect MQTT if necessary
    if (!client.connected()) {
        reconnect();
    }
    client.loop();  // Process MQTT messages

    // Check if weight is below threshold
    if (weight_gram < 50) {
        if (!send_message) {
            client.publish(mqtt_topic, "1");  // Send activation signal
            delay(10*1000);                   // Wait 10 seconds
            client.publish(mqtt_topic, "0");  // Send deactivation signal
        }
    }

    // Wait for 24 hours before next reading
    delay(60*60*60*24*1000);
}