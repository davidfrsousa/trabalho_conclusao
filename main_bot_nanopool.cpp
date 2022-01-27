#include <Arduino.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#define Opto 5
#define Verde 4
#define BOT_TOKEN "XXXXXXXXXXXXXXXXXXXXXXXXX"
#define INTERVAL 2000
#define SSID "XXXXXXXXXXXXXXXXXXXXXX"
#define PASSWORD "XXXXXXXXXXXXXXXXXXX"

const String urlHashrate = "https://api.nanopool.org/v1/eth/reportedhashrate/0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
const String urlBalance = "https://api.nanopool.org/v1/eth/balance/0xFFFFFFFFFFFFFFFFFFFFFFFFFFF";
const String urlWorkers = "https://api.nanopool.org/v1/eth/reportedHashrates/0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
const String urlEstimate = "https://api.nanopool.org/v1/eth/approximated_earnings/425";
WiFiClientSecure client;
HTTPClient http;
UniversalTelegramBot bot(BOT_TOKEN, client);
String rHashrate, aBalance;
String workers = "";

//Tempo
uint32_t lastCheckTime = 0;
//unsigned long bot_lasttime;
//const unsigned long BOT_MTBS = 1000;

//N de usuários
#define SENDER_ID_COUNT 1

//IDs de usuários
String validSenderIds[SENDER_ID_COUNT] = {"1796248229"};

boolean validateSender(String senderId)
{
  for (int i = 0; i < SENDER_ID_COUNT; i++)
  {
    if (senderId == validSenderIds[i])
    {
      return true;
    }
  }
  return false;
}

void handleHashrate(String chatId)
{
  http.begin(urlHashrate);
  int httpCode = http.GET();
  if (httpCode > 0)
  {
    double valor = 0;

    Serial.println("Requisição GET de Hashrate");

    DynamicJsonDocument doc(512);
    DeserializationError erro = deserializeJson(doc, http.getStream());

    if (erro)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(erro.f_str());
      return;
    }

    rHashrate = "Hashrate: ";
    valor += doc["data"].as<double>();
    rHashrate += valor;
    rHashrate += " MH/s";
    String message = rHashrate;
    bot.sendMessage(chatId, message, "HTML");
  }
  else
  {
    Serial.print("Código de erro: ");
    Serial.println(httpCode);
  }

  http.end();
}

void handleBalance(String chatId)
{
  http.begin(urlBalance);
  int httpCode = http.GET();

  if (httpCode > 0)
  {
    String valor = "";
    Serial.println("Requisição GET de Balance");

    DynamicJsonDocument doc(512);
    DeserializationError erro = deserializeJson(doc, http.getStream());

    if (erro)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(erro.f_str());
      return;
    }
    aBalance = "Saldo: ";
    valor += doc["data"].as<String>();
    aBalance += valor;
    aBalance += " ETH";
    String message = aBalance;
    bot.sendMessage(chatId, message, "HTML");
  }
  else
  {
    Serial.print("Código de erro: ");
    Serial.println(httpCode);
  }

  http.end();
}

void handleWorkers(String chatId)
{
  http.begin(urlWorkers);
  int httpCode = http.GET();

  if (httpCode > 0)
  {
    String message = "";
    String valor = "";
    Serial.println("Requisição GET de Workers");

    DynamicJsonDocument doc(24576);
    DeserializationError erro = deserializeJson(doc, http.getStream());

    if (erro)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(erro.f_str());
      return;
    }
    for (int i = 0; i < 5; i++)
    {
      if (i != 4)
      {
        workers += doc["data"][i]["worker"].as<String>() + ": ";
        valor = doc["data"][i]["hashrate"].as<String>();
        workers += valor + " MH/s\n";
      }
      else
      {
        workers += doc["data"][i]["worker"].as<String>() + ": ";
        valor = doc["data"][i]["hashrate"].as<String>();
        workers += valor + " MH/s";
      }
    }

    message += workers;

    bot.sendMessage(chatId, message, "HTML");
  }
  else
  {
    Serial.print("Código de erro: ");
    Serial.println(httpCode);
  }

  http.end();
}

void handleStale(String chatId)
{
  http.begin(urlBalance);
  //int httpCode = http.GET();

  /*if (httpCode > 0) {
    String stale = "";

    Serial.println("Requisição GET de Stale shares");

    DynamicJsonDocument doc(512);
    deserializeJson(doc, http.getStream());
    stale = "Stale shares: ";
    stale += doc["data"]["staleShares"].as<String>();
    String message = stale;
    bot.sendMessage(chatId, message, "HTML");
  }
  else {
    Serial.print("Código de erro: ");
    Serial.println(httpCode);
  }*/

  http.end();
}

void handleEstimate(String chatId)
{
  http.useHTTP10(true);
  http.begin(urlEstimate);
  int httpCode = http.GET();

  if (httpCode > 0)
  {
    String valor = "";

    Serial.println("Requisição GET de Estimativa");

    DynamicJsonDocument doc(1024);
    DeserializationError erro = deserializeJson(doc, http.getStream());

    if (erro)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(erro.f_str());
      return;
    }

    JsonObject data = doc["data"];
    JsonObject day = data["day"];
    valor += "Saldo: ";
    valor += day["coins"].as<String>();
    valor += " ETH";
    String message = valor;
    bot.sendMessage(chatId, message, "HTML");
  }
  else
  {
    Serial.print("Código de erro: ");
    Serial.println(httpCode);
  }

  http.end();
}

void handleNotFound(String chatId)
{
  String message = "Comando não encontrado";
  bot.sendMessage(chatId, message, "HTML");
}

void handleNewMessages(int numNewMessages)
{
  Serial.print("Esperando novas mensagens...");
  Serial.println(numNewMessages);
  for (int i = 0; i < numNewMessages; i++)
  {
    Serial.println("Nova mensagem!");
    String chatId = String(bot.messages[i].chat_id);
    String senderId = String(bot.messages[i].from_id);

    Serial.println("SenderId: " + senderId);
    boolean validSender = validateSender(senderId);

    if (!validSender)
    {
      bot.sendMessage(chatId, "Você não tem permissão", "HTML");
      continue;
    }

    String text = bot.messages[i].text;

    if (text.equalsIgnoreCase("hashrate"))
    {
      handleHashrate(chatId);
    }
    else if (text.equalsIgnoreCase("balance"))
    {
      handleBalance(chatId);
    }
    else if (text.equalsIgnoreCase("workers"))
    {
      handleWorkers(chatId);
    }
    else if (text.equalsIgnoreCase("stale"))
    {
      handleStale(chatId);
    }
    else if (text.equalsIgnoreCase("estimate"))
    {
      handleEstimate(chatId);
    }
    else
    {
      handleNotFound(chatId);
    }
  }
}

void setup()
{
  pinMode(Opto, OUTPUT);
  pinMode(Verde, OUTPUT);
  Serial.begin(115200);
  Serial.print("Conectando ao WiFi...");

  WiFi.begin(SSID, PASSWORD); //Tenta conectar à rede WiFi

  //Enquanto o status não for de conectado imprime pontos na tela
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  //Mensagem de conexão sucedida
  Serial.println("\nConectado");
  pinMode(Opto, OUTPUT); //Define o pino do acoplador óptico como saída

  client.setInsecure();
}

void loop()
{
  uint32_t now = millis();

  if (now - lastCheckTime > INTERVAL)
  {
    lastCheckTime = now;
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    handleNewMessages(numNewMessages);
  }
}
