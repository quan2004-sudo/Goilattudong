/*************************************************************

  This is a simple demo of sending and receiving some data.
  Be sure to check out other examples!
 *************************************************************/

/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6Nmo5lgbt"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "rL3JE2n1IVg-YymC0WBoRdEnCzkiT2aX"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiManager.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_MLX90614.h>

String Heart_SpO2;

unsigned long lastUpdate=0;
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "AmericanStudy T1";
char pass[] = "66668888";

BlynkTimer timer;
WiFiManager wifiManager;


PulseOximeter pox; // hàm khai báo tên cho cảm biến SpO2
Adafruit_MLX90614 mlx = Adafruit_MLX90614(); // hàm khai báo cho cảm biến thân nhiệt 

float nhiptim, oxy, nhietdo;



// This function is called every time the device is connected to the Blynk.Cloud




void setup() {
  // Debug console
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  wifiManager.autoConnect("GoiLat2", "88888888");
  Serial.println();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
   if (!pox.begin()) {
    Serial.println("POX: FAILED");
    failed();
    for (;;)
      failed();
  } else {
    Serial.println("POX: SUCCESS");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_14_2MA);

  mlx.begin(0x5A);

  Wire.setClock(100000);


  
}

void loop() {
  Blynk.run();
  timer.run();
    pox.update();
    pox_mlx_read();
     Heart_SpO2 = "NHIPTIM:" +String(nhiptim)+"  SpO2:" +String(oxy) +"  THANNHIET:" +String(nhietdo) +"\n"; // ép các giá trị thành dạng chuỗi để gửi lên Blynk
     Serial.println(Heart_SpO2);
 if (millis()- lastUpdate>3000){ // 3giay đồng bộ 1 lần
 Blynk.virtualWrite(V7, Heart_SpO2); // đồng bộ dữ liệu từ ESP 8266 lên Blynk
 lastUpdate = millis();
 }
 
 
}

void pox_mlx_read() { // hàm đọc giá trị cảm biến SpO2 và nhịp tim
  nhiptim = pox.getHeartRate();
  oxy = pox.getSpO2();
  nhietdo = mlx.readObjectTempC();
}



void failed() { // nếu khởi tạo lỗi thì nó sẽ chạy hàm nháy led
  digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));
  delay(100);
  
}
