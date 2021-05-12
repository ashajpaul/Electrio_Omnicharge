/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini
   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
   Has a characteristic of: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - used for receiving data with "WRITE" 
   Has a characteristic of: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - used to send data with  "NOTIFY"
   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.
   In this example rxValue is the data received (only accessible inside that function).
   And txValue is the data to be sent, in this example just a byte incremented every second. 
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>


#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

//Display Pins (Pins for wiring reference)
#define CS         5    //D5
#define RST        A13  //D15
#define DC         A12  //D2
#define MOSI       23   //D23 - Data out
#define SCLK       18   //D18 - Clock out

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
float solarValue = 0;
float batValue = 0;
const int solarPin = A14; // solar voltage pin 
//const int solarPin = A16;   //solar current pin
const int batPin = A16;     // get correct pin
char prevSolar[8];          // make sure this is big enuffz
char prevBat[8];
char newSolar[8];
char newBat[8]; 
char values[20];
int voltLen;
Adafruit_ST7789 tft = Adafruit_ST7789(CS, DC, MOSI, SCLK, RST); //TFT object


//std::string rxValue; // Could also make this a global var to access it in loop()

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");

        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }
      }
    }
};

void setup() {
  Serial.begin(9600);

  tft.init(240, 320); //Initialize tft display
  tft.setSPISpeed(40000000); //SPI speed at 40M
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  //int row, int col, char *text, uint16_t colorText, uint16_t colorBack, int fontSize, bool textWrap
  drawtext(0, 0, "STARTING UP DEVICE!", ST77XX_GREEN, 3, true);

  // Create the BLE Device
  BLEDevice::init("Arya ESP32"); // Give it a name
  

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
                      
  pCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
  tft.fillScreen(ST77XX_BLACK);

  //Setting titles for LCD
  drawtext(0, 5, "Charge Controller", ST77XX_GREEN, 3, true); //Title
  drawtext(50, 10, "Solar Output:", ST77XX_GREEN, 3, true); //Solar Output
  drawtext(150, 10, "Battery Voltage:", ST77XX_GREEN, 3, true);//Battery Voltage
  drawtext(90, 175, "V", ST77XX_GREEN, 3, true);//Voltage label for solar
  drawtext(190, 175, "V", ST77XX_GREEN, 3, true);//Voltage label for battery
}

void loop() {
  if (deviceConnected) {
    
    //solarValue = random(5);
    solarValue = analogRead(solarPin) * 0.00401; 
    //solarValue = analogRead(solarPin);
    //batValue = random(5);
    batValue = analogRead(batPin) * 0.0041;
    // Let's convert the value to a char array:

    Serial.print("Solar value: "); 
    Serial.println(solarValue);
    Serial.print("Battery value: ");  
    Serial.println(batValue); 
    
    dtostrf(solarValue, 1, 2, newSolar); // float_val, min_width, digits_after_decimal, char_buffer
    dtostrf(batValue, 1, 2, newBat);

    voltLen = 1 + strlen(newSolar) + strlen(newBat);

    for (int i=0; i < voltLen; i++) {
      if (i < strlen(newSolar)) {
        values[i] = newSolar[i];
      }
      else if (i == (strlen(newSolar))) {
        values[i] = '-';
      }
      else {
        values[i] = newBat[i % (strlen(newSolar) + 1)];
      }
    }
    
    /*for (int i=0; i < 10; i++) {
      if (i < 4) {
        values[i] = newSolar[i];
      }
      else if (i == 4) {
        values[i] = '-';
      }
      else {
        values[i] = newBat[i-5];
      }
    }*/

//    pCharacteristic->setValue(&txValue, 1); // To send the integer value
//    pCharacteristic->setValue("Hello!"); // Sending a test message
    pCharacteristic->setValue(values);

    
    
    drawtext(90, 30, prevSolar, ST77XX_BLACK, 3, true);
    drawtext(90, 30, newSolar, ST77XX_GREEN, 3, true);
    
    drawtext(190, 30, prevBat, ST77XX_BLACK, 3, true);
    drawtext(190, 30, newBat, ST77XX_GREEN, 3, true);

    strcpy(prevSolar, newSolar);
    strcpy(prevBat, newBat);
    //drawtext(10, 100, solar, ST77XX_GREEN, ST77XX_BLACK, 3, true);//Solar data
    //drawtext(20, 100, bat, ST77XX_GREEN, ST77XX_BLACK, 3, true);//Battery data
    
    pCharacteristic->notify(); // Send the value to the app!
    Serial.print("*** Sent Value: ");
    Serial.print(values);
    Serial.println(" ***");
  }
  delay(1000); //Change this accordingly to sampling rate
}

void drawtext(int row, int col, char *text, uint16_t colorText, int fontSize, bool textWrap) {
  tft.setCursor(col, row);
  tft.setTextColor(colorText);
  tft.setTextSize(fontSize);
  tft.setTextWrap(textWrap);
  tft.print(text);
}
