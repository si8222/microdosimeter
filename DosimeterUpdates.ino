#include <SPI.h>
#include "FS.h"
#include <SD.h>
#include "RTClib.h"
#include <cstdint>
const int chipSelect = SS;



#include "driver/adc.h"

#include "esp_adc_cal.h"

#define ADCPIN 32 

//define CHANNELS 512 // number of channels in buffer for histogram, including negative numbers 9-Bit(512)
//#define CHANNELS 1024// 10 bit
//#define CHANNELS 2048 //11bit
#define CHANNELS 4096 //12bit

#define RESET 27// SD2

#define VETO 34

RTC_DS3231 rtc;
//Test deafult auf 32 bit, testen ob analogSetWidth 12 bit matchen muss mit bits der Channels
//uint_fast8_t CHANNELS;




uint_fast32_t u_sensor, maximum=0,threshold=0, i=0,j=0,signalveto=0;

uint_fast32_t histogram[CHANNELS];

void setup() 

{
  Serial.begin(115200);
  uint16_t sixteenBitInteger = 0xABCD; // Beispiel 16-Bit-Integer
  uint16_t mask = 0x0FFF; // Bitmaske zum Abschneiden der oberen 4 Bits
  uint16_t signal0 = sixteenBitInteger & mask; // Kombinieren mit Bitmaske
 








  analogSetWidth(12);//#bits

  analogSetAttenuation(ADC_6db);//Messbereich

  Serial.println("Start");

  // Peak detector Reset
  pinMode(RESET, OUTPUT); 

  digitalWrite(RESET, LOW); //grounded den Peak Detect

  pinMode(RESET, INPUT); 

  //

  pinMode(RESET, OUTPUT); // to start Reset on High

  digitalWrite(RESET, HIGH); //aktiviert Peak Detect

  Serial.println(millis());
  ////
  //Insert RTC for Time-Stamp

  /*
  if (!rtc.begin()) 
  {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  rtc.adjust(DateTime(__DATE__, __TIME__)); //set RTC to Now, only needed when rtc is not accurate

  */
  //add SD-Modul
  if(SD.begin(SS))
  {
    Serial.println("Karte gefunden.\n");
  }

  if(!SD.begin(SS))
 {
    Serial.println("Karte nicht gefunden!\r");
    return;
 }
  //***********************************************
  //Channels werden geleert 
  for(int n=0; n<CHANNELS; n++)
  {
    histogram[n]=0;
  }

}

void loop()
{
  pinMode(RESET,INPUT);//in der schleife die ganze Zeit im Input

  uint_fast32_t signal0=analogRead(ADCPIN);

  //***********************************************
  //Veto Impelentation
  //signalveto=digitalRead(VETO);
  //Serial.println("Veto: ");
  //Serial.println(signalveto);
  //***********************************************
  //Auslese Logik
  Serial.println(signal0);
  //Thereshold wird festgelegt
  if (signal0>threshold)
  {
    maximum=signal0;
    //Wenn Signal > thereshold, dann werden n Werte aufgenommen und daraus der größte Ermittelt und in Channels gespeichert.
    //Die länge der Messung muss an die Signallänge angepasst werden. n=2 bedeutet etwa 140ns Messung.   
    for (int n=0; n<2 ;n++) 
    {
      uint_fast32_t signal0=analogRead(ADCPIN);//neu weil bei jedem Durchgang neu ausgemessen werden soll
      if(signal0>maximum)
      { 
        uint_fast32_t maximum=signal0; 
      } 
    } 
    //Serial.println(maximum);
   
   // delay(5);
    //Serial.println(i); 
    pinMode(RESET, OUTPUT); // Reset for peak detetor
    digitalWrite(RESET, LOW);
    digitalWrite(RESET,HIGH);
    histogram[maximum]++;
    //Serial.println(maximum);
    maximum=0;
    i++;

    //Delay um der Kapazität des Dosimeters Zeit zu geben sich zu entladen. 
    //Sweet spot finden um trotzdem möglichst viele Signale zu messen.
    //delay(1);
    delayMicroseconds(200);
    
  }
  
  //***********************************************
  //Logik zum abspeichern der Daten. i gibt die Anzahl der Werte gespeichert in Channels an. 
  //Maximal  65536 da Esp32 32 bits.
  if(i>200000)
  {
    //RTC initialisieren
    DateTime now = rtc.now();
    Serial.println(millis());
    //Test Histogram

    
    /*
    Ausgabe des Histograms in die console
    for(int j=0; j < CHANNELS; j++)
    {
      Serial.println(histogram[j]);
    }
    */



    int count=i;
    char buf2[] = "YYMMDD-hh:mm:ss";
    String dataString = "";
    // make a string for assembling the data to log:
    dataString += "MIT,";
    dataString += now.toString(buf2);
    dataString += ",";
    dataString += String(count);
    //Schreibt Zeit der aktuellen Messung in die Konsole
    Serial.println(dataString);

    //Fehler wenn Karte während der Messung den Kontakt verliert
     if(!SD.begin(SS))
    {
    Serial.println("Karte nicht gefunden!\r");
    return;
    }

    //Channels in Datastring speichern
    for(int n=0; n<CHANNELS; n++)  
    {
      dataString += ",";
      dataString += String(histogram[n]); 
    }
    //Serial.println(dataString);
   
    String file = now.toString(buf2);
    File dataFile = SD.open("/datalog.txt", FILE_APPEND);
    
   // if the file is available, write to it:
   if (dataFile) 
   {
     //digitalWrite(LED_yellow, HIGH);  // Blink for Dasa
      dataFile.println(dataString);  // write to SDcard (800 ms) 
      dataFile.println("\n");//Zeilenumbruch in Datei wenn nicht Funktioniert in Zeile 232    
      //digitalWrite(LED_yellow, LOW);          
     dataFile.close();
     Serial.println("Loop");

     
    
     //***********************************************
     //Buffer leeren
     //for(int n=0; n<CHANNELS; n++)
      //{
      //histogram[n]=0;
      //}
    
    
     }  
    // if the file isn't open, pop up an error:
    else 
    {
      Serial.println("#error opening datalog.txt");
    }
    //readFile(SD,"/datalog.txt");
    //testFileIO(SD,"/datalog.txt");
    /*    
    File dataFile = writeFile(SD,"/datalog.txt", dataString);

    if (dataFile) 
    {
     appendFile(SD,"/datalog.txt",dataString);
    }
    */              
    i=0;
  }
}
