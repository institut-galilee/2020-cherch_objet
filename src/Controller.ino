#include <ESP8266WiFi.h>
#include <espnow.h>

//etat des Objets
bool etat1 = false ; 
bool etat2 = false ;
String etatBuzzer1 = "off";
String etatBuzzer2 = "off";

// Callback when data is sent : verifier si on a bien envoye le msg
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus);

// Callback when data is received : recevoir msg
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len); 

const int bouton1 = D5 ;
const int bouton2 = D6 ;

char * ssid = "MON PROJET IoT" ;
char * password = "sapasgadew" ;

// MAC Address commande 
uint8_t Obj1Addresse[] = {0xD8, 0xBF, 0xC0, 0x14, 0xBC, 0x24};
uint8_t Obj2Addresse[] = {0xD8, 0xBF, 0xC0, 0x14, 0x09, 0x26};


// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;


// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;


IPAddress ip(192,168,4,20);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);
  
void setup() 
{
  Serial.begin(115200);
  pinMode (bouton1, INPUT_PULLUP) ;
  pinMode (bouton2, INPUT_PULLUP) ;
  
  // Set device as a Wi-Fi soft ap Station
  WiFi.mode(WIFI_AP_STA);

  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(ssid,password);

  //configurer esp now

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // ESP-NOW mode
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  //Objet 1
  esp_now_add_peer(Obj1Addresse, ESP_NOW_ROLE_COMBO, 1, NULL, 0) ;
  //Objet 2
  esp_now_add_peer(Obj2Addresse, ESP_NOW_ROLE_COMBO, 1, NULL, 0) ;

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  server.begin();

}

   bool test = false ;
   
void loop() 
{
   WiFiClient client = server.available();   // Listen for incoming clients

  if (client) 
  {                             
    Serial.println("New Client.");          
    String currentLine = ""; 
    currentTime = millis();
    previousTime = currentTime;
    
    while (client.connected() && currentTime - previousTime <= timeoutTime) 
    { 
      currentTime = millis();         
      if (client.available()) {             
        char c = client.read();             
        header += c;
       
        if (c == '\n') 
        {                    
          if (currentLine.length() == 0) {
            
            if (header.indexOf("GET /1/on") >= 0) 
            {
              Serial.println("Obj1 ON");
              etatBuzzer1 = "on";
              etat1 = true ;
              esp_now_send(Obj1Addresse, (uint8_t *) &etat1, sizeof(etat1));
            } 
            
            else if (header.indexOf("GET /1/off") >= 0) 
            {
              Serial.println("Obj1 OFF");
              etatBuzzer1 = "off";
              etat1 = false ;
              esp_now_send(Obj1Addresse, (uint8_t *) &etat1, sizeof(etat1));
            } 
            
            else if (header.indexOf("GET /2/on") >= 0) 
            {
              Serial.println("Obj2 ON");
              etatBuzzer2 = "on";
              etat2 = true ;
              esp_now_send(Obj2Addresse, (uint8_t *) &etat2, sizeof(etat2));
            } 
            
            else if (header.indexOf("GET /2/off") >= 0) 
            {
              Serial.println("Obj2 OFF");
              etatBuzzer2 = "off";
              etat2 = false; 
              esp_now_send(Obj2Addresse, (uint8_t *) &etat2, sizeof(etat2));
            }
            
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta http-equiv=\"refresh\" content=\"1; url=/\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            client.println("<body><h1>Controle des objets</h1>");
            
            if(etat1)
              etatBuzzer1 = "on" ;
            
            else
              etatBuzzer1 = "off" ;

            if(etat2)
              etatBuzzer2 = "on" ;
            
            else
              etatBuzzer2 = "off" ;

            client.println("<p>Objet 1 - Etat " + etatBuzzer1 + "</p>");

            if (etatBuzzer1=="off") 
              client.println("<p><a href=\"/1/on\"><button class=\"button\">ALLUMER</button></a></p>");
            
            else 
              client.println("<p><a href=\"/1/off\"><button class=\"button button2\">ETEINDRE</button></a></p>");
               
            client.println("<p>Objet 2 - Etat " + etatBuzzer2 + "</p>");
            
            if (etatBuzzer2=="off") 
              client.println("<p><a href=\"/2/on\"><button class=\"button\">ALLUMER</button></a></p>");
            
            else 
              client.println("<p><a href=\"/2/off\"><button class=\"button button2\">ETEINDRE</button></a></p>");
            
            client.println("</body></html>");
            
            client.println();
            break;
          } 
          
          else 
            currentLine = "";
        } 
        
        else if (c != '\r') 
          currentLine += c;      // add it to the end of the currentLine
      }
    }
    
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
  }
  
  int bouton1_etat = digitalRead(bouton1); 
    
  if (bouton1_etat == LOW) //si on appuie sur le bouton 
  {
    etat1 = !etat1 ; //changement d'etat
    delay(1000) ;  
    esp_now_send(Obj1Addresse, (uint8_t *) &etat1, sizeof(etat1));
  }

  int bouton2_etat = digitalRead(bouton2); 
  
  if (bouton2_etat == LOW) //si on appuie sur le bouton 
  {
    etat2 = !etat2 ; //changement d'etat
    delay(1000) ;
    esp_now_send(Obj2Addresse, (uint8_t *) &etat2, sizeof(etat2));
  }
}

// verifier si on a bien envoye les donnees
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
  if (sendStatus == 0)
    Serial.println("Delivery success");

  else
    Serial.println("Delivery fail");
}

//recevoir donnee
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) 
{
  bool verife = true ;
  
  for(int i = 0 ; i < 6 ; i++ )
  {
    if(mac[i] != Obj1Addresse[i] )
    {
      verife = false ;  
      break;
    }
  }

  if(verife)
     memcpy(&etat1, incomingData, sizeof(etat1));
  
  else
     memcpy(&etat2, incomingData, sizeof(etat2));
}
