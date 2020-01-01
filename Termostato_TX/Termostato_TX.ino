#include <dht11.h>
#include <LiquidCrystal.h>
#include <VirtualWire.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

dht11 DHT;
#define DHT11_PIN 11
#define TXpin 12
#define backlight_pin 10

int pulsante = 0;
int UPlimit = 10;
int LOWlimit = 10;

boolean blt = LOW;        //retroilluminazione
boolean statoSens = LOW;  // statoSens del sensore
boolean acceso = LOW;

int temp = 0;
int hum = 0;


#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5


char accendi[3] = {'a','a','a'};
char spegni[3] = {'s','s','s'};

void setup(){
  
 Serial.begin(9600);
 pinMode(10, OUTPUT); 
 lcd.begin(16, 2); 
 vw_set_tx_pin(TXpin);               //set the tx pin
 vw_set_ptt_pin(13);                 // use this because default is 10 and would interfer with backlight
 vw_set_ptt_inverted(true);          // configure push to talk polarity
 vw_setup(2000);                     // Bits per sec 
 digitalWrite(backlight_pin, blt);
}
 
void loop(){ 
  
 statoSens = sensore(); // leggo temp e hum per ogni ciclo
 
 if ( statoSens == 0) { // se il sensore non funziona 
  digitalWrite(backlight_pin, HIGH);  // accendo il display
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Err sens temp");
  lcd.setCursor(0,1);
  lcd.print("manual");  
  /* MANUAL MODE */
  manual();  
    
 }// fine if sensore non presente 
 
 else{   
  if (readButtons() == btnSELECT){  // uso il pulasnte SELECT per impostare la retroilluminazione
    if (blt == LOW){
      blt = HIGH;
    }
    else {
      blt = LOW;
    }
  }
  
 if (blt == HIGH){ // solo se ho attivato le retroilluminazione  
  doppio_pulsante( readButtons() );

 }       


  /* INVIO MESSAGGIO AL RICEVITORE */ 

  if (temp > UPlimit  ){   // inserire and acceso == HIGH se si vuole che invii solo un mesaggio
    on_off2 (LOW);
  }
  if (temp < LOWlimit ) { // inserire eventualmente and acceso == LOW
    on_off2 (HIGH);
  }
  
  /* SCRIVO SUL DISPLAY*/
  
  digitalWrite(backlight_pin, blt);  // accendo o spengo la retroilluminazione 
  lcd.clear();   
  
  /* prima riga*/
  lcd.setCursor(0,0); 
  String msg1 = "Temp:" ;
  msg1 = msg1 + temp +"C" + " Hum:" + hum +"%"; // printo temperatura e umidità
  lcd.print(msg1);
  
  /* seconda riga*/
  lcd.setCursor(0,1);
  String msg2 = "Min";
  msg2 = msg2 + LOWlimit + " Max" + UPlimit; // printo i limiti
  if (acceso == LOW){         // printo acceso o spento
    msg2 = msg2 + " OFF"; 
  }
  else{
    msg2 = msg2 + " ON";
  }  
  lcd.print(msg2);
  
 }// fine else 

 
}//fine loop


/*Functions*/

int readButtons(){
  int lettura = analogRead(0);      // read the value from the buttons
  if (lettura > 1000) return btnNONE; 
  if (lettura < 50)   return btnRIGHT;  
  if (lettura < 195)  return btnUP; 
  if (lettura < 380)  return btnDOWN; 
  if (lettura < 555)  return btnLEFT; 
  if (lettura < 790)  return btnSELECT;   
  return btnNONE; 
}

int sensore (){ 
  int chk;
  chk = DHT.read(DHT11_PIN);    // read data from sensor
  if (chk == DHTLIB_ERROR_CHECKSUM or chk == DHTLIB_ERROR_TIMEOUT ){
    delay (1000);
    return 0;  
  }
  else {
    hum = DHT.humidity;
    temp = DHT.temperature;
    Serial.println(temp);
    delay(1000);
    return 1;
  }
}

void manual (){
int pulsante_on_error = readButtons();
  if (pulsante_on_error == btnUP){
    on_off (HIGH);
    delay (500);
    on_off (HIGH);
    delay(500);
    on_off (HIGH);
    }
  if (pulsante_on_error == btnDOWN){       
    on_off (LOW);
    delay (500);
    on_off (LOW);
    delay(500);
    on_off (LOW);
    }  
  lcd.setCursor(13,1); 
  if (acceso == LOW){         // printo acceso o spento
    lcd.print("OFF");
  }
  else{
    lcd.print("ON");
  }
}

void on_off (bool comando){
  if (comando == HIGH){
    acceso = HIGH;
    vw_send( (uint8_t*)accendi, 3 ); 
    //Serial.println("accendi");
    vw_wait_tx();
    delay(50);
  }
  else{
    acceso = LOW;
    vw_send( (uint8_t*)spegni, 3 ); 
    //Serial.println("spegni");
    vw_wait_tx();
    delay(50);    
  }
}

void on_off2 (bool comando){
  if (comando == HIGH){
    acceso = HIGH;
    //Serial.println("accendi");
    vw_send( (uint8_t*)accendi, 3 );     
    vw_wait_tx();
    delay(100);
    vw_send( (uint8_t*)accendi, 3 );
    vw_wait_tx();
    delay(100);
  }
  else{
    acceso = LOW;
    //Serial.println("accendi");
    vw_send( (uint8_t*)spegni, 3 );     
    vw_wait_tx();
    delay(100);
    vw_send( (uint8_t*)spegni, 3 );
    vw_wait_tx();
    delay(100);
  }
}

void doppio_pulsante(int pulsante){
  if (pulsante == btnUP){
    UPlimit ++;   
  }
  if (pulsante == btnDOWN){
    if ( (UPlimit - 1) >= LOWlimit ){
      UPlimit --;
    }    
  }
  if (pulsante == btnRIGHT){
    if ( (LOWlimit + 1) <= UPlimit ){
      LOWlimit ++;  
    }      
  }
  if (pulsante == btnLEFT){
    LOWlimit --;
  }
}

void menu (int pulsante){
  if (pulsante == btnRIGHT){
    
  }
}

