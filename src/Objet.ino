#include <ESP8266WiFi.h>
#include <espnow.h>

// Replace with your network credentials
char * ssid = "MON PROJET IoT" ;
char * password = "sapasgadew" ;

//etat
bool etat = false;
String etatBuzzer = "off";

// MAC Address commande 
uint8_t CommandeAddress[] = {0x10, 0x52, 0x1C, 0x02, 0x4C, 0xAC};

//port
const int buzzer = D5 ;
const int bouton = D3 ;
const int led = D4 ;

//configurer ip static
IPAddress ip(192,168,4,200);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);

// Web Server port 80
WiFiServer server(80);

String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  
  pinMode (buzzer, OUTPUT) ;
  pinMode (led, OUTPUT) ;

  //Configure wifi
  WiFi.mode(WIFI_STA);
  
  Serial.print("Connection a :");
  Serial.println(ssid);
 
  WiFi.config(ip,gateway,subnet);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Wifi connecte");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  
  //configuration espnow
  if (esp_now_init() != 0) {
    Serial.println("Erreur ESP-NOW");
    return;
  }

  // ESP-NOW mode
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_add_peer(CommandeAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  
  //fonction envoyer et recevoir
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  
  server.begin();
}

void loop(){
  digitalWrite(led, HIGH);
  
  WiFiClient client = server.available(); 

  if (client) 
  {                            
    Serial.println("New Client."); 
    String currentLine = "";       
    currentTime = millis();
    previousTime = currentTime;
    
    while (client.connected() && currentTime - previousTime <= timeoutTime) 
    { 
      currentTime = millis();         
      
      if (client.available()) 
      {    
        char c = client.read();
        header += c;
        
        if (c == '\n') 
        {                    
          if (currentLine.length() == 0) 
          {
            if (header.indexOf("GET /1/on") >= 0) 
            {
              Serial.println("BUZZER on");
              etat = true ;
              tone(buzzer,10);
              esp_now_send(CommandeAddress, (uint8_t *) &etat, sizeof(etat));
            } 
            
            else if (header.indexOf("GET /1/off") >= 0) 
            {
              Serial.println("BUZZER off");
              etat = false ;
              noTone(buzzer);
              esp_now_send(CommandeAddress, (uint8_t *) &etat, sizeof(etat));
            } 
                        
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta http-equiv=\"refresh\" content=\"1; url=/\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            client.println("<body><h1>Objet 2</h1>");
            
            if(etat)
              etatBuzzer = "on" ;
            
            else
              etatBuzzer = "off" ;
               
            client.println("<p>Etat sonnerie - " + etatBuzzer + "</p>");
            
            if (etatBuzzer=="off") 
              client.println("<p><a href=\"/1/on\"><button class=\"button\">ALLUMER</button></a></p>");
            
            else 
              client.println("<p><a href=\"/1/off\"><button class=\"button button2\">ETEINDRE</button></a></p>");
      
            client.println("</body></html>");
            
            client.println();
            break;
          } 
          
          else 
            currentLine = "";
        } 
        
        else if (c != '\r') 
          currentLine += c;      
      }
    }
    
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
  }

  int bouton_etat = digitalRead(bouton); // read new state
   
  if (bouton_etat == LOW) //si on appuie sur le bouton 
  {
    etat = !etat ; //changement d'etat
    delay(1000) ;  
    sonner(etat);
    esp_now_send(CommandeAddress, (uint8_t *) &etat, sizeof(etat));
  }
}


void sonner(bool action)
{  
  if(action)
  {
    Serial.println("Bouton ON");
    tone(buzzer,10);
  }

  else
  {
    Serial.println("Bouton OFF");
    noTone(buzzer);
  }
}

//verifier si on a bien envoye les donnees
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) 
{
  if (sendStatus == 0)
    Serial.println("Delivery success");
  
  else
    Serial.println("Delivery fail");
}

// fonction recevoir donnee
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) 
{
  memcpy(&etat, incomingData, sizeof(etat));
  Serial.print("Recu: ");
  Serial.println(etat);
  sonner(etat);
}
