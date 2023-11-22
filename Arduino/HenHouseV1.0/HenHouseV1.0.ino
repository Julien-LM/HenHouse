/*
  HenHouse V1.0
  LE MELLEC Julien 22/11/2023

  board:
    - RTC
    - (Aborted) SD Card
    - DC motor 12V
    - 2 limit switches
    - 1 Error LED
    - Photo resistor
    - ACD input, fuel gauge
    - 1 BP open/close door

  Fonctionnalités:
   - Boucle while, sortie si Vbat < 9,5V
   - Si BpPorte Down => OpenClose Door
   - Toutes les 5 minutes, faire un get de l'heure
     - Si 00:07:00 < h < 00:07:05 && porte fermée => ouvrir porte
     - Si 00:19:30 < h < 00:19:35 && porte ouverte => fermer porte
     - Récupérer la valeur de la photo résistance
     - Récupérer la valeur Vbat
     - (Aborted) Inscrire la valeur de la PhotoRes, Vbat et l'heure dans la carte SD
   - Si doorError = 1 => toogle ErrorLed

  Fonction OpenClose door:
   - Variables de la classe:
    - Door error
    - Actual door state
   - Arguments:
    - Command, can be (open, close, invert)

*/

#include <SPI.h>
#include <SD.h>
#include <Wire.h>//https://www.arduino.cc/en/reference/wire
#include <DS3231.h>//https://github.com/NorthernWidget/DS3231
#include <time.h>

// Constantes photo résistance
#define RES_DIV_PHOTO       47200
#define R_PHOTO_PIN         A1
#define V_ALIM              5

// Constantes limit switchs
#define LIMIT_SW_DOWN_PIN   2
#define LIMIT_SW_UP_PIN     3

// Constantes fuel gauge
#define FUELGAUGE_PIN       A2

// Constante BP nav
#define MULTI_BP_PIN        A0

// Constante Led de témoins d'erreur
#define ERROR_LED_PIN       7

// Constantes DC motor
#define BORNE_ENA           6 
#define BORNE_IN1           5
#define BORNE_IN2           4
#define MOTOR_SPEED         150 //between 0 & 255
#define TIMEOUT_MOVE        15

// Variables DC motor
char motorState = 'U'; // M=Monter, D=Descendre, 'I'=Inverser, 'O'=Ouvrir, 'F'=Fermer 'H'=Haut, 'B'=Bas, U=Unknown, 'E'=Erreur
short startActionTime = 0;
void manageDcMotor();

//RTC Variables
DS3231 Clock;
byte hour;
byte minute;
bool Century = false;
bool h12 ;
bool PM ;
void init_RTC();
void readRTC(bool log=false);

// Photo res var
int valeurDeLentreeAnalogique;          // Contiendra la valeur lue sur l'entrée analogique ; pour rappel, il s'agit d'une valeur 10 bits (0..1023)
float tensionDeLentreeAnalogique;       // Contiendra la valeur de calcul de tension, exprimée en volt, à partir de la "valeurDeLentreeAnalogique"
long valeurOhmiqueDeLaLDR;              // Contiendra la valeur ohmique calculée de la photorésistance, à partir de la "tensionDeLentreeAnalogique"
void getPhotoResVal();

// Timer var
byte varCompteur = 0; // La variable compteur

// Multi BP managing
void multiBpManaging();

void setup_pinMode();
void setup_timer();

void setup () {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  init_RTC();
  setup_pinMode();
  setup_timer();
}

void write_tempAndTime() {
  readRTC(false);
  Serial.print(hour, DEC); //24-hr
  Serial.print(":");
  Serial.println(minute, DEC);
  getPhotoResVal();
  Serial.print("Machine d'état :");
  Serial.println(motorState);
}



  
void loop () {
  manageDcMotor();
  multiBpManaging();
  if (bitRead (TIFR2, 0) == 1) {       // Flag TOV2 mis à 1 ?
    TCNT2 = 256 - 250;         // Rechargement du timer à 6
    bitSet (TIFR2, TOV2);      // Remise à zéro du flag TOV2 (voir texte)
    if (varCompteur++ > 125) { // Incrémentation et a atteint 125 ?
      varCompteur = 0;         // On recommence un nouveau cycle
      write_tempAndTime();
    }
  }
}
/*----------------------------------------------
---------------  Setup functions ---------------
------------------------------------------------*/
void setup_pinMode() {
  pinMode(ERROR_LED_PIN, OUTPUT);
  pinMode(LIMIT_SW_DOWN_PIN, INPUT_PULLUP);
  pinMode(LIMIT_SW_UP_PIN, INPUT_PULLUP);
  pinMode(MULTI_BP_PIN, INPUT_PULLUP);
  pinMode(BORNE_ENA, OUTPUT);
  pinMode(BORNE_IN1, OUTPUT);
  pinMode(BORNE_IN2, OUTPUT);
}
void setup_timer() {
  bitClear (TCCR2A, WGM20); // WGM20 = 0
  bitClear (TCCR2A, WGM21); // WGM21 = 0 
  TCCR2B = 0b00000111;      // Clock / 256 soit 16 micro-s et WGM22 = 0
  TIFR2 = 0b00000001;       // TOV2
  TCNT2 = 256 - 250;        // Chargement du timer à 6
}

/*----------------------------------------------
--------------  Multi BP input  ----------------
------------------------------------------------*/
void multiBpManaging() {
  if(analogRead(MULTI_BP_PIN) < 500) {
    Serial.println(analogRead(MULTI_BP_PIN));
    if(motorState != 'M' && motorState != 'D') {
      motorState = 'I';
    }
  }
}

/*----------------------------------------------
-------------  DC motor functions --------------
------------------------------------------------*/
void manageDcMotor(){
  if(motorState == 'O') { // Ouvrir
    if(digitalRead(LIMIT_SW_UP_PIN) != 0) {
      monter();
    }
  } else if(motorState == 'F') { // Fermer
    if(digitalRead(LIMIT_SW_DOWN_PIN) != 0) {
      descendre();
    }
  } else if(motorState == 'I') { // Inverser
    if(digitalRead(LIMIT_SW_UP_PIN) == 0) {
      descendre();
    } else if(digitalRead(LIMIT_SW_DOWN_PIN) == 0) {
      monter();
    } else {
      // Mouvement par défaut
      descendre();
    }
  } else if(motorState == 'M') { // Monter
    if(digitalRead(LIMIT_SW_UP_PIN) == 0) {
      stopMotor();
      motorState = 'H';
    } else if(getCurrentTimeMS() > (startActionTime+TIMEOUT_MOVE)) {
      stopMotor();
      motorState = 'E';
    }
  } else if(motorState == 'D') { // Descendre
    if(digitalRead(LIMIT_SW_DOWN_PIN) == 0) {
      stopMotor();
      motorState = 'B';
    } else if(getCurrentTimeMS() > (startActionTime+TIMEOUT_MOVE)) {
      stopMotor();
      motorState = 'E';
    }
  }
}

void monter() {
  configurerSensDeRotationPontA('M');
  changeVitesseMoteurPontA(MOTOR_SPEED);
  saveActionStartTime();
  motorState = 'M';
}

void descendre() {
  configurerSensDeRotationPontA('D');
  changeVitesseMoteurPontA(MOTOR_SPEED);
  saveActionStartTime();
  motorState = 'D';
}

void stopMotor() {
  changeVitesseMoteurPontA(0);
}

void configurerSensDeRotationPontA(char sensDeRotation) {

  if(sensDeRotation == 'D') {
    // Configuration du L298N en "marche avant", pour le moteur connecté au pont A. Selon sa table de vérité, il faut que :
    digitalWrite(BORNE_IN1, HIGH);                 // L'entrée IN1 doit être au niveau haut
    digitalWrite(BORNE_IN2, LOW);                  // L'entrée IN2 doit être au niveau bas    
  }
  
  if(sensDeRotation == 'M') {
    // Configuration du L298N en "marche arrière", pour le moteur câblé sur le pont A. Selon sa table de vérité, il faut que :
    digitalWrite(BORNE_IN1, LOW);                  // L'entrée IN1 doit être au niveau bas
    digitalWrite(BORNE_IN2, HIGH);                 // L'entrée IN2 doit être au niveau haut
  }
}

void changeVitesseMoteurPontA(int nouvelleVitesse) {
  
  // Génère un signal PWM permanent, de rapport cyclique égal à "nouvelleVitesse" (valeur comprise entre 0 et 255)
  analogWrite(BORNE_ENA, nouvelleVitesse);
}

/*----------------------------------------------
----------------  RTC functions ----------------
------------------------------------------------*/
void init_RTC() {
  Serial.println(F("RTC initilization"));
 	Wire.begin();
}

void saveActionStartTime() {
  startActionTime = Clock.getMinute()*60 + Clock.getSecond();
  Serial.print("Action started at ");
  Serial.print(startActionTime);
  Serial.println("s");
}

short getCurrentTimeMS() {
  return Clock.getMinute()*60 + Clock.getSecond();
}

void readRTC(bool log=false) {
  // Args: Log (True, False)
 	// Return Time, format: 
  byte year = Clock.getYear();
  byte month = Clock.getMonth(Century);
  byte day = Clock.getDate();
  hour = Clock.getHour(h12, PM);
  minute = Clock.getMinute();

  if(log) {
    Serial.print(year, DEC);
    Serial.print("-");
    Serial.print(month, DEC);
    Serial.print("-");
    Serial.print(day, DEC);
    Serial.print(" ");
    Serial.print(hour, DEC); //24-hr
    Serial.print(":");
    Serial.print(minute, DEC);
    Serial.print(":");
    Serial.println(Clock.getSecond(), DEC);
  }
}

/*----------------------------------------------
-------------  Photo res functions -------------
------------------------------------------------*/
void getPhotoResVal() {
  // Lecture de l'entrée analogique (pour rappel, cela retourne une valeur sur 10 bits, comprise entre 0 et 1023)
  valeurDeLentreeAnalogique = analogRead(R_PHOTO_PIN);

  // Détermination de la tension présente sur l'entrée analogique
  tensionDeLentreeAnalogique = float(V_ALIM * valeurDeLentreeAnalogique) / 1023;

  // Détermination de la valeur de la LDR, en fonction de la valeur de la résistance fixe, et de la tension précédemment trouvée
      // Nota : je reprends ici la formule de calcul développée un peu plus haut dans ce tuto, à savoir :
      //   → LDR = R*(Vcc – Vs) / Vs
      //   → d'où valLDR = R * (tensionAlimArduino – tensionEntreeAnalogique) / tensionEntreeAnalogique
  valeurOhmiqueDeLaLDR = RES_DIV_PHOTO * (V_ALIM - tensionDeLentreeAnalogique) / tensionDeLentreeAnalogique;

  // Puis on affiche ces valeurs sur le moniteur série de l'interface arduino
  Serial.print(F("Tension au milieu du pont diviseur : "));
  Serial.print(tensionDeLentreeAnalogique);
  Serial.println(F(" V"));

  Serial.print(F("Estimation de la valeur de la LDR (par calcul) : "));
  Serial.print(valeurOhmiqueDeLaLDR);
  Serial.println(F(" ohms"));

  Serial.println("");
}
