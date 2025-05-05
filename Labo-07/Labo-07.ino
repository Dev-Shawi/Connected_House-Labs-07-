#include <LCD_I2C.h>
#include <HCSR04.h>
#include <U8g2lib.h>
#include "Alarm.h"
#include "PorteAutomatique.h"

// Configuration de l'écran MAX7219
#define CLK_PIN 30
#define DIN_PIN 34
#define CS_PIN  32
U8G2_MAX7219_8X8_F_4W_SW_SPI u8g2(
  U8G2_R0, CLK_PIN, DIN_PIN, CS_PIN, U8X8_PIN_NONE, U8X8_PIN_NONE
);

LCD_I2C lcd(0x27, 16, 2);
HCSR04 hc(6, 7);

// Variables partagées
float distance = 0.0;
unsigned long currentTime = 0;

// Objets des nouvelles classes
Alarm alarm(3, 4, 5, 2, &distance);
PorteAutomatique porte(8, 10, 9, 11, distance);

// Variables pour l'affichage spécial
unsigned long specialDisplayStartTime = 0;
bool isSpecialDisplayActive = false;
String specialDisplayType = "";

void setup() {
  Serial.begin(9600);

  // Initialisation LCD
  lcd.begin();
  lcd.backlight();
  lcd.print("ETD:2305204");
  lcd.setCursor(0, 1);
  lcd.print("Labo 7");

  // Configuration initiale de l'alarme
  alarm.setColourA(255, 0, 0);  // Rouge
  alarm.setColourB(0, 0, 255);   // Bleu
  alarm.setVariationTiming(500);
  alarm.setDistance(20);
  alarm.setTimeout(3000);

  // Configuration initiale de la porte
  porte.setAngleOuvert(90);
  porte.setAngleFerme(0);
  porte.setPasParTour(2048);
  porte.setDistanceOuverture(20);
  porte.setDistanceFermeture(30);

  // Initialisation de l'écran MAX7219
   u8g2.begin();
   u8g2.setContrast(200); // Contrôle la luminosité
   u8g2.setPowerSave(0);  // Désactive l'économie d'énergie
   u8g2.clearBuffer();
   u8g2.sendBuffer();

  delay(2000);
  lcd.clear();
}

void loop() {
  currentTime = millis();

  // Mise à jour de la distance
  distance = mesureDistanceTask(currentTime);

  // Mise à jour des objets
  porte.update();
  alarm.update();

  // Affichage LCD
  lcdTask(currentTime, distance, porte.getAngle());

  // Gestion série
  serialTask();

  // Affichage MAX7219
  max7219Task(currentTime);
}

float mesureDistanceTask(unsigned long ct) {
  static unsigned long lastTime = 0;
  const unsigned long rate = 50;
  static float lastDistance = 0.0;

  if (ct - lastTime < rate) return lastDistance;

  lastTime = ct;
  lastDistance = hc.dist();
  return lastDistance;
}

void lcdTask(unsigned long ct, float distance, float angle) {
  static unsigned long lastTime = 0;
  const unsigned long rate = 100;

  if (ct - lastTime < rate) return;

  lastTime = ct;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Dist: ");
  lcd.print(distance);
  lcd.print(" cm");

  lcd.setCursor(0, 1);
  lcd.print("");
  lcd.print(porte.getEtatTexte());
  lcd.print(" ");
  lcd.print(static_cast<int>(angle));
  lcd.print(" deg");
}

void max7219Task(unsigned long ct) {
  if(isSpecialDisplayActive) {
    u8g2.clearBuffer();
    
    if(specialDisplayType == "OK") {
      // ✔️ Version simplifiée pour 8x8
      u8g2.drawLine(2, 3, 4, 5);
      u8g2.drawLine(4, 5, 6, 1);
    }
    else if(specialDisplayType == "ERROR") {
      // ❌ Croix
      u8g2.drawLine(1, 1, 6, 6);
      u8g2.drawLine(1, 6, 6, 1);
    }
    else if(specialDisplayType == "INVALID") {
      // Cercle barré
      u8g2.drawCircle(3, 3, 2);
      u8g2.drawLine(1, 1, 5, 5);
    }
    
    u8g2.sendBuffer();
    
    if(ct - specialDisplayStartTime >= 1000) { // Réduire la durée
      isSpecialDisplayActive = false;
    }
  } else {
    u8g2.clearBuffer();
    u8g2.sendBuffer();
  }
}

void serialTask() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "gDist") {
      Serial.println(distance);
      activateSpecialDisplay("OK");
    }
    else if (command.startsWith("cfg;alm;")) {
      int newDist = command.substring(8).toInt();
      if (newDist > 0) {
        alarm.setDistance(newDist);
        Serial.print("Alarm distance set to ");
        Serial.println(newDist);
        activateSpecialDisplay("OK");
      } else {
        Serial.println("Invalid alarm distance");
        activateSpecialDisplay("ERROR");
      }
    }
    else if (command.startsWith("cfg;lim_inf;")) {
      int newLimit = command.substring(12).toInt();
      if (newLimit < porte.getAngleOuvert()) {
        porte.setDistanceFermeture(newLimit);
        Serial.print("Lower limit set to ");
        Serial.println(newLimit);
        activateSpecialDisplay("OK");
      } else {
        Serial.println("Error - Lower limit >= Upper limit");
        activateSpecialDisplay("INVALID");
      }
    }
    else if (command.startsWith("cfg;lim_sup;")) {
      int newLimit = command.substring(12).toInt();
      if (newLimit > porte.getAngleFerme()) {
        porte.setDistanceOuverture(newLimit);
        Serial.print("Upper limit set to ");
        Serial.println(newLimit);
        activateSpecialDisplay("OK");
      } else {
        Serial.println("Error - Upper limit <= Lower limit");
        activateSpecialDisplay("INVALID");
      }
    }
    else {
      Serial.println("Unknown command");
      activateSpecialDisplay("ERROR");
    }
  }
}

void activateSpecialDisplay(String type) {
  isSpecialDisplayActive = true;
  specialDisplayType = type;
  specialDisplayStartTime = millis();
  u8g2.setPowerSave(false);
}