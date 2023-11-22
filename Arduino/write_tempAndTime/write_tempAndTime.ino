#include <SPI.h>
#include <SD.h>
#include <Wire.h>//https://www.arduino.cc/en/reference/wire
#include <DS3231.h>//https://github.com/NorthernWidget/DS3231

DS3231 Clock;
File myFile;

#define valeurDeLaResistanceFixe                      47200   // Mettre ici la "vraie" valeur (mesurée) de la résistance fixe de 10 kohms que vous allez utiliser
#define brocheEntreePointMilieuPontDiviseurDeTension  A0      // L'entrée A0 de l'arduino uno sera utilisée pour lire la tension en point milieu du pont diviseur
#define tensionAlimentationMicrocontroleurArduino     5       // Ici, on utilise un Arduino Uno, donc un modèle équipé du µC ATmega328P, fonctionnant sous 5 volts
#define LedYellow                                     5       // Led clignotante du programme principal

#define LedToggle digitalWrite (LedYellow, !digitalRead(LedYellow))

// RTC var
bool Century 	= false;
bool h12 ;
bool PM ;
// SD card var
const String fileName = "testters.txt";

// Photo res var
int valeurDeLentreeAnalogique;          // Contiendra la valeur lue sur l'entrée analogique ; pour rappel, il s'agit d'une valeur 10 bits (0..1023)
float tensionDeLentreeAnalogique;       // Contiendra la valeur de calcul de tension, exprimée en volt, à partir de la "valeurDeLentreeAnalogique"
long valeurOhmiqueDeLaLDR;              // Contiendra la valeur ohmique calculée de la photorésistance, à partir de la "tensionDeLentreeAnalogique"

// Timer var
byte varCompteur = 0; // La variable compteur


void setup () {

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

 	Serial.println(F("Initialize System"));
 	Wire.begin();

  Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");



  // re-open the file for reading:
  myFile = SD.open(fileName);
  if (myFile) {
    //Serial.println(fileName+":");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening "+fileName);
  }

  pinMode (LedYellow, OUTPUT);
  bitClear (TCCR2A, WGM20); // WGM20 = 0
  bitClear (TCCR2A, WGM21); // WGM21 = 0 
  TCCR2B = 0b00000111;      // Clock / 256 soit 16 micro-s et WGM22 = 0
  TIFR2 = 0b00000001;       // TOV2
  TCNT2 = 256 - 250;        // Chargement du timer à 6
}

void write_tempAndTime() {
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open(fileName, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to "+fileName+"...");
    myFile.println("testing 1, 2, 3.");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening "+fileName);
  }
  readRTC();
  getPhotoResVal();
}

void readRTC( ) { /* function readRTC */
 	////Read Real Time Clock
 	Serial.print(Clock.getYear(), DEC);
 	Serial.print("-");
 	Serial.print(Clock.getMonth(Century), DEC);
 	Serial.print("-");
 	Serial.print(Clock.getDate(), DEC);
 	Serial.print(" ");
 	Serial.print(Clock.getHour(h12, PM), DEC); //24-hr
 	Serial.print(":");
 	Serial.print(Clock.getMinute(), DEC);
 	Serial.print(":");
 	Serial.println(Clock.getSecond(), DEC);
}

void getPhotoResVal() {
  // Lecture de l'entrée analogique (pour rappel, cela retourne une valeur sur 10 bits, comprise entre 0 et 1023)
  valeurDeLentreeAnalogique = analogRead(brocheEntreePointMilieuPontDiviseurDeTension);

  // Détermination de la tension présente sur l'entrée analogique
  tensionDeLentreeAnalogique = float(tensionAlimentationMicrocontroleurArduino * valeurDeLentreeAnalogique) / 1023;

  // Détermination de la valeur de la LDR, en fonction de la valeur de la résistance fixe, et de la tension précédemment trouvée
      // Nota : je reprends ici la formule de calcul développée un peu plus haut dans ce tuto, à savoir :
      //   → LDR = R*(Vcc – Vs) / Vs
      //   → d'où valLDR = R * (tensionAlimArduino – tensionEntreeAnalogique) / tensionEntreeAnalogique
  valeurOhmiqueDeLaLDR = valeurDeLaResistanceFixe * (tensionAlimentationMicrocontroleurArduino - tensionDeLentreeAnalogique) / tensionDeLentreeAnalogique;

  // Puis on affiche ces valeurs sur le moniteur série de l'interface arduino
  Serial.print(F("Tension au milieu du pont diviseur : "));
  Serial.print(tensionDeLentreeAnalogique);
  Serial.println(F(" V"));

  Serial.print(F("Estimation de la valeur de la LDR (par calcul) : "));
  Serial.print(valeurOhmiqueDeLaLDR);
  Serial.println(F(" ohms"));

  Serial.println("");

}
  
void loop () {
  if (bitRead (TIFR2, 0) == 1) {       // Flag TOV2 mis à 1 ?
    TCNT2 = 256 - 250;         // Rechargement du timer à 6
    bitSet (TIFR2, TOV2);      // Remise à zéro du flag TOV2 (voir texte)
    if (varCompteur++ > 125) { // Incrémentation et a atteint 125 ?
      varCompteur = 0;         // On recommence un nouveau cycle
      LedToggle;             // Inversion de la LED
      write_tempAndTime();
    }
  }
}