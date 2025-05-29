# Monitoramento e Controle Inteligente do Consumo de Água Residencial com Alerta de Vazamentos via MQTT

Este projeto desenvolve um sistema inteligente para monitorar o consumo de água, detectar vazamentos e permitir o controle remoto da válvula de água via MQTT.

## Visão Geral

O sistema utiliza um **ESP8266 NodeMCU**, um **sensor de fluxo de água** e uma **válvula solenoide** para medir o consumo em tempo real. Os dados são exibidos em um **display LCD** e publicados em um broker **MQTT**. Em caso de consumo anormal ou vazamento, o sistema envia alertas via MQTT e pode fechar automaticamente a válvula.

## Funcionalidades

-   **Monitoramento em Tempo Real:** Leitura de fluxo e volume de água.
-   **Display Local:** Exibição de dados em LCD.
-   **Comunicação IoT:** Envio de dados de consumo e alertas via MQTT.
-   **Detecção de Vazamentos:** Identificação de consumo excessivo e contínuo.
-   **Controle Automático:** Fechamento da válvula em caso de vazamento.
-   **Controle Remoto:** Abertura/fechamento da válvula via comandos MQTT.

## Hardware Utilizado

-   **ESP8266 NodeMCU**
-   **Sensor de Fluxo de Água G 1/2 1-30 l/min (YF-S201)**
-   **Válvula Solenoide 12V (Normalmente Fechada)**
-   **Módulo Relé 1 Canal 5V**
-   **Display LCD 16x2 com Módulo I2C**
-   Fontes de Alimentação (5V e 12V), protoboard, jumpers.

Consulte o diagrama em `Hardware/esquema_eletronico.fzz` para detalhes das conexões.

## Software

O firmware do NodeMCU foi desenvolvido na Arduino IDE.

### Bibliotecas Necessárias:

-   `ESP8266WiFi.h`
-   `PubSubClient.h`
-   `LiquidCrystal_I2C.h`
-   `Wire.h`

Instale-as via "Gerenciar Bibliotecas..." no Arduino IDE.

### Configuração do Código (`MonitoramentoAguaIoT.ino`):

Ajuste as seguintes linhas com suas informações antes do upload:

```cpp
// --- Configurações de Rede e MQTT ---
const char* ssid = "SEU_WIFI_SSID";         
const char* password = "SUA_SENHA_WIFI";    

const char* mqtt_broker = "SEU_BROKER_MQTT_IP_OU_DOMINIO"; 
const int mqtt_port = 1883;                                
const char* mqtt_client_id = "NodeMCU_Monitor_Agua_SeuNome"; 
// const char* mqtt_user = "seu_usuario"; // Descomente se necessário
// const char* mqtt_pass = "sua_senha";   // Descomente se necessário

// --- Configurações do Display LCD ---
const int LCD_I2C_ADDRESS = 0x27; // Verifique o endereço do seu LCD