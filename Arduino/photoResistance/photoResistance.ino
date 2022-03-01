/*
 * Code d'exemple pour une photorésistance.
 */

// Fonction setup(), appelée au démarrage de la carte Arduino
void setup() {

  // Initialise la communication avec le PC
  Serial.begin(9600);
}

// Fonction loop(), appelée continuellement en boucle tant que la carte Arduino est alimentée
void loop() {

  // Mesure la tension sur la broche A7
  int valeur = analogRead(A7);

  // Envoi la mesure au PC pour affichage et attends 250ms
  Serial.println(valeur);
  Serial.println("gt");
  delay(250);
}
