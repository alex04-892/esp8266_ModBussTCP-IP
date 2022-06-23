 /*
  * ModBus bridge TCP for RTU
  * description: esp8266 is a TCP server, it converts the modbasTCP data packet to modbasRTU and sends it to uart.
  * author: Alex
  * date: 03.06.2022
  * 
  */
#include <ESP8266WiFi.h>
#include "functions.h"

const char *ssid     = "Parus";
const char *password = "135_135_"; 
/**********нужно только для статического IP**********/
IPAddress ip(192,168,70,111);                       // static IP for esp8266
IPAddress gateway(192,168,70,1);
IPAddress subnet(255,255,255,0);
/***(end)**нужно только для статического IP**********/

int ModbusTCP_port = 502;
uint8_t buffTcp[130] = {0};
uint8_t buffRtu[130] = {0};
uint8_t buffUart[130] = {0};
uint8_t flag1 = 0,
        flag2 = 0;

uint8_t mesege[] = {0, 1, 0, 0, 0, 5, 1, 3, 2, 0x00, 0x03};\
uint8_t count1 = 0;

struct 
{
  uint16_t idTransaction;
  uint16_t idProtocol;
  uint16_t lengthЬessage;
  uint8_t deviceAddress;
  uint8_t functionCode;
  uint8_t quantityBytes;
  // далее идут значения регистров по 16 бит
}tcpHeader;



WiFiServer MBServer(ModbusTCP_port);

uint16_t crc16(uint8_t *adr_buffer, uint32_t byte_cnt);                               // расчета контрольной суммы CRC
void formatingTcpforRtu(uint8_t* buffTcp, uint8_t* buffRtu, uint16_t* countForBuffTcp);    // преобразование данныс из Tcp в Rtu
void sendBufRtu(uint8_t* buffRtu, uint16_t* countForBuffTcp);                              // отправка данных буфера Rtu в уарт

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println("");
  Serial.println("Starting program");
  
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet);               // set parameters for static IP
  
  delay(100) ;
  Serial.println(".");
  while (WiFi.status() != WL_CONNECTED)
  {
  delay(500);
  Serial.print(".");
  }
  
  MBServer.begin();
  Serial.println("Connected ");
  Serial.print("ESP8266 Slave Modbus TCP/IP ");
  Serial.print(WiFi.localIP());
  Serial.print(":");
  Serial.println(String(ModbusTCP_port));
  Serial.println("Modbus TCP/IP Online");
}


void loop()
{
  uint16_t countForBuffTcp = 0,
            countForBuffUart = 0;
  // Check if a client has connected     
  WiFiClient client = MBServer.available();     // Listen for incoming clients
  // wait untile client connected
  if (!client)                                  
  {
    return;
  }

  // пока клиент подключен
  while (client.connected())
  {

    
 
    
      // прием данных из уарт
      while (Serial.available())
      {
        buffUart[countForBuffUart] = Serial.read();
        countForBuffUart++;
        flag2 = 1;
      }

  
      // прием данных по Tcp в буфер
      while (client.available())
      {
        buffTcp[countForBuffTcp] = client.read();
        countForBuffTcp++;
        flag1 = 1;
      }
  
      // преобразование данныс из Tcp в Rtu 
      if (flag1)
      {
        formatingTcpforRtu(buffTcp, buffRtu, &countForBuffTcp);     
        flag1 = 0;

  tcpData.quantityBytes = 0x02;
  tcpData.reg1Hi = 0;
  //tcpData.reg1Lo = 11;
  
  client.write( tcpHeaderBuff, sizeof(tcpHeaderBuff) );
  Serial.write( tcpHeaderBuff, sizeof(tcpHeaderBuff) );
  client.write( &tcpData.quantityBytes, 1 );
  Serial.write( &tcpData.quantityBytes, 1 );
  client.write( &tcpData.reg1Hi, 1 );
  Serial.write( &tcpData.reg1Hi, 1 );
  
  client.write( &tcpData.reg1Lo, 1 );
  Serial.write( &tcpData.reg1Lo, 1 );
  (tcpData.reg1Lo)++;
  
      }
     
      sendBufRtu(buffRtu, &countForBuffTcp);          // отправка данных буфера Rtu в уарт

      if (flag2)
      {
  
        client.write( buffUart, countForBuffUart );
        countForBuffUart = 0;
        flag2 = 0;
      }
    
  }
 
  client.flush();                               // cleared buffer after send data
}
