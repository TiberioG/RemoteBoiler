/*
Firmware per il comado remoto della caldaia tramite relais. */

#include <GSM.h> // library for Arduino original GSM Shield

#define rele1 11
#define rele2 10
#define butt 4
#define ledR 13
#define ledY 6
#define sens 0
#define blinktime 1000
#define manualbutton 1


boolean manual = 0;
boolean forced = 0;
boolean command =  0; // flag per comando mandato via sms
boolean caldaiaState = 0; // cosa sta facendo ora la caldaia

int LIMITemp = 10; // limite temperatura
int accensioni = 0;

unsigned long previousMillis = 0;
int ledState = LOW;

GSM gsmAccess;
GSM_SMS sms;

void setup()
{
  //Serial.begin(9600);  //enable for debug
  //Serial.println("hello");

  pinMode(rele1, OUTPUT);
  pinMode(rele2, OUTPUT);
  pinMode(ledR, OUTPUT);
  pinMode(ledY, OUTPUT);
  pinMode(butt, INPUT);
  pinMode(sens, INPUT);
  pinMode(manualbutton, INPUT);

  /* set relais off and led off */
  digitalWrite(rele1,HIGH);  //when HIGH relais is off
  digitalWrite(rele2,HIGH);
  digitalWrite(ledR, LOW);
  digitalWrite(ledY, LOW);

  /* start GSM connection */
  boolean notConn = true;
  while(notConn)
  {
    if(gsmAccess.begin("") == GSM_READY){
      notConn = false;
    } else {
      blink(4, 200, ledY);
    }
  }
  blink(2, 100, ledY); // setup completed
  //Serial.println("GSM initialized");
}// end of setup

void loop()
{
  unsigned long currentMillis = millis();
  String azione;   // string which contains the message
  char recvNum[20];  // array which contains the number of the sender of SMS
  float tempread = temperatura(); // get temperature

  if (manual){
    digitalWrite(ledY, HIGH); // when manual led yellow is ON
  }
  else{
    if (currentMillis - previousMillis >= blinktime) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    // set the LED with the ledState of the variable:
    digitalWrite(ledY, ledState);
    }
  }

  if ( sms.available() ) // if there is a message
  {
    //Serial.println("Message received ");
    sms.remoteNumber(recvNum, 20); //stores the number
    char c;
    int cont = 0;
    char msg[sms.available()];
    while( c = sms.read() )
      {
       msg[cont] = c;
       cont++;
      }
    sms.flush();
    azione = msg;
  }
  sms.flush();
  delay(1000);

   if (azione.equals("auto") || azione.equals("Auto") ){ // toggles to automatic mode
      manual = 0;
      forced = 0;
    }
    if (azione.equals("manual") || azione.equals("Manual") ){ //enables manual
      manual = 1;
      forced = 0;
    }
    if( azione.equals("spegni") || azione.equals("Spegni") || azione.equals("OFF") ) { // turn off
      manual = 0;
      forced = 1;
      command = 0;
    }
    if( azione.equals("accendi") || azione.equals("ON")  || azione.startsWith("acc") ) { // turn on
   //Serial.println("sono in ifazione");
      manual = 0;
      forced = 1;
      command = 1;
    }

    if( azione.startsWith("set") || azione.startsWith("Set") ) {   // send a message like "set13" to set temp limit to 13 °C
      azione.remove(0, 3);
      LIMITemp = azione.toInt();
    }
    if ( azione.equals("status") || azione.equals("Status") )  { // send status of system
      String stato = "Stato: ";
      if(manual){
       stato = stato + "manual" ;
      }
      if (!manual){
       stato = stato + "auto" ;
      }
      if (forced){
       stato = stato + "forced";
      }
      stato = stato + " Caldaia:";
      if (caldaiaState){
        stato = stato + " accesa";
      }
      else{
        stato = stato + " spenta";
      }
      stato = stato + " Limit:" + LIMITemp + " Temp:" + tempread +"acc:" +accensioni;

      sendSMS(stato, recvNum);
    }

  if (manual == 1 && forced == 0){ // se è manuale cosidero il bottone
    if ( analogRead(butt) > 500) {
        action(1);
    }
    else{
        action(0);
    }
  }

  if (manual == 0 && forced == 0) { // se è auto
    if ( tempread < LIMITemp - 1 ){  // when temperature goes down the limit
        action (1);
    }
    if (tepread > LIMITemp + 1){
        action (0);
    }
  }


  if (forced==1){
    if (command ==  1){
      action (1);
    }
    if (command == 0){
      action (0);
    }
}


   if ( analogRead(manualbutton) > 500) {
    manual = 1;
    forced = 0;
    }

  delay(1000);
}



float temperatura (){  // returns actual temperature
  //float temp = analogRead(sens);
  float temp = 0;
  for (int i = 0; i < 10; i++){
    temp = temp + analogRead(sens);
    delay(10);
  }
  temp = temp / 10;
  temp = temp * 0.48828125;
  return temp;

}

void sendSMS(String text, char number[20]){ //sends SMS
  sms.beginSMS(number);
  sms.print(text);
  sms.endSMS();
  Serial.println("message sent");
}


void action (boolean state){
  if (state == 0){  //spegni
    digitalWrite(rele1,HIGH);
    digitalWrite(rele2,HIGH);
    digitalWrite(ledR,LOW);
    caldaiaState =  0;
  }
  else {  //accendi
    digitalWrite(rele1,LOW);
    digitalWrite(rele2,LOW);
    digitalWrite(ledR,HIGH);
    if (caldaiaState == 0){
      accensioni++;
      }
    caldaiaState = 1;
  }
}


void blink(int rep, int dur, int led){
  for (int i = 0; i < rep; i++){
    digitalWrite(led, HIGH);
    delay(dur);
    digitalWrite(led, LOW);
    delay(dur);
  }
}
