#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>

int sensorPin1 = 34;  // 寻迹模块引脚1
int sensorPin2 = 39;  // 寻迹模块引脚2
int sensorPin3 = 35;  // 寻迹模块引脚3
int sensorPin4 = 36;  // 寻迹模块引脚4
int sensorValue1 = digitalRead(sensorPin1);
int sensorValue2 = digitalRead(sensorPin2);
int sensorValue3 = digitalRead(sensorPin3);
int sensorValue4 = digitalRead(sensorPin4);
int angle=90;//摄像头初始化角度

const int channel_1 = 3;
const int channel_2 = 4;
const int channel_3 = 6;
const int channel_4 = 7;

//定义机械臂结构体
Servo servo_jiazhao;
Servo servo_zhongzhou;    
Servo servo_dipan;
Servo servo_shexiangtou;

bool setup_flag = 0;//小车启动标志位
bool stop_flag = 0;//小车停止标志位
bool stop_flag2 = 0;//小车停止标志位2
bool stop_flag3 = 0;
bool clamp_flag = 0;//机械臂夹取标志位
bool put_flag = 0;//机械臂放置标志位
bool stepforward_flag = 0;//步进标志位
bool stepbackward_flag = 0;//步退标志位


int line_flag = 0;  //小车位于第几条黑线
bool wrongbook1 = 0;  //从左到右第一本书错误标志位
bool wrongbook2 = 0;  //从左到右第二本书错误标志位
bool wrongbook3 = 0;  //从左到右第三本书错误标志位


bool view_flag = 0; //视觉识别标志位

int back_flag = 0;  //判断小车后退了几条黑线


// Wi-Fi网络设置
const char* ssid = "abc";
const char* password = "czq123456";

// MQTT服务器设置
const char* mqttServer = "esp.icce.top";
const int mqttPort = 1883;
const char* mqttUser = "esp32";
const char* mqttPassword = "";
const char* CarTopic = "Car";//名为Car的主题，用于控制小车轮子和机械臂
const char* BookTopic = "WrongBook";//名为WrongBook的主题，用于接收错书信息
const char* ViewTopic = "view"; //名为view的主题，用于视觉识别开始和判断识别是否完成

//创建一个 Wi-Fi 客户端对象和一个 MQTT 客户端对象
WiFiClient espClient;
PubSubClient client(espClient);

void Motor_Setup(int motorID, int pin1, int pin2) {  //电机初始化 ID=1~4 定义四组电机
  ledcSetup(motorID * 2 - 2, 5000, 8);
  ledcAttachPin(pin1, motorID * 2 - 2);
  ledcSetup(motorID * 2 - 1, 5000, 8);
  ledcAttachPin(pin2, motorID * 2 - 1);
}
void Motor_Speed(int motorID, int speed) {  //电机速度设置 ID=1~4,speed=-255~255
  if (speed == 0) {
    ledcWrite(motorID * 2 - 2, 0);
    ledcWrite(motorID * 2 - 1, 0);
  } else if (speed > 0) {
    ledcWrite(motorID * 2 - 2, speed);
    ledcWrite(motorID * 2 - 1, 0);
  } else {
    ledcWrite(motorID * 2 - 2, 0);
    ledcWrite(motorID * 2 - 1, -speed);
  }
}

void Forward() {
  // 寻迹模块读数为0100时，车子轻微向左校正方向
  if (sensorValue1 == LOW && sensorValue2 == HIGH && sensorValue3 == LOW && sensorValue4 == LOW) {
    Motor_Speed(channel_1, 165);  // 右前轮
    Motor_Speed(channel_2, 165);  // 右后轮
    Motor_Speed(channel_3, 150);  // 左后轮（反向）
    Motor_Speed(channel_4, 150);  // 左前轮（反向）
  }
  // 寻迹模块读数为0010时，车子轻微向右校正方向
  else if (sensorValue1 == LOW && sensorValue2 == LOW && sensorValue3 == HIGH && sensorValue4 == LOW) {
    Motor_Speed(channel_1, 150);  // 右前轮（反向）
    Motor_Speed(channel_2, 150);  // 右后轮（反向）
    Motor_Speed(channel_3, 165);  // 左后轮
    Motor_Speed(channel_4, 165);  // 左前轮
  }
  // 寻迹模块读数为1100或1000时，车子大幅向左校正方向
  else if ((sensorValue1 == HIGH && sensorValue2 == HIGH && sensorValue3 == LOW && sensorValue4 == LOW) || (sensorValue1 == HIGH && sensorValue2 == LOW && sensorValue3 == LOW && sensorValue4 == LOW)) {
      Motor_Speed(channel_1, 180);  // 右前轮
      Motor_Speed(channel_2, 180);  // 右后轮
      Motor_Speed(channel_3, -180);  // 左后轮（反向）
      Motor_Speed(channel_4, -180);  // 左前轮（反向）
  }
  // 寻迹模块读数为0011或0001时，车子大幅向右校正方向
  else if ((sensorValue1 == LOW && sensorValue2 == LOW && sensorValue3 == HIGH && sensorValue4 == HIGH) || (sensorValue1 == LOW && sensorValue2 == LOW && sensorValue3 == LOW && sensorValue4 == HIGH)) {
      Motor_Speed(channel_1, -190);  // 右前轮（反向）
      Motor_Speed(channel_2, -190);  // 右后轮（反向）
      Motor_Speed(channel_3, 190);  // 左后轮
      Motor_Speed(channel_4, 190);  // 左前轮
  }
  //其余所有情况，车子直行
  else{
    Motor_Speed(channel_1, 160);  // 右前轮
    Motor_Speed(channel_2, 160);  // 右后轮
    Motor_Speed(channel_3, 160);  // 左后轮
    Motor_Speed(channel_4, 160);  // 左前轮
  }
}

void Backward() {
  // 寻迹模块读数为0100时
  if (sensorValue1 == LOW && sensorValue2 == HIGH && sensorValue3 == LOW && sensorValue4 == LOW) {
    Motor_Speed(channel_1, -165);  // 右前轮(反向)
    Motor_Speed(channel_2, -165);  // 右后轮(反向)
    Motor_Speed(channel_3, -150);  // 左后轮
    Motor_Speed(channel_4, -150);  // 左前轮
  }
  // 寻迹模块读数为0010时
  else if (sensorValue1 == LOW && sensorValue2 == LOW && sensorValue3 == HIGH && sensorValue4 == LOW) {
    Motor_Speed(channel_1, -150);  // 右前轮
    Motor_Speed(channel_2, -150);  // 右后轮
    Motor_Speed(channel_3, -165);  // 左后轮(反向)
    Motor_Speed(channel_4, -165);  // 左前轮(反向)
  }
  // 寻迹模块读数为1100或1000时
  else if ((sensorValue1 == HIGH && sensorValue2 == HIGH && sensorValue3 == LOW && sensorValue4 == LOW) || (sensorValue1 == HIGH && sensorValue2 == LOW && sensorValue3 == LOW && sensorValue4 == LOW)) {
    Motor_Speed(channel_1, -180);  // 右前轮(反向)
    Motor_Speed(channel_2, -180);  // 右后轮(反向)
    Motor_Speed(channel_3, 180);   // 左后轮
    Motor_Speed(channel_4, 180);   // 左前轮
  }
  // 寻迹模块读数为0011或0001时
  else if ((sensorValue1 == LOW && sensorValue2 == LOW && sensorValue3 == HIGH && sensorValue4 == HIGH) || (sensorValue1 == LOW && sensorValue2 == LOW && sensorValue3 == LOW && sensorValue4 == HIGH)) {
    Motor_Speed(channel_1, 190);   // 右前轮
    Motor_Speed(channel_2, 190);   // 右后轮
    Motor_Speed(channel_3, -190);  // 左后轮(反向)
    Motor_Speed(channel_4, -190);  // 左前轮(反向)
  }
  // 其余所有情况,车子直行
  else {
    Motor_Speed(channel_1, -160);  // 右前轮(反向)
    Motor_Speed(channel_2, -160);  // 右后轮(反向)
    Motor_Speed(channel_3, -160);  // 左后轮(反向)
    Motor_Speed(channel_4, -160);  // 左前轮(反向)
  }
}


void Stop(){
    Motor_Speed(channel_1, 0);  
    Motor_Speed(channel_2, 0);  
    Motor_Speed(channel_3, 0);  
    Motor_Speed(channel_4, 0);  
}

//去第三条黑线
void go_three()
{
  while(1)
  {
    sensorValue1 = digitalRead(sensorPin1);
    sensorValue2 = digitalRead(sensorPin2);
    sensorValue3 = digitalRead(sensorPin3);
    sensorValue4 = digitalRead(sensorPin4);

    //判断小车是否碰到黑线，如果四个探头检测数据为1111，且stop_flag==0，则发布一个stop消息让小车停下，并且让stop_flag变为1，防止重复进入停车状态
    if (sensorValue1 == HIGH && sensorValue2 == HIGH && sensorValue3 == HIGH && sensorValue4 == HIGH && stop_flag == 0)
    {
      // 发布停车指令,龙芯端需要接收这个指令，延时1S后开始视觉识别，识别完毕后返回一个clamp
      line_flag++;
      if(line_flag >= 3)
      {
        line_flag = 0;
        stop_flag = 1;
      }
      else
        delay(100);
    }
    if(stop_flag == 1)
    {
      Stop();
      return;
    }
    else
      Forward();
  }
}

//去第二条黑线
void go_two()
{
  while(1)
  {
    sensorValue1 = digitalRead(sensorPin1);
    sensorValue2 = digitalRead(sensorPin2);
    sensorValue3 = digitalRead(sensorPin3);
    sensorValue4 = digitalRead(sensorPin4);

    //判断小车是否碰到黑线，如果四个探头检测数据为1111，且stop_flag==0，则发布一个stop消息让小车停下，并且让stop_flag变为1，防止重复进入停车状态
    if (sensorValue1 == HIGH && sensorValue2 == HIGH && sensorValue3 == HIGH && sensorValue4 == HIGH && stop_flag == 0)
    {
      stop_flag = 1;
      line_flag = -1;
    }

    if(stop_flag == 1)
    {
      Stop();
      return;
    }
    else
      Backward();
  }
}

//去第一条黑线
void go_one()
{
  back_flag = 0;
  while(1)
  {
    sensorValue1 = digitalRead(sensorPin1);
    sensorValue2 = digitalRead(sensorPin2);
    sensorValue3 = digitalRead(sensorPin3);
    sensorValue4 = digitalRead(sensorPin4);

    //判断小车是否碰到黑线，如果四个探头检测数据为1111，且stop_flag==0，则发布一个stop消息让小车停下，并且让stop_flag变为1，防止重复进入停车状态
    if (sensorValue1 == HIGH && sensorValue2 == HIGH && sensorValue3 == HIGH && sensorValue4 == HIGH && stop_flag == 0)
    {
      back_flag++;
      if(back_flag >= 2)
      {
        stop_flag = 1;
        line_flag = -2;
      }
      else
        delay(100);
    }
    if(stop_flag == 1)
    {
      Stop();
      return;
    }
    else
      Backward();
  }
}

//向前微调小车位置，传入一个时间参数
void step_forward(int duration_ms) {
  // 记录当前时间
  unsigned long start_time = millis();

  while (true) {
    if(stepforward_flag == 1)
    {
      if (millis() - start_time >= duration_ms) 
      {
        Stop();
        stepforward_flag = 0;
        return;
      }
      else
        Forward();
    }
    else
    {
      Stop();
      return;
    }
  }
}

//向后微调小车位置，传入一个时间参数
void step_backward(int duration_ms) {
  // 记录当前时间
  unsigned long start_time = millis();

  while (true) {
    if(stepbackward_flag == 1)
    {
      if (millis() - start_time >= duration_ms) 
      {
        Stop();
        stepbackward_flag = 0;
        return;
      }
      else
        Backward();
    }
    else
    {
      Stop();
      return;
    }
  }
}


void Clamp(){  
  
  //还原机械臂初始状态
  servo_jiazhao.write(180);
  servo_zhongzhou.write(90);
  servo_dipan.write(1);
  delay(1000);

  //将中轴角度调整到5度
  for (int pos = 90; pos >= 5; pos--){
    servo_zhongzhou.write(pos);
    delay(10);
  }
  // servo_zhongzhou.write(5);
  delay(2000);
  // 将夹爪角度调整为125度
  servo_jiazhao.write(125);
  delay(1500);
  // 将中轴角度缓慢调整到90度
  for (int pos = 5; pos <= 110; pos++){
    servo_zhongzhou.write(pos);
    delay(30);
  }

  clamp_flag = 1;
}

void Put(){

  // 将底盘角度调整到170度
  servo_dipan.write(170);
  delay(2000);
  // 将中轴角度调整到30度
  for (int pos = 110; pos >= 30; pos--){
    servo_zhongzhou.write(pos);
    delay(30);
  }
  // servo_zhongzhou.write(30);
  // delay(2000);
  // 将夹爪角度调整为180度
  delay(1000);
  servo_jiazhao.write(180);
  delay(1000);

  //还原机械臂初始状态
  servo_zhongzhou.write(90);
  delay(1000);
  servo_jiazhao.write(180);
  servo_dipan.write(1);
  delay(1000);

  put_flag = 0;//执行放置完毕之后复位标志
}


void setup() {

  Serial.begin(115200);
  // 连接到Wi-Fi网络
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to the WiFi network");
  
  Serial.println("Connected to WiFi");

  // 设置MQTT服务器和回调函数
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  // 连接到MQTT服务器
  while (!client.connected()) {
    Serial.println("Connecting to MQTT server...");
    
    if (client.connect("ESP32Client", mqttUser, mqttPassword )) {
      Serial.println("Connected to MQTT server");
      client.subscribe(CarTopic);
      client.subscribe(BookTopic);
      client.subscribe(ViewTopic);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
  
  //初始化电机
  Motor_Setup(channel_1, 27, 13); 
  Motor_Setup(channel_2, 2, 4);
  Motor_Setup(channel_3, 17, 12);
  Motor_Setup(channel_4, 15, 14);
  Stop();

  //初始化机械臂，夹爪角度为180, 中轴角度为90, 底盘角度为0     
  servo_jiazhao.attach(25,500,2500);   // 定义25号引脚为夹爪
  servo_zhongzhou.attach(26,500,2500);   // 定义26号引脚为中轴
  servo_dipan.attach(33,500,2500);   // 定义33号引脚为底盘
  servo_shexiangtou.attach(32,500,2500);   // 定义32号引脚为摄像头

  servo_jiazhao.write(180);
  delay(500);
  servo_zhongzhou.write(90);
  delay(500);
  servo_dipan.write(1);
  delay(500);

  //初始化寻迹
  pinMode(sensorPin1, INPUT);
  pinMode(sensorPin2, INPUT);
  pinMode(sensorPin3, INPUT);
  pinMode(sensorPin4, INPUT);

}

void loop() {
  //如果连接mqtt失败，重新连接mqtt服务器
  if (!client.connected()) 
  {
    reconnect();
  }

  //启动
  if(setup_flag == 1)
  {
    setup_flag = 0;

    //去第三条黑线
    step_forward(240);
    go_three();
    delay(1000);
    step_backward(225);
    //step_backward(185);
    delay(1000);

    //准备识别
    client.publish(ViewTopic, "开始视觉识别");
  }

  //识别完成后的动作
  if(view_flag == 1 && stop_flag3 != 1)
  {
    view_flag = 0;
    delay(2000);

    if(wrongbook1 == 1)   //识别到从左到右第一本书错误
    {
      Clamp();
      delay(1000);    

      Put();
      delay(1000); 
    }
    else if(wrongbook2 == 1)  //识别到从左到右第二本书错误
    {
      //去第二条黑线
      step_backward(240);
      go_two();
      delay(1000);
      step_forward(180);
      //step_forward(185);
      delay(1000);

      //机械臂抓取
      Clamp();

      delay(1000);

      Put();
      delay(1000);
    }
    else if(wrongbook3 == 1)  //识别到从左到右第三本书错误
    {
      //去第一条黑线
      step_backward(240);
      go_one();
      delay(1000);
      step_forward(190);
      //step_forward(185);
      delay(1000);

      //机械臂抓取
      Clamp();  

      delay(1000);   

      Put();
      delay(1000);
    }
    
    stop_flag = 0;
    stop_flag2 = 0;
    stepbackward_flag = 1;
    stepforward_flag = 1;

    //去下一个第三条黑线
    step_forward(240);
    go_three();
    delay(1000);
    step_backward(225);
    //step_backward(185);
    delay(1000);

    //判断是否要放书
    // if(clamp_flag == 1)
    // {
    //   clamp_flag = 0;
    //   Put();
    //   delay(1000);
    // }

    //容易断开服务器
    if (!client.connected()) 
    {
      reconnect();
    }
    delay(100);
    //准备识别
    client.publish(ViewTopic, "开始视觉识别");

  }

  //保持MQTT客户端与服务器之间的连接
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {

  char message[length+1]; // 创建一个字符数组，比字节数据的长度多一个字符，用于存储空字符
  memcpy(message, payload, length); // 将字节数据复制到字符数组中
  message[length] = '\0'; // 添加空字符到字符数组的末尾

  Serial.println(message);

  if (strcmp(topic, ViewTopic) == 0) 
  {
    if(strcmp(message, "视觉识别完成") == 0)
      view_flag = 1;
  }

  if (strcmp(topic, BookTopic) == 0) 
  {
    if(strcmp(message, "3") == 0)
    {
        wrongbook3 = 1;
        wrongbook2 = 0;
        wrongbook1 = 0;
        stop_flag = 0;
        stop_flag2 = 0;
        stepbackward_flag = 1;
        stepforward_flag = 1;

    }
    else if(strcmp(message, "2") == 0)
    {
        wrongbook3 = 0;
        wrongbook2 = 1;
        wrongbook1 = 0;
        stop_flag = 0;
        stop_flag2 = 0;
        stepbackward_flag = 1;
        stepforward_flag = 1;

    }
    else if(strcmp(message, "1") == 0)
    {
        wrongbook3 = 0;
        wrongbook2 = 0 ;
        wrongbook1 = 1;
        stop_flag = 0;
        stop_flag2 = 0;
        stepbackward_flag = 1;
        stepforward_flag = 1;

    }
    else if(strcmp(message, "没有错书") == 0)
    {
        wrongbook3 = 0;
        wrongbook2 = 0 ;
        wrongbook1 = 0;
        stop_flag = 0;
        stop_flag2 = 0;
        stepbackward_flag = 1;
        stepforward_flag = 1;
    }
  }

  if (strcmp(topic, CarTopic) == 0) 
  {
    if (strcmp(message, "setup") == 0) 
    {
      wrongbook2 = 0;
      wrongbook3 = 0;
      wrongbook1 = 1;

      stop_flag = 0;
      stop_flag2 = 0;
      stepbackward_flag = 1;
      stepforward_flag = 1;

      setup_flag = 1;
      stop_flag3 = 0;
    } 
    if (strcmp(message, "stop") == 0) 
    {
      wrongbook2 = 0;
      wrongbook3 = 0;
      wrongbook1 = 0;

      stop_flag = 0;
      stop_flag2 = 0;
      stepbackward_flag = 1;
      stepforward_flag = 1;

      setup_flag = 0;

      stop_flag3 = 1;

    } 
  }
  if (strncmp(message, "angle:", 6) == 0) 
  {
    angle = atoi(message + 6);
    // 限制角度值在25-155度之间
    if (angle < 25) {
      angle = 25;
    } else if (angle > 155) {
      angle = 155;
    }
    Serial.print("摄像头转到角度: ");
    Serial.println(angle);
    servo_shexiangtou.write(angle);  // 根据收到的角度值转动舵机
  } 
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    
    if (client.connect("ESP32Client", mqttUser, mqttPassword )) {
      Serial.println("Connected to MQTT server");
      client.subscribe(CarTopic);
      client.subscribe(BookTopic);
      client.subscribe(ViewTopic);     
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}