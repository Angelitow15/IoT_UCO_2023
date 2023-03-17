/*
=============================================================================
PRACTICA 2 MQTT Y REST 
Esta practica busca explorar la comunicación entre dominios en los que se 
pueda interactuar con varios dispositivos como lo es el protocolo MQTT 
y la información con la que se cuenta es una consumida por una API para 
dar la fecha y hora mundial
=============================================================================
*/

// Se importan las librerías necesarias para el código
//===========================================================================
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

//Se escriben las credenciales de conexión a la red y broker
//===========================================================================
const char* ssid = "xxxxxx";
const char* password =  "xxxx";
const char* mqttServer = "broker.mqtt-dashboard.com";
const int mqttPort = 1883;
const char* mqttuser = "xxxxxx";
const char* mqttpassword = "xxxx";

//Se establecen los topicos que se van a tener en el broker
//===========================================================================
const char* TopicInput = "input";
const char* TopicOutput = "output";
const char* TopicAlive = "alive";
const char* TopicStatusRequest = "StatusRequest";
const char* TopicJsonStatus = "JsonStatus";

//Se escribe la dirección de la cual se consume la información, este caso es 
//la world time api para la fecha y hora de alguna ciudad seleccionada
//==========================================================================
const String api= "http://worldtimeapi.org/api/timezone/";
String continenteCiudad;

//Se inicializan los objetos con los que se trabaja en el código
//===========================================================================
WiFiClient espClient;
WiFiClient clientHttp;
PubSubClient client(espClient);


void setup_wifi() {
//Se realiza la conexión a la red por medio de las credenciales utilizadas 
//===========================================================================
  delay(10);
  Serial.println();
  Serial.print(F("Connecting to ")) ;
  Serial.println(ssid);//Nombre de la red

  WiFi.begin(ssid, password);//credenciales

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));//Periodo de espera de conexión
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

String obtenerDiaLetras(int diaDeLaSemana){
//Se aplica una selección del nombre del día de la semana de acuerdo con el 
//Archivo Json que se obtiene de la api y baja un numero por defecto
//===========================================================================
  switch (diaDeLaSemana) {
  case 0:
    return "Domingo";
    break;
  case 1:
    return "Lunes";
    break;
  case 2:
    return "Martes";
    break;
  case 3:
    return "Miercoles";
    break;
  case 4:
    return "Jueves";
    break;
  case 5:
    return "Viernes";
    break;
  case 6:
    return "Sabado";
    break;
  default:
    return "Dia";
    break;
}
}

String obtenerMesLetras(String mesDelAnho){
//Al igual que con los días de la semana, con los meses del año se hace 
//verificación por condicionales para tener el texto equivalenteal numero
//===========================================================================
  if(mesDelAnho=="01"){
    return "Enero";
  }else if (mesDelAnho=="02"){
    return "Febrero";
  }else if (mesDelAnho=="03"){
    return "Marzo";
  }else if (mesDelAnho=="04"){
    return "Abril";
  }else if (mesDelAnho=="05"){
    return "Mayo";
  }else if (mesDelAnho=="06"){
    return "Junio";
  }else if (mesDelAnho=="07"){
    return "Julio";
  }else if (mesDelAnho=="08"){
    return "Agosto";
  }else if (mesDelAnho=="09"){
    return "Septiembre";
  }else if (mesDelAnho=="10"){
    return "Octubre";
  }else if (mesDelAnho=="11"){
    return "Noviembre";
  }else if (mesDelAnho=="12"){
    return "Diciembre";
  }else{
    return "Mes";
  }
}

String obtenerAnio(String cadena){
   char str[cadena.length()+1];
   unsigned int i=0;
   for (i=0;i<cadena.length();i++) {
      //Serial.println((char)cadena[i]);
      if((char)cadena[i] != '-'){
        str[i]=(char)cadena[i];
      }else {
        break;
      }

    }
    str[i] = 0; // Null termination
    Serial.println(str);
    return str;
}

String obtenerMes(String cadena){
  return cadena.substring(5,7);;
}

String obtenerDiaNumero(String cadena){
  return cadena.substring(8,10);
}

String obtenerHora(String cadena){
  return cadena.substring(11,16);
}

void armarOutput(String diaLetras, String anio, String diaNumero, String mes, String hora){

String cadena;

  cadena = diaLetras + ", " + diaNumero + " de " + mes + " de " + anio + " -- " + hora;
  
  Serial.println(cadena);
  client.publish(TopicOutput, cadena.c_str());
  
}

void consumirApi(String api, String continenteCiudad){

    HTTPClient http;
    if (http.begin(clientHttp, api+continenteCiudad)) { 
    
    int httpCode = http.GET();  //Realizamos la petición
    String codigoStr = String(httpCode);
        if (httpCode > 0) { //código de retorno

           Serial.println(httpCode); // esperamos que sea 200
           if(httpCode==200){
            client.publish(TopicStatusRequest,"HTTP OK");
           }
           //client.publish(TopicStatusRequest,codigoStr.c_str());
           String data = http.getString();
            client.publish(TopicJsonStatus,"JSON OK");
           
            
            Serial.println(data);
    
            StaticJsonDocument <256> doc;
            deserializeJson(doc,data);
    
            // deserializeJson(doc,str); can use string instead of payload
            const char* datetime = doc["datetime"];
            int day_of_week = doc["day_of_week"];
          
            Serial.print("datetime: ");
            Serial.println(datetime);
            
            Serial.println("day_of_week: "+day_of_week);
            armarOutput(obtenerDiaLetras(day_of_week),obtenerAnio(datetime),obtenerDiaNumero(datetime),obtenerMesLetras(obtenerMes(datetime)),obtenerHora(datetime));
    
    
            
            //client.publish(TopicOutput,data);
          }
          else {
            Serial.println("Error en la petición HTTP");
          }
 
    http.end(); //cerramos conexión y liberamos recursos
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  
    Serial.println("callback se esta ejecutando: ");
    
    char str[length+1];
    Serial.print("topico: ");
    Serial.println(topic);
    Serial.print("mensaje: ");
    unsigned int i=0;
    for (i=0;i<length;i++) {
      Serial.print((char)payload[i]);
      str[i]=(char)payload[i];
    }
    Serial.println();
   
    str[i] = 0;
    continenteCiudad = str;
    Serial.println("zona horaria: "+ continenteCiudad);
    consumirApi(api,continenteCiudad);

}

void setup() {
 
  //Start Serial Communication
  Serial.begin(115200);
  
  //Connect to WiFi
  setup_wifi();

  //Connect to MQTT Broker
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  //MQTT Connection Validation
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("pruebas"/*,mqttuser,mqttpassword*/)) {
 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
  
  //Publish to desired topic and subscribe for messages
  client.publish(TopicAlive, "MEQUIEROIRADORMIR :v");
  client.subscribe(TopicInput);
 
}

void loop() {
  //MQTT client loop
  client.loop();
}