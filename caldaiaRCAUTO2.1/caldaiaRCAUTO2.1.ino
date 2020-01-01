/*
Firmware per il comado remoto della caldaia tramite relais. */


#define rele1 11
#define rele2 10
#define butt 4
#define ledR 13
#define ledY 6
#define sens 0
#define trigg 1


boolean manual = 0;
boolean forced = 0;
boolean command =  0;
boolean caldaiaState = 0;


int LIMITemp = 27;
int accensioni = 0;

#define interval  1000
#define timetresh  3600000


unsigned long previousMillis = 0;
int ledState = LOW;
unsigned long startime = 0;

bool firstime = 1;


void setup()
{
  Serial.begin(9600);  //enable for debug
  Serial.println("hello");
  /* set pinMode */
  pinMode(rele1, OUTPUT);
  pinMode(rele2, OUTPUT);
  pinMode(ledR, OUTPUT);
  pinMode(ledY, OUTPUT);
  pinMode(butt, INPUT);
  pinMode(sens, INPUT);
  pinMode(trigg, INPUT);

  /* set relais off and led off */
  digitalWrite(rele1,HIGH);  //when HIGH relais is off
  digitalWrite(rele2,HIGH);
  digitalWrite(ledR, LOW);
  digitalWrite(ledY, LOW);

  /* two blinks when setup completed*/
  digitalWrite(ledY, HIGH);
  delay(100);
  digitalWrite(ledY, LOW);
  delay(100);
  digitalWrite(ledY, HIGH);
  delay(100);
  digitalWrite(ledY, LOW);
  Serial.println("GSM initialized");


}


void loop()
{
  Serial.print( "first irime" );
  Serial.println(firstime);
  unsigned long currentMillis = millis();
  if (currentMillis > startime + timetresh){
    firstime = 1;
  }

    Serial.print( "first irime dopo if" );
  Serial.println(firstime);

  String azione;   // string which contains the message
  char recvNum[20];  // array which contains the number of the sender of SMS
  int tempread = temperatura(); // get temperature
  Serial.println(tempread);
  if (manual){
    digitalWrite(ledY, HIGH); // when manual led yellow is ON
  }
  else{
    if (currentMillis - previousMillis >= interval) {
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




  //Serial.println(azione);


// se è MANUAL
  if (manual == 1 && forced == 0){
    if ( analogRead(butt) > 500) {
        action(1);
    }
    else{
        action(0);
    }
  }

// se è AUTO
  if (manual == 0 && forced == 0) {

    if (tempread < LIMITemp && firstime == 1 ){ // when temperature goes down the limit and it's the first time
        action (1);
        startime = millis();
        firstime = 0;
    }
    if (tempread < LIMITemp && firstime == 0 ){ // when temperature goes down the limit and not the first time
        action(1);
    }
    if (tempread > LIMITemp && firstime == 1 ){
        action(0);
    }
    if (tempread > LIMITemp && firstime == 0){

    }
  }

// modalità forzata
if (forced==1){
  if (command ==  1){
    action (1);
  }
  if (command == 0){
    action (0);
  }
}
   if ( analogRead(trigg) > 500) {
    manual = 1;
    forced = 0;
    }

  delay(1000);


   Serial.print( "first irime" );
  Serial.println(firstime);
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
