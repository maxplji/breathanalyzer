
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

float bazoweADC;


float interpolacja(float x, float x0, float x1,
                   float y0, float y1) {
  return y0 + (y1 - y0) * (x - x0) / (x1 - x0);
}


float obliczStosunekRezystancji(float napiecieAO, float napiecieBazowe) {
    float Vc = 4.8;

    if (napiecieAO <= 0 || napiecieBazowe <= 0) {
        return NAN;
    }

    float Rs = Vc / napiecieAO - 1;
    float Rbazowe = Vc / napiecieBazowe - 1;

    return Rs / Rbazowe;
}

float korekcjaDHT11 (float Rs, float T, float H){
  float F = wspolczynnikKorekcji(T, H);

  if (isnan(F) || F<=0){
    return NAN;
  }
return Rs / F;
}
int k;
float wspolczynnikKorekcji(float T,  float H){

const int LICZBA_T = 7;
const int LICZBA_H = 4;

// Temperatura w stopniach Celsjusza
const float temperatury[LICZBA_T] = {
  -10, 0, 10, 20, 30, 40, 50
};

// Wilgotnosc wzgledna w %
const float wilgotnosci[LICZBA_H] = {
  20, 40, 55, 85
};

// Wspolczynniki Rs/Rso
// Wiersze: wilgotnosc
// Kolumny: temperatura

const float korekcja[LICZBA_H][LICZBA_T] = {
  // -10   0    10    20    30    40    50 C

  {1.83, 1.62, 1.46, 1.32, 1.20, 1.12, 1.05}, // 20% RH
  {1.72, 1.52, 1.32, 1.16, 1.02, 0.95, 0.93}, // 40% RH
  {1.62, 1.38, 1.18, 1.00, 0.91, 0.82, 0.79}, // 55% RH
  {1.56, 1.30, 1.10, 0.93, 0.83, 0.77, 0.73}  // 85% RH
};

int indeksT = -1;

int indeksH = -1;

for(k=0; k < LICZBA_T -1; k++){
 if (T >= temperatury[k] &&
        T <= temperatury[k + 1]) {

        indeksT = k;
        break;
    }

}

float h_diff;

for(k = 0; k< LICZBA_H -1; k++){
 if (H >= wilgotnosci[k] &&
        H <= wilgotnosci[k + 1]) {

        indeksH = k;
        break;
    }
}

if (indeksT == -1 || indeksH == -1) {
  return NAN;
}

float F00 = korekcja[indeksH][indeksT];
float F01 = korekcja[indeksH][indeksT +1];

float F10 = korekcja[indeksH+1][indeksT];
float f11 = korekcja[indeksH+1][indeksT+1];

float T0 = temperatury[indeksT];
float T1 = temperatury[indeksT + 1];

float H0 = wilgotnosci[indeksH];
float H1 = wilgotnosci[indeksH + 1];

float Fa = interpolacja(T, T0, T1, F00, F01);


float Fb = interpolacja(T, T0, T1, F10, F11);


float F = interpolacja(H, H0, H1, Fa, Fb);

return F;



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
  bazoweADC = wyznaczBazoweADC();

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
