#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Remplacez par vos identifiants WiFi
const char* ssid = "MYSSID";
const char* password = "MYPASS";
// Pins pour les LEDs
const int bluePin = 14; // D5
const int redPin = 13; // D6
const int greenPin = 12; // D7

ESP8266WebServer server(80);
ESP8266WebServer serverSecure(443);

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>Contrôle des LEDs</title></head><body>";
  html += "<h1>Contrôle des LEDs</h1>";
  html += "<form action='/setColor' method='GET'>";
  html += "Rouge: <input type='range' name='r' min='0' max='1023'><br>";
  html += "Vert: <input type='range' name='g' min='0' max='1023'><br>";
  html += "Bleu: <input type='range' name='b' min='0' max='1023'><br>";
  html += "<input type='submit' value='Envoyer'>";
  html += "</form></body></html>";
  server.send(200, "text/html", html);
  Serial.println("Page principale servie");
}

void handleSetColor() {
  if (server.hasArg("r") && server.hasArg("g") && server.hasArg("b")) {
    int r = server.arg("r").toInt();
    int g = server.arg("g").toInt();
    int b = server.arg("b").toInt();

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
  server.send(200, "text/plain", "Couleurs mises à jour");
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

  // Connexion au WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connexion au WiFi ");
  Serial.print(ssid);
  Serial.println("...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connecté au WiFi");
  Serial.print("Adresse IP : ");
  Serial.println(WiFi.localIP());

  // Configuration des pins
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

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
