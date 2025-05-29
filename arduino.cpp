#include <ESP8266WiFi.h>          
#include <PubSubClient.h>         
#include <Wire.h>                 
#include <LiquidCrystal_I2C.h>    

// --- Configurações de Rede e MQTT ---
const char* ssid = "SEU_WIFI_SSID";         
const char* password = "SUA_SENHA_WIFI";    

const char* mqtt_broker = "SEU_BROKER_MQTT_IP_OU_DOMINIO"; 
const int mqtt_port = 1883;                                
const char* mqtt_client_id = "NodeMCU_Monitor_Agua_Celio";    
const char* mqtt_user = "";                                
const char* mqtt_pass = "";                                

// Tópicos MQTT
const char* mqtt_topic_data = "/casa/agua/dados";
const char* mqtt_topic_alert = "/casa/agua/alerta";       
const char* mqtt_topic_command = "/casa/agua/comando";     

// --- Pinos do Hardware ---
const int FLOW_SENSOR_PIN = D2;   // Sensor de fluxo de água
const int VALVE_CONTROL_PIN = D1; // Relé da válvula

// --- Configurações do Display LCD ---
const int LCD_I2C_ADDRESS = 0x27; 
const int LCD_COLS = 16;          
const int LCD_ROWS = 2;           

// --- Variáveis Globais ---
volatile long pulseCount = 0;             
float flowRateLitersPerMin = 0.0;         
float totalLiters = 0.0;           
unsigned long lastFlowMillis = 0; 

// Fator de calibração do sensor (pulsos por litro). Ajuste este valor.
const float PULSE_PER_LITER = 450.0; 

// --- Detecção de Vazamento ---
const float VAZAMENTO_THRESHOLD_LPM = 0.5; 
const unsigned long VAZAMENTO_DURATION_SECONDS = 300; 
unsigned long leakStartTime = 0;                  
bool valveOpen = true; // Estado inicial da válvula

// --- Objetos ---
WiFiClient espClient;          
PubSubClient client(espClient); 
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS); 

// --- Funções ---

void IRAM_ATTR pulseCounter() {
  pulseCount++; 
}

void setup_wifi() {
  delay(10); 
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password); 

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Conectando WiFi");
    lcd.setCursor(0, 1);
    lcd.print("...");
  }

  Serial.println("\nWiFi Conectado!"); 
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP()); 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Conectado!");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP()); 
  delay(2000); 
  lcd.clear();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida em [");
  Serial.print(topic);
  Serial.print("]: ");
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i]; 
  }
  Serial.println(message);

  if (String(topic) == mqtt_topic_command) {
    if (message == "abrir_valvula") {
      digitalWrite(VALVE_CONTROL_PIN, HIGH); // Liga o relé
      valveOpen = true; 
      Serial.println("Comando: Válvula Aberta!");
      publishMqttAlert("Válvula Aberta por Comando Remoto."); 
    } else if (message == "fechar_valvula") {
      digitalWrite(VALVE_CONTROL_PIN, LOW); // Desliga o relé
      valveOpen = false; 
      Serial.println("Comando: Válvula Fechada!");
      publishMqttAlert("Válvula Fechada por Comando Remoto."); 
    }
  }
}

void reconnect_mqtt() {
  while (!client.connected()) { 
    Serial.print("Tentando conexão MQTT...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("MQTT Conectando");
    lcd.setCursor(0, 1);
    lcd.print("...");
    
    if (client.connect(mqtt_client_id, mqtt_user, mqtt_pass)) {
      Serial.println("Conectado!"); 
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("MQTT Conectado!");
      delay(1000);
      lcd.clear();
      
      client.subscribe(mqtt_topic_command);
      Serial.print("Inscrito no tópico: ");
      Serial.println(mqtt_topic_command);
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tente novamente em 5 segundos");
      delay(5000); 
    }
  }
}

void publishMqttAlert(String message) {
  String payload = "{\"status\": \"ALERTA\", \"mensagem\": \"" + message + "\"}";
  client.publish(mqtt_topic_alert, payload.c_str()); 
  Serial.print("Alerta MQTT Publicado: ");
  Serial.println(payload);
}

void setup() {
  Serial.begin(115200); 

  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP); 
  pinMode(VALVE_CONTROL_PIN, OUTPUT);
  digitalWrite(VALVE_CONTROL_PIN, HIGH); // Garante que a válvula comece aberta
  valveOpen = true;

  Wire.begin();     
  lcd.init();       
  lcd.backlight();  
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");
  lcd.setCursor(0, 1);
  lcd.print("Monitor Agua");
  delay(2000);
  lcd.clear();

  setup_wifi();

  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback); 

  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING);

  lastFlowMillis = millis(); 
}

void loop() {
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop(); 
  
  unsigned long currentMillis = millis(); 
  
  if (currentMillis - lastFlowMillis >= 1000) { 
    detachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN));
    
    flowRateLitersPerMin = (pulseCount / PULSE_PER_LITER) * 60.0; 
    totalLiters += (pulseCount / PULSE_PER_LITER); 

    pulseCount = 0;
    
    attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING);

    lastFlowMillis = currentMillis;

    lcd.clear(); 
    lcd.setCursor(0, 0);
    lcd.print("Vazao: ");
    lcd.print(flowRateLitersPerMin, 2); 
    lcd.print(" L/min");
    lcd.setCursor(0, 1);
    lcd.print("Total: ");
    lcd.print(totalLiters, 2); 
    lcd.print(" L");

    String dataPayload = "{\"fluxo\":" + String(flowRateLitersPerMin, 2) + 
                         ", \"volume\":" + String(totalLiters, 2) + "}";
    client.publish(mqtt_topic_data, dataPayload.c_str()); 
    Serial.print("Dados MQTT Publicados: ");
    Serial.println(dataPayload);

    if (valveOpen) { 
      if (flowRateLitersPerMin > VAZAMENTO_THRESHOLD_LPM) {
        if (leakStartTime == 0) { 
          leakStartTime = currentMillis;
          Serial.println("Possível vazamento detectado. Iniciando contagem...");
        } else if ((currentMillis - leakStartTime) / 1000 >= VAZAMENTO_DURATION_SECONDS) {
          digitalWrite(VALVE_CONTROL_PIN, LOW); 
          valveOpen = false; 
          Serial.println("VAZAMENTO DETECTADO! Válvula fechada automaticamente.");
          publishMqttAlert("VAZAMENTO DETECTADO! Válvula fechada automaticamente."); 
          leakStartTime = 0; 
        }
      } else {
        leakStartTime = 0;
      }
    }
  }
  delay(10); 
}