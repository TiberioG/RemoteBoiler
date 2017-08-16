/*
Firmware per il comado remoto della caldaia tramite relais. */

#include <GSM.h> // library for Arduino original GSM Shield

#define tiberioNum "3398593331"
#define casaNum "3383913320"

#define rele1 12
#define rele2 11
#define butt 4
#define ledR 13
#define ledY 6
#define sens 0
#define reset 5


char cmd[10];
boolean manual = 1;
boolean automatic = 0;
int caldaiaState = 0;
int LIMITemp = 12;

GSM gsmAccess;
GSM_SMS sms;

void setup() 
{  
  //Serial.begin(9600);  enable for debug
  
  /* set pinMode */
  pinMode(rele1, OUTPUT);
  pinMode(rele2, OUTPUT);
  pinMode(ledR, OUTPUT);
  pinMode(ledY, OUTPUT);
  pinMode(butt, INPUT); 
  pinMode(sens, INPUT);
  pinMode(reset, INPUT); 

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
      //Serial.println("Not connected");
      delay(1000);
    }
  }
  //Serial.println("GSM initialized");

  /* two blinks when setup completed*/
  digitalWrite(ledY, HIGH);
  delay(100);
  digitalWrite(ledY, LOW);
  delay(100);
  digitalWrite(ledY, HIGH);
  delay(100);
  digitalWrite(ledY, LOW);
}


void loop() 
{
  String azione;   // string which contains the message 
  char recvNum[20];  // array which contains the number of the sender of SMS
  float tempread = temperatura(); // get temperature
  
  //Serial.print(analogRead(butt));
  //Serial.print("  ");
  //Serial.println(tempread);

  if (manual){
    digitalWrite(ledY, HIGH); // when manual led yellow is ON
  }
  else{
    digitalWrite(ledY, LOW);
  }

  if ( sms.available() ) // if there is a message
  {        
    //Serial.println("Message received ");
    sms.remoteNumber(recvNum, 20); //stores the number
    //Serial.println(recvNum);    
    azione = String( getAzione() ); // stores the message
    pulisciStringa(cmd, 10);  //clean previous string 
    //Serial.println(azione); 

    if (azione.equals("auto") ){ // toggles to automatic mode
      manual = 0;
      automatic = 1;
    }
    if (azione.equals("noauto") ){ //disable automatic
      automatic = 0;
    }  
    if (azione.equals("manual") ){ //enables manual
      manual = 1;
      automatic = 0;
    }
    if( azione.equals("spegni") ) { // turn off
      action (0);
      manual = 0;
    }       
    if( azione.equals("accendi") ) { // turn on
      action(1);
      manual = 0;
    }

    if( azione.equals("temp") )    { // send actual temperature         
      String tempToSend = String( temperatura() );
      sendSMS(tempToSend, recvNum);       
    }
    if( azione.startsWith("set") ) {   // send a message like "set13" to set temp limit to 13 °C
      azione.remove(0, 3);
      LIMITemp = azione.toInt();      
    }
    if ( azione.equals("status") )  { // send status of system
      String stato = "Stato: ";
      if(manual){
       stato = stato + "manual" ;
      }
      if (automatic){
       stato = stato + "auto";
      }
      stato = stato + " Interr.:";
      if (caldaiaState){
        stato = stato + "acceso";
      }
      else{
        stato = stato + "spento";
      }
      stato = stato + " Limit:" + LIMITemp + " temp:" + tempread;
      
      sendSMS(stato, recvNum);
    }
    
  }

  if (manual){  
    
    if ( analogRead(butt) > 500) { 
        action (1);
    }
    else{
        action(0);
    }
  }
  
  if (automatic){
    if ( tempread < LIMITemp){  // when temperature goes down the limit
        action (1);
    }
    else {
        action (0);
    }
  }

  //Serial.print(buttState);
  //Serial.print("    ");
  //Serial.print(smState);
  //Serial.print("    ");
  //Serial.print(tempState);
  //Serial.print("    ");
  //Serial.println("");  

  
   delay(); 
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

void pulisciStringa(char* tmp,int dim) {
  for (int x = 0; x < dim; x++)
  tmp[x] = 0;
  tmp[0] = '\0';
}

char* getAzione(){
  char c;
  int cont = 0;
  char msg[10];
    
  while( c = sms.read() ){
       msg[cont] = c;
       cont++;
  }
  delay(1000);
  sms.flush(); 
     
  for(int j = 0; j < cont; j ++){
     cmd[j] = msg[j];
  }     
  return cmd; 
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
    caldaiaState = 1;
  }
}




