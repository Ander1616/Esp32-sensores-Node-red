#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHT_PIN 23
#define DHT_TIPO DHT11

#define RELAY_PIN_1 2
#define RELAY_PIN_2 15
#define RELAY_PIN_3 4

DHT dht(DHT_PIN, DHT_TIPO);

//const char* ssid = "Omni_lite611D20";
//const char* password = "major8954";
//const char* ssid = "CONEXION-RIANO";
//const char* password = "RP38203290@";
const char* ssid = "ARP";
const char* password = "12345678";

const char* mqtt_server = "broker.mqttdashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);

char msg[16];

bool relayState1 = LOW;
bool relayState2 = LOW;
bool relayState3 = LOW;

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);
  pinMode(RELAY_PIN_3, OUTPUT);

  dht.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  delay(500);
  float temperatura = dht.readTemperature();
  float humedad = dht.readHumidity();

  if (isnan(temperatura) || isnan(humedad)) {
    Serial.println("Error al leer la temperatura y la humedad");
    return;
  }

  Serial.println(temperatura);
  Serial.println(humedad);

  snprintf(msg, 16, "%.2f,%.2f", temperatura, humedad);
  client.publish("canal", msg);
  delay(10);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32")) {
      Serial.println("connected!");
      // Suscribirse a los temas relevantes en Node-RED
      client.subscribe("control/relay1");
      client.subscribe("control/relay2");
      client.subscribe("control/relay3");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setup_wifi() {
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String message = String((char*)payload);

  // Control de relés
  if (strcmp(topic, "control/relay1") == 0) {
    if (message.equals("ON")) {
      relayState1 = !relayState1;  // Invierte el estado actual
      digitalWrite(RELAY_PIN_1, relayState1);
    }
  } else if (strcmp(topic, "control/relay2") == 0) {
    if (message.equals("ON")) {
      relayState2 = !relayState2;  // Invierte el estado actual
      digitalWrite(RELAY_PIN_2, relayState2);
    }
  } else if (strcmp(topic, "control/relay3") == 0) {
    if (message.equals("ON")) {
      relayState3 = !relayState3;  // Invierte el estado actual
      digitalWrite(RELAY_PIN_3, relayState3);
    }
  }
}
