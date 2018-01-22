#include <Arduino.h>
#include <Wire.h>
#include <SSD1306.h>
#include <OLEDDisplayUi.h>

#include <SPI.h>
#include <RH_RF95.h>

#define RF95_FREQ 868.0

#define RFM95_CS 18
#define RFM95_RST 14
#define RFM95_INT 26

RH_RF95 rf95(RFM95_CS, RFM95_INT);

// OLED pins to ESP32 GPIOs via this connecthin:
#define OLED_ADDRESS 0x3c
#define OLED_SDA 4  // GPIO4
#define OLED_SCL 15 // GPIO15
#define OLED_RST 16 // GPIO16

#define SCK 5      // GPIO5 - SX1278's SCK
#define MISO 19    // GPIO19 - SX1278's MISO
#define MOSI 27    // GPIO27 - SX1278's MOSI
#define SS 18      // GPIO18 - SX1278's CS
#define RST 14     // GPIO14 - SX1278's RESET
#define DI0 26     // GPIO26 - SX1278's IRQ (interrupt request)
#define BAND 868E6 // 915E6

SSD1306 display(OLED_ADDRESS, OLED_SDA, OLED_SCL);
OLEDDisplayUi ui(&display);

unsigned int counter = 0;

void setup()
{
    //while (!Serial);
    Serial.begin(9600);
    delay(100);
    // put your setup code here, to run once:
    Serial.println("setup complete.");
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW); // low to reset OLED
    delay(50);
    digitalWrite(OLED_RST, HIGH); // must be high to turn on OLED

    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);    

    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);

    // manual reset
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);

    while (!rf95.init())
    {
        Serial.println("LoRa radio init failed");
        while (1)
            ;
    }
    Serial.println("LoRa radio init OK!");
    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
    if (!rf95.setFrequency(RF95_FREQ))
    {
        Serial.println("setFrequency failed");
        while (1)
            ;
    }
    Serial.print("Set Freq to: ");
    Serial.println(RF95_FREQ);

    // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

    // The default transmitter power is 13dBm, using PA_BOOST.
    // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
    // you can set transmitter powers from 5 to 23 dBm:
    rf95.setTxPower(23, false);
}

void loop()
{
    // put your main code here, to run repeatedly:
    Serial.println("loop");
    if (rf95.available())
    {
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t len = sizeof(buf);

        if (rf95.recv(buf, &len))
        {
            display.clear();
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.setFont(ArialMT_Plain_10);

            display.drawString(0, 0, "Received packet:");
            display.drawString(90, 0, String(counter));
            display.drawString(0, 20, "RSSI:");
            display.drawString(90, 20, String(rf95.lastRssi()));
            display.drawString(0, 40, "SNR:");
            display.drawString(90, 40, String(rf95.lastSNR()));
            display.display();
            counter++;
        }
    }    
}