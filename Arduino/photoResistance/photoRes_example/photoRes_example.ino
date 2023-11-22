/*
   ______               _                  _///_ _           _                   _
  /   _  \             (_)                |  ___| |         | |                 (_)
  |  [_|  |__  ___  ___ _  ___  _ __      | |__ | | ___  ___| |_ _ __ ___  _ __  _  ___  _   _  ___
  |   ___/ _ \| __|| __| |/ _ \| '_ \_____|  __|| |/ _ \/  _|  _| '__/   \| '_ \| |/   \| | | |/ _ \
  |  |  | ( ) |__ ||__ | | ( ) | | | |____| |__ | |  __/| (_| |_| | | (_) | | | | | (_) | |_| |  __/
  \__|   \__,_|___||___|_|\___/|_| [_|    \____/|_|\___|\____\__\_|  \___/|_| |_|_|\__  |\__,_|\___|
                                                                                      | |
                                                                                      \_|
  Fichier :       prgPhotoresistance-1-LectureTensionLdrAvecArduino.ino
  
  Description :   Programme permettant d'afficher, sur le moniteur série de l'IDE arduino, la valeur ohmique
                  de la photorésistance branchée sur l'entrée A0 d'un Arduino Uno (via un pont diviseur de tension)
                  
  Auteur :        Jérôme TOMSKI (https://passionelectronique.fr/)
  Créé le :       09.05.2022

*/

#define valeurDeLaResistanceFixe                      47200  // Mettre ici la "vraie" valeur (mesurée) de la résistance fixe de 10 kohms que vous allez utiliser
#define brocheEntreePointMilieuPontDiviseurDeTension  A0    // L'entrée A0 de l'arduino uno sera utilisée pour lire la tension en point milieu du pont diviseur
#define tensionAlimentationMicrocontroleurArduino     5     // Ici, on utilise un Arduino Uno, donc un modèle équipé du µC ATmega328P, fonctionnant sous 5 volts

int valeurDeLentreeAnalogique;          // Contiendra la valeur lue sur l'entrée analogique ; pour rappel, il s'agit d'une valeur 10 bits (0..1023)
float tensionDeLentreeAnalogique;       // Contiendra la valeur de calcul de tension, exprimée en volt, à partir de la "valeurDeLentreeAnalogique"
long valeurOhmiqueDeLaLDR;              // Contiendra la valeur ohmique calculée de la photorésistance, à partir de la "tensionDeLentreeAnalogique"
                                        // et de la résistance fixe (constante nommée "valeurDeLaResistanceFixe", dans ce programme)

// ========================
// Initialisation programme
// ========================
void setup() {

  // Initialise la liaison série (arduino uno -> PC)
  Serial.begin(9600);
  Serial.println(F("================================================================================"));
  Serial.println(F("PHOTORESISTANCE / Programme exemple 1"));
  Serial.println(F("- Lecture de la tension en point milieu du pont diviseur"));
  Serial.println(F("- Détermination de la valeur ohmique de la LDR"));
  Serial.println(F("================================================================================"));
  Serial.println("");

}

// =================
// Boucle principale
// =================
void loop() {
  
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

  // Et on reboucle cette fonction loop, après 5 secondes de "pause"
  delay(1000);
  
}