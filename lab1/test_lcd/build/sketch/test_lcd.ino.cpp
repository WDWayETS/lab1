#include <Arduino.h>
#line 1 "C:\\TEMP\\lab1\\lab1\\test_lcd\\test_lcd.ino"
/*
 * test_lcd.ino
 * Utiliser la bibliothèque LiquidCrystal pour faire afficher
 * des messages/données sur l'afficheur LCD.
 * 
 * L'afficheur LCD pourrait être intégré dans le projet IoT
 * de ce cours.
 * 
 * GPA788 - ETS
 * T. Wong
 * 08-2019
 * 08-2020
 * 
 * Ce programme est inspéré de l'exemple "Hello World!" de la page:
 * https://www.arduino.cc/en/Tutorial/HelloWorld. La classe LiquidCrystal()
 * et ses fonctions membres sont listées sur la page: 
 * https://www.arduino.cc/en/Reference/LiquidCrystal
 * Le branchement électrique est présenté d'une façon plus évidente dans le
 * protocole de laboratoire du cours. Finalement, la technique de lecture
 * de la température interne de l'ATmega328P est donnée dans les notes
 * de cours.
 */
// Classe pour afficher des caractères à l'aide du LCD
#include <LiquidCrystal.h>
// Classe pour mesurer la température et l'humidité calculée
#include "src\dhtlib_gpa788.h"


/* ---------------------------------------------------------------
   Créer un objet (global) de type LiquidCrystal.
   Le constructeur utilisé est:
     LiquidCrystal(rs, enable, d4, d5, d6, d7)
     rs (register select): broche 12
     enable: broche 11
     d4: broche 5, d5: broche 4, d6: broche 3, d7: broche 2
   ---------------------------------------------------------------    
   Note: Les numéros de broches sont ceux du côté d'Arduino.   
         Brancher ces broches sur l'afficheur LCD selon le
         diagramme électrique donné sur le protocole de laboratoire.
   --------------------------------------------------------------- */
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
/* ---------------------------------------------------------------
   Créer un objet (global) de type ChipTemp.
   Le constructeur utilisé est:
     ChipTemp(float g, float d)
     g: gain de l'ajustement, d: décalage de l'ajustement
     
   Normalement le gain et le décalage sont des valeurs obtenues
   par une procédure de calibration. Dans ce laboratoire, on utilisera
   des constantes proposées par Albert van Daln, l'auteur du code
   original de ChipTemp.
   --------------------------------------------------------------- */
const float DECALAGE{316.0}; //332.70; //335.2; // Choisir la bonne...
const float GAIN{1.22}; //1.06154;              // Choisir la bonne...
dhtlib_gpa788 DHT;
const int DHT11_PIN{7};

/* ---------------------------------------------------------------
   Fonction d'initialisation obligatoire (exécutée 1 seule fois).
   --------------------------------------------------------------- */
#line 61 "C:\\TEMP\\lab1\\lab1\\test_lcd\\test_lcd.ino"
void setup();
#line 86 "C:\\TEMP\\lab1\\lab1\\test_lcd\\test_lcd.ino"
void loop();
#line 123 "C:\\TEMP\\lab1\\lab1\\test_lcd\\test_lcd.ino"
void waitUntil(uint32_t w);
#line 134 "C:\\TEMP\\lab1\\lab1\\test_lcd\\test_lcd.ino"
void welcome(LiquidCrystal &l);
#line 149 "C:\\TEMP\\lab1\\lab1\\test_lcd\\test_lcd.ino"
void showTemp(dhtlib_gpa788 &DHT, LiquidCrystal &l);
#line 61 "C:\\TEMP\\lab1\\lab1\\test_lcd\\test_lcd.ino"
void setup() {
  // Pour afficher la température sur le terminal série
  Serial.begin(9600);

  // Sur le VS Code, l'ouverture du port série prend du temps et on
  // peut perdre des caractères. Ce problème n'existe pas sur la carte
  // de l'Arduino.
  waitUntil(2000);

  // Régler la broche utilisée pour le capteur DHT11
  DHT.setConnectedPin(DHT11_PIN);
  
  // Pour l'afficheur LCD
  // 0) Il s'agir d'un afficheur 16 x 2
  lcd.begin(16, 2);
  // 1) Effacer l'affichage
  lcd.clear();

}

/* ---------------------------------------------------------------
   Fonction principale
   (Exécutée comme une fonction dans une boucle sans fin)
   --------------------------------------------------------------- */
const int16_t NB_MSG_COUNT{2};
void loop() {
  // Alterner entre le message de bienvenue et la température 
  // interne de l'ATmega328P.
  // messageCount: nombre de fois le contenu a été affiché
  // showingMessage: true -> bienvenue, false -> température
  static int16_t messageCount{0};
  static bool showingMessage{true};

  // Si le compteur est à zéro alors change le contenu
  if (messageCount == 0) {
    lcd.clear();
    if (showingMessage)        // était message de bienvenue? alors
      showTemp(DHT, lcd); // affiche la température interne.
    else
      welcome(lcd);
  }
  // Si le compteur est > NB_MSG_COUNT alors remettre le compteur
  // à zéro et change le contenu de l'afficheur.
  if (messageCount++ > NB_MSG_COUNT) {
    messageCount = 0;
    showingMessage = !showingMessage;
  }
  
  // Faire clignoter lentement l'afficheur
  lcd.display();
  waitUntil(2000);
  lcd.noDisplay();
  waitUntil(1000);  
}

/* ---------------------------------------------------------------
   Fonction pour créer un delai de w millisecondes
   
   La fonction delay() est utilisée dans bien des tutoriel pour
   créer un delai temporel. On peut aussi créer notre propre délai
   et utiliser une unité de temps à notre guise.
   --------------------------------------------------------------- */
void waitUntil(uint32_t w) {
  uint32_t t{millis()};
  // Attendre w millisecondes
  while (millis() < t + w) {}
}

/* ---------------------------------------------------------------
   Fonction pour afficher un message de bienvenue.
   ---------------------------------------------------------------
   Note: On aurait pu utiliser l'objet global lcd directement.
   --------------------------------------------------------------- */
void welcome(LiquidCrystal &l) {
  // D'abord le terminal série
  Serial.println(F("Bienvenue au GPA788 OC/IoT")); 
  // Ensuite l'afficheur LCD
  l.setCursor(0, 0);  // Cursuer à la 1ere colonne, 1ere ligne
  l.print("Bienvenue au");  // Afficher le texte...
  l.setCursor(0, 1);  // Curseur à la 1ere colonne, 2e ligne
  l.print("GPA788 OC/IoT"); // Afficher le texte...}
}

/* ---------------------------------------------------------------
   Fonction pour afficher la température interne de l'ATmega328P.
   ---------------------------------------------------------------
   Note: On aurait pu utiliser l'objet global chipTemp directement.
   --------------------------------------------------------------- */
void showTemp(dhtlib_gpa788 &DHT, LiquidCrystal &l) {
  DHTLIB_ErrorCode chk = DHT.read11();

  if (chk == DHTLIB_ErrorCode::DHTLIB_OK) {
    Serial.print(F("Temperature = "));
    Serial.println(DHT.getTemperature());
    Serial.print(F("Humidity = "));
    Serial.println(DHT.getHumidity());

    // Ensuite l'afficheur LCD
    l.setCursor(0,0); l.print("Temp.: ");
    l.print(DHT.getTemperature(), 0); l.print((char)223); l.print("C");
    l.setCursor(0,1); l.print("Humidity.: ");
    l.print(DHT.getHumidity(), 0); l.print("%"); 
  }
  else {
    Serial.println(F("DHT11: Erreur"));
    Serial.print(F("DHT11:"));
    Serial.println(static_cast<int>(chk));

    // Ensuite l'afficheur LCD
    l.setCursor(0,0); l.print("DHT11: Erreur");
    l.setCursor(0,1); l.print("DHT11: code ");
    l.print(static_cast<double>(chk), 0); 
  }
    
}

