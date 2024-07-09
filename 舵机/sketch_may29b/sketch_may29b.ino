//WIFI 连接库和MQTT客户端
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <Adafruit_PWMServoDriver.h>

#define SERVO_0 102 
#define SERVO_45 187 
#define SERVO_90 280 
#define SERVO_135 373 
#define SERVO_180 510



int SERVO[16]={0};

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVOMIN  102 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  500 // this is the 'maximum' pulse length count (out of 4096)

// WiFi 账号密码
const char *ssid = "abc"; // Wifi 账号
const char *password = "czq123456";  // wifi 密码
 
// MQTT Broker 服务端连接
const char *mqtt_broker = "esp.icce.top";//mqtt服务器地址
const char *LocationTopic = "location"; //订阅主题
const char *mqtt_username = "czq";
const char *mqtt_password = "12345678";
const int mqtt_port = 1883;//端口
 
//客户端变量
WiFiClient espClient;
PubSubClient client(espClient);

//设置pwm
void setup_pwm() 
{
  pwm.begin(); 
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
}

// connecting to a WiFi network
void setup_wifi()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to the WiFi network");
}

//reconnect to mqtt
void reconnect() {
  // Loop until we're reconnected
    while(!client.connected()) {
        String client_id = "esp8266-client";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public emqx mqtt broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());//返回连接状态
            delay(1000);
        }
    }
    client.subscribe(LocationTopic);
}

void callback(char* topic, byte* payload, unsigned int length) {

  char message[length+1]; // 创建一个字符数组，比字节数据的长度多一个字符，用于存储空字符
  memcpy(message, payload, length); // 将字节数据复制到字符数组中
  message[length] = '\0'; // 添加空字符到字符数组的末尾

  Serial.println(message);

  if (strcmp(topic, LocationTopic) == 0) 
  {
    if(strcmp(message, "01-1") == 0)
      SERVO[2]=1;
    else if(strcmp(message, "01-2") == 0)
      SERVO[1]=1;
    else if(strcmp(message, "01-3") == 0)
      SERVO[0]=1;
    else if(strcmp(message, "02-1") == 0)
      SERVO[6]=1;
    else if(strcmp(message, "02-2") == 0)
      SERVO[5]=1;
    else if(strcmp(message, "02-3") == 0)
      SERVO[4]=1;
    else if(strcmp(message, "03-1") == 0)
      SERVO[10]=1;
    else if(strcmp(message, "03-2") == 0)
      SERVO[9]=1;
    else if(strcmp(message, "03-3") == 0)
      SERVO[8]=1;
    else if(strcmp(message, "04-1") == 0)
      SERVO[14]=1;
    else if(strcmp(message, "04-2") == 0)
      SERVO[13]=1;
    else if(strcmp(message, "04-3") == 0)
      SERVO[12]=1;
  }

}


void setup() {
  Serial.begin(115200);
  setup_wifi();
  setup_pwm();
  //connecting to a mqtt broker 连接服务端
  client.setServer(mqtt_broker, mqtt_port);
  // publish and subscribe
  client.subscribe(LocationTopic);
  client.setCallback(callback);//回调
  
}

void loop() {
  if ( !client.connected() ) {
    reconnect();
  }
  delay(200);
  for(int i=0;i<16;i++) 
  {
    if(SERVO[i]==1) 
    {

      for (uint16_t pulselen = SERVOMAX; pulselen > SERVOMIN; pulselen--)
        pwm.setPWM(i, 0, pulselen);

      delay(500);

      for (uint16_t pulselen = SERVOMIN; pulselen < SERVOMAX; pulselen++) 
        pwm.setPWM(i, 0, pulselen);

      delay(500);

      SERVO[i]=0;

    }
  }
  
  client.loop();//循环
}


