## instructions

**Setup Instructions for NAU7802 Weight Measurement System:**

**Hardware Requirements:**

- Arduino board
- NAU7802 load cell amplifier
- SSD1306 OLED display
- Load cell sensor
- Jumper wires

**Wiring:**

 NAU7802 to Arduino:
   - VDD → 3.3V
   - SDA → A4/SDA
   - SCL → A5/SCL
   - GND → GND

 **OLED Display:**

   - VCC → 3.3V
   - GND → GND
   - SDA → A4/SDA
   - SCL → A5/SCL

**Libraries Needed:**

- Adafruit_NAU7802
- U8g2lib
- Wire

**Calibration Steps:**

1. Connect load cell to NAU7802
2. Upload code to Arduino
3. Place known weight for calibration
4. Adjust divisor (currently 108) to match actual measurements

**Recommended Testing:**

- Verify serial monitor output
- Check OLED display readings
- Test with different weights
- Ensure stable mounting of load cell

**Troubleshooting:**

- Check all connections
- Verify library installations
- Ensure proper voltage levels