
#include <VirtualWire.h>

#define rx_pin 0 // pin RX RF
#define led 1   // led and relais

void setup()
{
    vw_set_rx_pin(rx_pin);       //set the rx pin
    vw_set_ptt_inverted(true);  // configure push to talk polarity
    vw_setup(2000);            // Bits per sec
    vw_rx_start();
    pinMode(led, OUTPUT);
}

void loop()
{
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  if (vw_get_message(buf, &buflen)){
    char msg[buflen];
    for (int i = 0; i < buflen; i++){
      msg[i]= buf[i];
      }

    if( msg[1] == 'a' ){
      digitalWrite(led, HIGH);
    }
    if (msg[1] == 's'){
      digitalWrite(led, LOW);
    }

  }


}
