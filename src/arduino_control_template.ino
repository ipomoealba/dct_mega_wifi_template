#include "Arduino.h"

#include "ESP8266.h"
#include <Servo.h>

#include <SPI.h>
#include <MFRC522.h> // 引用程式庫

#define RST_PIN A0 // 讀卡機的重置腳位
#define SS_PIN 10  // 晶片選擇腳位

#define SSID "nccudct"
#define PASSWORD "nccudctproject"
// #define HOST_NAME "172.20.10.4"
// #define HOST_PORT (8000)
#define HOST_NAME "www.google.com"
#define HOST_PORT (80)
#define DEBUGPIN 12
#define DEBUG true

MFRC522 mfrc522(SS_PIN, RST_PIN); // 建立MFRC522物件

bool startHttpServer = false;
ESP8266 wifi(Serial1);

Servo myservo;

void setup(void)
{
    Serial.println("RFID reader is ready!");

    SPI.begin();
    mfrc522.PCD_Init(); // 初始化MFRC522讀卡機模組
    pinMode(DEBUGPIN, OUTPUT);
    myservo.attach(9, 500, 2400);
    myservo.write(90);
    Serial.println(F("setup begin"));
    Serial1.begin(115200);
    Serial.begin(9600);

    wifiConnector();
    getRequest();
    enableMUX();
    tcpSetup();

    Serial.print(F("setup end\r\n"));
}

void loop(void)
{
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
    {
        byte *id = mfrc522.uid.uidByte; // 取得卡片的UID
        byte idSize = mfrc522.uid.size; // 取得UID的長度

        Serial.print("PICC type: "); // 顯示卡片類型
        // 根據卡片回應的SAK值（mfrc522.uid.sak）判斷卡片類型
        MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
        Serial.println(mfrc522.PICC_GetTypeName(piccType));

        Serial.print("UID Size: "); // 顯示卡片的UID長度值
        Serial.println(idSize);

        for (byte i = 0; i < idSize; i++)
        { // 逐一顯示UID碼
            Serial.print("id[");
            Serial.print(i);
            Serial.print("]: ");
            Serial.println(id[i], HEX); // 以16進位顯示UID值
        }
        Serial.println();

        mfrc522.PICC_HaltA(); // 讓卡片進入停止模式
    }
    /* String request = httpReciver(); */
    /* if (request.length() > 0) */
    /* { */
    /* Serial.println("\r\n=====    request    ====="); */
    /* Serial.println(request); */
    /* Serial.println("=====  request end  ====="); */
    /* } */
    /* if (request.indexOf("/LED=ON") != -1) */
    /* { */
    /* digitalWrite(DEBUGPIN, HIGH); */
    /* } */
    /* else if (request.indexOf("/LED=OFF") != -1) */
    /* { */
    /* digitalWrite(DEBUGPIN, LOW); */
    /* } */
    /* for(int i = 0; i <= 180; i+=1){ */
    /* myservo.write(i); // 使用write，傳入角度，從0度轉到180度 */
    /* delay(20); */
    /* } */
    /* for(int i = 180; i >= 0; i-=1){ */
    /* myservo.write(i);// 使用write，傳入角度，從180度轉到0度 */
    /* delay(20); */
    /* } */
}

void lightLed(int pin, int level)
{
    digitalWrite(pin, level);
}

void wifiConnector(void)
{

    if (wifi.kick())
    {
        Serial.println(F("ESP8266 Live Check ... OK"));
    }
    else
    {
        Serial.println(F("ESP8266 Live Check ... Error"));
    }

    if (wifi.restart())
    {
        Serial.println(F("ESP8266 Restart ... Ok"));
    }
    else
    {
        Serial.println(F("ESP8266 Restart ... Error"));
    }

    if (wifi.setOprToStationSoftAP())
    {
        Serial.print(F("Set Operation Mode = STATION + AP ... OK\r\n"));
    }
    else
    {
        Serial.print(F("Set Operation Mode = STATION + AP ... Error\r\n"));
    }

    if (wifi.joinAP(SSID, PASSWORD))
    {
        Serial.print(F("Join AP ... OK\r\n"));
        Serial.print(F("IP ... "));
        Serial.println(wifi.getLocalIP().c_str());
    }
    else
    {
        Serial.print(F("Join AP ... Error\r\n"));
    }
}

void enableMUX(void)
{
    if (wifi.enableMUX())
    {
        Serial.print(F("Setup Multiple Connection ... OK\r\n"));
    }
    else
    {
        Serial.print(F("Setup Multiple Connection ... Error\r\n"));
    }
}

void tcpSetup(void)
{
    if (wifi.startTCPServer(80))
    {
        Serial.print(F("Start Tcp Server ... OK\r\n"));
    }
    else
    {
        Serial.print(F("Start Tcp Server ... Error\r\n"));
    }

    if (wifi.setTCPServerTimeout(10))
    {
        Serial.print(F("Set Tcp Server Timout 10 Seconds ... OK\r\n"));
    }
    else
    {
        Serial.print(F("Set Tcp Server Timout ... Error\r\n"));
    }
}

String httpReciver(void)
{
    uint8_t buffer[128] = {0};
    uint8_t mux_id;
    uint32_t len = wifi.recv(&mux_id, buffer, sizeof(buffer), 100);
    if (len > 0)
    {
        Serial.print(F("Status:["));
        Serial.print(wifi.getIPStatus().c_str());
        Serial.println(F("]"));

        Serial.print(F("Received from :"));
        Serial.print(mux_id);
        Serial.print(F("]\r\n"));

        String request = (char *)buffer;

        uint8_t header[] = "HTTP/1.1 200 OK\r\n"
                           "Content-Length: 24\r\n"
                           "Server: ESP8266\r\n"
                           "Content-Type: text/html\r\n"
                           "Connection: keep-alive\r\n\r\n";

        uint8_t response[] = "<p>yes your majesty</p>";
        wifi.send(mux_id, header, sizeof(header));
        wifi.send(mux_id, response, sizeof(response));

        if (wifi.releaseTCP(mux_id))
        {
            Serial.print(F("Release tcp "));
            Serial.print(mux_id);
            Serial.println(F(" ... OK"));
        }
        else
        {
            Serial.print(F("Release tcp"));
            Serial.print(mux_id);
            Serial.println(F("... Error"));
        }

        Serial.print(F("Status:["));
        Serial.print(wifi.getIPStatus().c_str());
        Serial.println(F("]"));
        return request;
    }
    return "";
}

void getRequest(void)
{
    while (true)
    {
        if (wifi.disableMUX())
        {
            Serial.print("single ok\r\n");
            break;
        }
        else
        {
            Serial.print("single err\r\n");
            delay(1000);
        }
    }

    uint8_t buffer[1024] = {0};

    if (wifi.createTCP(HOST_NAME, HOST_PORT))
    {
        Serial.print("Create Tcp ... ok\r\n");
    }
    else
    {
        Serial.print("Create Tcp ... err\r\n");
    }
    const char *data = "GET /\r\n\r\n";
    // char *data = "GET /device_handshaker/?device_id=<Central> HTTP/1.1\r\n\r\n";
    wifi.send((const uint8_t *)data, strlen(data));

    uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);

    if (len > 0)
    {
        Serial.print("Received:[");
        for (uint32_t i = 0; i < len; i++)
        {
            Serial.print((char)buffer[i]);
        }
        Serial.print("]\r\n");
    }

    if (wifi.releaseTCP())
    {
        Serial.print("Release Tcp ... Ok\r\n");
    }
    else
    {
        Serial.print("Release Tcp ... Error\r\n");
    }
}
