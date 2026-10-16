
#include<DHT.h>
#include<Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DHT_PIN 4
#define DHT_TYPE DHT11
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define PIN_A0 34

DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
int i = 0;

void setup() {
  Serial.begin(115200);
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
    oled.display();
    delay(2000);
    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.println("Wyniki: ");
    oled.println();
    oled.print("Temperatura: ");
    oled.print(temperatura, 1);
    oled.println(" C");
    oled.print("Wilgotnosc: ");
    oled.print(wilgotnosc, 2);
    oled.println(" %");
    oled.print("Wartosc ADC: ");
    oled.print(alkohol);
    oled.display();
    delay(2000);
  }
  delay(1000);
}
