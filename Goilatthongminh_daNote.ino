
#define MQ135pin A1  // đây là chân kết nối cảm biến chất lượng không khí (là chân A1- hay còn gọi là chân analog1)
#define voPin A2 //  đây là chân kết nối với cảm biến đọc bên trong cảm biến bụi - mình cứ hiểu là chân kết nối với cảm biến bụi
#define btnlen 3 // đây là chân kết nối với nút nhấn đẩy gối lên bằng nút , được kết nối với chân D3 (hay còn gọi là chân digital3 )
#define btnxuong 2 // tương tự nhưng đẩy gối xuống
#define WaterSensor A3 // chân cắm cảm biến phát hiện độ ẩm ( nước ) , chân A3
#define buzzer 10 // chân cắm còi cảnh báo , chân D10
int ledPin = 9; // chân led bên trong cảm biến bụi, chân D9 , đây là led phát tín hiệu của cảm biến bụi , còn chân A2 là Led thu tín hiệu
#include <Wire.h>   // thư viện I2c (nếu người ta hỏi dùng cho cái gì thì bảo dùng cho cảm biến nhiệt độ ,độ ẩm )
#include "Adafruit_SHT31.h" // thư viện cho cảm biến nhiệt độ, độ ẩm SHT31
#include "SharpGP2Y10.h" // thư viện cho cảm biến bụi 2.5PM
#include "SoftwareSerial.h" // thư viện để tạo các cổng Uart ảo  (Serial ảo) , nó cho phép mình chọn 2 chân bất kì trên arduino làm chân UART thay vì chỉ có 1 cổng duy nhất trên Arduino

#define SIMRX 5 // Chân cắm module sim
#define SIMTX 4 // chân cắm module sim

unsigned long lastSendData = 0; // các biến để lưu mốc thời gian
bool check = true, check1 = true, btnlenVal = true, btnxuongVal = true, dongColen = false, dongCoxuong = false; // tạo các biến là set trạng thái ban đầu cho các biến 

SharpGP2Y10 dustSensor(voPin, ledPin);  // khai báo chân và tên( lưu ý "dustSensor" nó chỉ là cái tên do mình đặt) cho cảm biến bụi
Adafruit_SHT31 sht31 = Adafruit_SHT31(); // khai báo cảm biến nhiệt độ độ ẩm SHT31
SoftwareSerial ESP_Serial(6, 7); // tạo Cổng Uart ảo để giao tiếp dữ liệu với ESP 8266
SoftwareSerial mysim(SIMRX, SIMTX); //// tạo Cổng Uart ảo để giao tiếp dữ liệu với module sim
float h, t, dustDensity; // tạo biến lưu giá trị cảm biến , h:độ ẩm , t: nhiệt độ,  dustDensity: cảm biến bụi
int MQ135; // biến lưu chỉ số chất lượng không khí
int WaterValue = 0; // biến lưu trạng thái xem có phát hiện nước hay ko ( tè dầm)
char phoneNum[] = "+84327824568";  //change this   // đây là số điện thoại để thiết bị nhắn tin cảnh báo về 

void setup() {
  //====================================================
  // hàm pinMode là hàm khởi tạo các thiết bị đầu ra đầu vào
  pinMode(btnlen, INPUT_PULLUP);
  pinMode(btnxuong, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  pinMode(ledPin, OUTPUT);
  //=================================================
// các câu lệnh có .begin() là các câu lệnh khởi tạo nhé

  Serial.begin(9600); // khởi tạo cổng Uart để giao tiếp với máy tính , tốc độ 9600
  Serial.println("SHT31 test");  // các lệnh Serial.print đơn thuần chỉ là in ra màn hình máy tính , k quan trọng
  if (!sht31.begin(0x44)) { // khởi tạo cảm biến SHT31
    Serial.println("Couldn't find SHT31");
  }
  ESP_Serial.begin(4800);
  mysim.begin(9600);
//==================================================================================================================

  sendSMS(phoneNum, "test"); // gửi thử tin nhắn đến SĐT mỗi lần khởi động xem  Sim đã chạy chưa
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  readSensor(); // gọi hàm  tên là  "readSensor"  , hàm này anh viết bên dưới
  btnlenVal = digitalRead(btnlen); // đọc giá trị nút nhấn lên và lưu trạng thái vào biến
  btnxuongVal = digitalRead(btnxuong); // đọc giá trị nút nhấn xuống và lưu trạng thái vào biến
  if (btnlenVal == false) {  // kiểm tra trạng thái
    dongColen = true; // nhấn thì động cơ chạy lên
  } else {
    dongColen = false; // nếu k nhấn thì dừng động cơ
  }
  if (btnxuongVal == false) { // tương tự
    dongCoxuong = true;
  } else {
    dongCoxuong = false;
  }
  if (dustDensity > 10 || t > 45 || h > 80 || MQ135 > 800 || WaterValue > 500) { // một trong các chỉ số này vượt ngưỡng thì sẽ cảnh báo bằng còi và tin nhắn
    digitalWrite(buzzer, HIGH); 
    if (check1 == true) {
      sendSMS(phoneNum, "Canh bao chi so moi truong!!");
      check1 = false;
    }

  } else {  // ngược lại thì còi tắt
    check1 = true;
    digitalWrite(buzzer, LOW);
  }


  if (millis() - lastSendData > 3000) {  // cứ 3 giây thì gửi dữ liệu sang ESP8266 một lần

    update_data();
    lastSendData = millis();
  }
}


// void motorControl(int len, int xuong) {
//   analogWrite(L_PWM, len);
//   analogWrite(R_PWM, xuong);
// }


void readSensor() { // hàm đọc giá trị cảm biến
  t = sht31.readTemperature(); // đọc giá trị nhiệt độ   // hàm readTemperature()  có sẵn trong thư viện của SHT nên chỉ việc lôi ra thôi
  h = sht31.readHumidity(); // đọc giá trị độ ẩm // tương tự trên
  MQ135 = analogRead(MQ135pin); // đọc giá trị cảm biến đo chỉ số chất lượng không khí
  dustDensity = dustSensor.getDustDensity(); // đọc giá trị cảm biến bụi // hàm getDustDensity() cũng nằm sẵn trong thư viện nên chỉ việc lôi ra dùng
  WaterValue = analogRead(WaterSensor); // đọc giá trị cảm biến độ ẩm 

}




void update_data() {  // hàm gửi data từ arduino nano sang ESP8266
  String data1[7] = { String(h), String(t), String(MQ135), String(dustDensity), String(WaterValue), String(dongColen), String(dongCoxuong) };
  String combinedData = "";  // Khởi tạo chuỗi kết quả

  // Kết hợp các dữ liệu vào chuỗi
  for (int i = 0; i < 7; i++) {
    combinedData += data1[i];
    if (i < 6) {
      combinedData += ",";  // Thêm dấu phẩy nếu không phải là phần tử cuối cùng
    }
  }
  combinedData += "\n";  // Thêm ký tự xuống dòng

  // Gửi chuỗi dữ liệu qua UART
  ESP_Serial.print(combinedData);
  Serial.print(combinedData);
 
}
void initModule(String cmd, char *res, int t) { // hàm kiểm tra xem sim có khả dụng hay không
  while (1) {
    Serial.println(cmd);
    mysim.println(cmd);
    delay(100);
    while (mysim.available() > 0) {
      if (mysim.find(res)) {
        Serial.println(res);
        delay(t);
        return;
      } else {
        Serial.println("Error");
      }
    }
    delay(t);
  }
}
void sendSMS(char *number, char *msg) { // hàm gửi tin nhắn
  mysim.print("AT+CMGS=\"");
  mysim.print(number);
  mysim.println("\"\r\n");  // AT+CMGS=”Mobile Number” <ENTER> - Assigning recipient’s mobile number
  delay(500);
  mysim.println(msg);  // Message contents
  delay(500);
  mysim.write(byte(26));  // Ctrl+Z  send message command (26 in decimal).
  delay(3000);
}
