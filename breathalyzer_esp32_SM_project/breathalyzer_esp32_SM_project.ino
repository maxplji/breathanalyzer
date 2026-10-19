
#include<DHT.h>
#include<Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include<WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include<time.h>
#include "secrets.h"



WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const unsigned long botDelay = 1000;
unsigned long ostatniTelegram = 0;
unsigned long ostatniPomiar = 0;


#define DHT_PIN 4
#define DHT_TYPE DHT11
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define PIN_A0 34
 

DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
int i = 0;
int j = 0;


void handleNewMessages(int numNewMessages) {

  for (int j = 0; j < numNewMessages; j++) {

    String chat_id = bot.messages[j].chat_id;
    String text = bot.messages[j].text;


if (chat_id != CHAT_ID) {
  continue;
}



    if (text == "/pomiar") {

      float temperatura = dht.readTemperature();
      float wilgotnosc = dht.readHumidity();
      int alkohol = analogRead(PIN_A0);

      if (isnan(temperatura) || isnan(wilgotnosc)) {
        bot.sendMessage(chat_id, "Blad DHT11", "");
        continue;
      }

      String wiadomosc = "Wyniki pomiaru:\n";

      wiadomosc += "Temperatura: ";
      wiadomosc += String(temperatura, 1);
      wiadomosc += " C\n";

      wiadomosc += "Wilgotnosc: ";
      wiadomosc += String(wilgotnosc, 1);
      wiadomosc += " %\n";

      wiadomosc += "MQ-3 ADC: ";
      wiadomosc += String(alkohol);

      bool sukces =bot.sendMessage(chat_id, wiadomosc, "");

      if (sukces){
        Serial.println("Wyslano pomiar do telegrama");
      }

      else {
        Serial.println("Blad wysylania pomiaru");
      }
    }
  }
}

float wyznaczBazoweADC() {
  float sr=0.0;
for (j = 0; j<30; j++){
  sr+= analogRead(PIN_A0);
  delay(2);
}
 
return (sr/30.0);
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_A0, ADC_0db);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());

  configTime(0, 0, "pool.ntp.org", "time.google.com");

struct tm timeinfo;

if (getLocalTime(&timeinfo, 15000)) {
  Serial.println("Czas zsynchronizowany");
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
} else {
  Serial.println("Blad synchronizacji czasu");
}

  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println(F("SSD1306 nie dziala"));
    while (true);
  }
  dht.begin();
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(0, 10);
  oled.println("Poczekaj na przygotowanie się urządzenia...");
  oled.display();
  delay(20000);
  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.println("Gotowe!");
  oled.display();
  delay(2000);
}

void loop() {

unsigned long teraz = millis();


if (teraz - ostatniTelegram >= botDelay) {

  ostatniTelegram = teraz;

  int numNewMessages =
    bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages > 0) {

    handleNewMessages(numNewMessages);

    numNewMessages =
      bot.getUpdates(bot.last_message_received + 1);
  }
}





if(teraz - ostatniPomiar >= 2000){
  ostatniPomiar = teraz;

  float temperatura = dht.readTemperature();
  float wilgotnosc = dht.readHumidity();
  int alkohol = analogRead(PIN_A0);
  if(isnan(temperatura) || isnan(wilgotnosc)){
    Serial.println("DHT11 nie dziala");
  }
  else {
    
i++;

oled.clearDisplay();
oled.setCursor(0, 0);

oled.print("Pomiar numer ");
oled.println(i);

oled.println();

oled.print("Temperatura: ");
oled.print(temperatura, 1);
oled.println(" C");

oled.print("Wilgotnosc: ");
oled.print(wilgotnosc, 1);
oled.println(" %");

oled.print("Wartosc ADC: ");
oled.println(alkohol);

oled.display();

  }
  


}



}
