#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Remplacez par vos identifiants WiFi
const char* ssid = "MYSSID";
const char* password = "MYPASS";
// Pins pour les LEDs
const int bluePin = 13; // D5
const int redPin = 12; // D6
const int greenPin = 14; // D7

ESP8266WebServer server(80);
ESP8266WebServer serverSecure(443);

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>Controle des LEDs</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "html, body { height: auto; margin: 0; padding: 0; }"; // Ajout de cette ligne pour ajuster la hauteur
  html += "body { font-family: Arial, sans-serif; background-color: #f0f0f0; text-align: center; padding: 20px; }";
  html += "h1 { color: #333; }";
  html += "form { background: #fff; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); display: inline-block; }";
  html += "input[type='submit'] { background-color: #4CAF50; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; }";
  html += "input[type='submit']:hover { background-color: #45a049; }";
  html += "@media (max-width: 600px) { form { width: 100%; box-sizing: border-box; } }";
  html += "</style>";
  html += "<script src='https://cdnjs.cloudflare.com/ajax/libs/jscolor/2.0.4/jscolor.min.js'></script>";
  html += "<script>";
  html += "document.addEventListener('DOMContentLoaded', function() {";
  html += "  var colorPicker = document.querySelector('.jscolor');";
  html += "  colorPicker.addEventListener('touchstart', function(e) {";
  html += "    document.body.style.overflow = 'hidden';";
  html += "  });";
  html += "  colorPicker.addEventListener('touchend', function(e) {";
  html += "    document.body.style.overflow = '';";
  html += "  });";
  html += "});";
  html += "</script>";
  html += "</head><body>";
  html += "<h1>Contrôle des LEDs</h1>";
  html += "<form action='/setColor' method='GET'>";
  html += "Choisissez une couleur: <input class='jscolor' name='color' value='ffffff'><br><br>";
  html += "<input type='submit' value='Envoyer'>";
  html += "</form></body></html>";
  server.send(200, "text/html", html);
  Serial.println("Page principale servie");
}




void handleSetColor() {
  if (server.hasArg("color")) {
    String color = server.arg("color");
    long number = strtol(color.c_str(), NULL, 16);
    int r = number >> 16;
    int g = number >> 8 & 0xFF;
    int b = number & 0xFF;

    analogWrite(redPin, r);
    analogWrite(greenPin, g);
    analogWrite(bluePin, b);

    Serial.print("Couleurs mises à jour : R=");
    Serial.print(r);
    Serial.print(", G=");
    Serial.print(g);
    Serial.print(", B=");
    Serial.println(b);
  }
  server.send(200, "text/plain", "Couleurs mises a jour");
}

void handleRedirect() {
  Serial.println("Redirection de HTTPS vers HTTP");
  serverSecure.sendHeader("Location", "http://" + WiFi.localIP().toString(), true);
  serverSecure.send(302, "text/plain", ""); // Redirection HTTP 302
  serverSecure.client().stop(); // Fermer la connexion client
}

void setup() {
  Serial.begin(115200);
  Serial.println("Démarrage...");

  // Configuration des pins
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Connexion au WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connexion au WiFi ");
  Serial.print(ssid);
  Serial.println("...");

  int fadeValue = 0;
  int fadeAmount = 1; // Réduire cette valeur pour un fade plus lent

  while (WiFi.status() != WL_CONNECTED) {
    // Fade in et fade out de la LED rouge
    analogWrite(redPin, fadeValue);
    fadeValue += fadeAmount;

    // Inverser la direction de l'effet de fade
    if (fadeValue <= 0 || fadeValue >= 1023) {
      fadeAmount = -fadeAmount;
    }

    delay(1); // Augmenter cette valeur pour un fade plus lent
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Connecté au WiFi");
  Serial.print("Adresse IP : ");
  Serial.println(WiFi.localIP());

  // Allumer la LED verte lorsque le WiFi est connecté
  analogWrite(redPin, 0);     // Rouge éteint
  analogWrite(greenPin, 1023); // Vert à pleine intensité
  analogWrite(bluePin, 0);    // Bleu éteint

  // Configuration du serveur web sur le port 80
  server.on("/", handleRoot);
  server.on("/setColor", handleSetColor);
  server.begin();
  Serial.println("Serveur web démarré sur le port 80");

  // Configuration du serveur web sur le port 443 pour la redirection
  serverSecure.onNotFound(handleRedirect);
  serverSecure.begin();
  Serial.println("Serveur web démarré sur le port 443 pour la redirection");
}

void loop() {
  server.handleClient();
  serverSecure.handleClient();
}
