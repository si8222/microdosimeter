#ifdef ESP8266
    extern "C" {
        #include "user_interface.h"
    }
#endif
#define num_samples 512
uint16_t adc_addr[num_samples]; // point to the address of ADC continuously fast sampling output
uint16_t adc_num = num_samples; // sampling number of ADC continuously fast sampling, range [1, 65535]
uint8_t adc_clk_div = 32; // ADC working clock = 80M/adc_clk_div, range [8, 32], the recommended value is 8


int low = 0;
float threshold = 0.1;
float high[9999];
int size = 9999;
float last =0.0;
float maxvalue = 0.0;
float fnull=0.0;
float voltage =0.0;
int i =0;
int count=0;



void setup() {
Serial.begin(9600);
pinMode(2, OUTPUT);
pinMode(5, OUTPUT);
}

void loop() {
 
#ifdef ESP8266
wifi_set_opmode(NULL_MODE);
system_soft_wdt_stop();
ets_intr_lock( ); //close interrupt
noInterrupts();
//Serial.println(threshold);
 //int sensorValue = analogRead(A0);
 int sensorValue=system_adc_read();
 voltage = -0.022+sensorValue*(3.145/1023.0);
 //Serial.println(voltage);
  if (voltage>threshold && i<100){
    //last=voltage;
    //high[i]=voltage;
    i++;
    //low=NULL;
    digitalWrite(5, LOW); 
    if(voltage>maxvalue){
      maxvalue=voltage;
      low=NULL;
    }//ermittelt maxwert aus Messreihe
    /*Speichert sobald Schwelle überschrittenn wird alle Werte in Array ab*/
  }else{//Schwelle wird nicht erreicht oder I größer als 999
    digitalWrite(2, HIGH); 
    if(i>50){
      //maximalwert des arrays ermitteln
      digitalWrite(2, LOW); 
      Ausgabe();
      count++;
      Serial.println(count);
      /*Serial.print("Signal[V]: ");
      Serial.print(maxvalue);
      Serial.print("\t");
      Serial.print("Voltage[V]: "); 
      Serial.print(voltage);
      Serial.println();
      */
      maxvalue=0.0;
      i=NULL;
      //Ist die Schwelle in länge eines Pulses dann wird der Maximalwert des Arrays ausgegeben      
    }else{
      low++;
      maxvalue=0.0;
      //Signal zu kurz für Puls, ausreißer wird gelöscht
    }
    if(low>100){
      Ausgabe();
      /*Serial.print("Signal[V]: ");
      Serial.print(maxvalue);
      Serial.print("\t");
      Serial.print("Voltage[V]: "); 
      Serial.print(voltage);
      Serial.println();
      */
      digitalWrite(5, HIGH); 
      low=NULL;
      i=NULL;
      //Wenn lange kein Ereignis gemessen wird, soll aktueller Messwert ausgegeben werden
    }
  }
  #endif
}


void ClearArray(){
  for (int x=0;x<9999; x++){
    high[x]=NULL;
  }

}
void Ausgabe(){
  //myTime = millis();
  Serial.print("Signal[V]:"); Serial.print(maxvalue); Serial.print("\t"); 
  Serial.print("Voltage[V]:"); Serial.print(voltage); Serial.print("\t"); 
  Serial.print("Threshold[V]:"); Serial.print(threshold);
  Serial.println();
}
