/********************  介绍  **************************
  项目名称/Project          : Nodemcu与贝壳物联实时通信/Nodemcu-Communicate-Bigiot
  程序名称/Program name     : NCB
  作者/Author               : 燊哥仔/Leo
  版本/Version              : 1.0
  日期/Date（YYYYMMDD）     : 20210219
  程序目的/Purpose          :通过互联网传递信息到贝壳物联，实现图形化的数据显示/Nodemcu transmits information to BIGIOT through the Internet, and realizes graphic data display. 
************************************#********************/
/********************###库文件声明###********************/
#include <ESP8266WiFi.h>
#include <aJSON.h>


//=============  此处必须修改============
String DEVICEID = "***"; // 你的设备ID========
String APIKEY = "***"; // 设备密码============
String INPUTID = "***"; //接口ID==============

//=======================================

unsigned long lastCheckStatusTime = 0; //记录上次报到时间
unsigned long lastUpdateTime = 0;//记录上次上传数据时间
const unsigned long postingInterval = 40000; // 每隔40秒向服务器报到一次
const unsigned long updateInterval = 5000; // 数据上传间隔时间5秒
unsigned long checkoutTime = 0;//登出时间

//=============wifi配置初始化===========
const char* ssid     = "***";//无线名称
const char* password = "***";//无线密码

const char* host = "www.bigiot.net";//通讯地址
const int httpPort = 8181;//TCP协议接口
//=======================================



void setup() {
  Serial.begin(115200);
  delay(1000);
  
  WiFi.begin(ssid, password);//这里就可以连接上网络
}

WiFiClient client;

void loop(){
  //============wifi连接测试================
    while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
//=======================================
 //==============使用TCP协议==============
  if (!client.connected()) {
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      delay(5000);
      return;
    }
  }
//=======================================

//================ 每一定时间查询一次设备在线状态，同时替代心跳=======================
if (millis() - lastCheckStatusTime > postingInterval) {
    checkStatus();
  }
  //checkout 50ms 后 checkin
  if ( checkoutTime != 0 && millis() - checkoutTime > 50 ) {
    checkIn();
    checkoutTime = 0;
  }
   //=======================================
 if (millis() - lastUpdateTime > updateInterval) {
    float val;//定义变量

    val=110;
    update1(DEVICEID, INPUTID, val);

    lastUpdateTime = millis();
  }
  //=================读取每个服务器的回应并打印在窗口======================
  if (client.available()) {
    String inputString = client.readStringUntil('\n');  //检测json数据是否完整
    inputString.trim();
    Serial.println(inputString);
    int len = inputString.length()+1;
    
    if(inputString.startsWith("{") && inputString.endsWith("}")){//净化json数据
      char jsonString[len];
      inputString.toCharArray(jsonString,len);
      aJsonObject *msg = aJson.parse(jsonString);
      processMessage(msg);
      aJson.deleteItem(msg);          
    }
  }
  //=======================================
  }
  //=====================================================================================
//=========================处理来自贝壳物联来的数据=========================
  void processMessage(aJsonObject *msg){
  aJsonObject* method = aJson.getObjectItem(msg, "M");
  aJsonObject* content = aJson.getObjectItem(msg, "C");     
  aJsonObject* client_id = aJson.getObjectItem(msg, "ID");
  if (!method) {
    return;
  }
   String M = method->valuestring;
    if (M == "WELCOME TO BIGIOT") {
    checkOut();
    checkoutTime = millis();
    return;
  }
  }
    //=======================================
    //=============查询设备在线状态===========
    void checkStatus() {
      
   String msg =  "{\"M\":\"status\"}\n";
   client.print(msg);
  lastCheckStatusTime = millis();
}
  //=============设备登录===========
 
  void checkIn() {
    String msg = "{\"M\":\"checkin\",\"ID\":\"" + DEVICEID + "\",\"K\":\"" + APIKEY + "\"}\n";
    client.print(msg);
    lastCheckStatusTime = millis(); 
}
//=======================================
//=============强制设备下线，用消除设备掉线延时===========

void checkOut() {
   String msg = "{\"M\":\"checkout\",\"ID\":\"" + DEVICEID + "\",\"K\":\"" + APIKEY + "\"}\n";
    client.print(msg);
 
}
//=======================================

//上传一个接口数据
//{"M":"update","ID":"2","V":{"2":"120"}}\n

void update1(String did, String inputid, float value) {

   String msg1 ="{\"M\":\"update\",\"ID\":\""+did+"\",\"V\":{\""+inputid+"\":\""+value+"\"}}\n";
   
     client.print(msg1);
 
}
