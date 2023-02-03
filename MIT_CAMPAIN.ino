

#include <SPI.h>
#include "FS.h"
#include <SD.h>
#include "RTClib.h"

const int chipSelect = SS;



#include "driver/adc.h"

#include "esp_adc_cal.h"

#define ADCPIN 27

#define CHANNELS 512 // number of channels in buffer for histogram, including negative numbers 9-Bit

#define RESET 32// SD2

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}


void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}


void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}


RTC_DS3231 rtc;


uint16_t u_sensor, maximum=0,threshold=10, i=0,j=0,signal0=0,signal1=0,maxi=0,highest=0;

uint16_t histogram[CHANNELS];





void setup() 

{


for(int n=0; n<CHANNELS; n++)
{

histogram[n]=0;
}


Serial.begin(115200);

analogSetWidth(9);

analogSetAttenuation(ADC_2_5db);
adc_set_clk_div(2);

Serial.println("Start");

pinMode(RESET, OUTPUT); // reset for peak detetor

digitalWrite(RESET, LOW);

pinMode(RESET, INPUT);



//

pinMode(RESET, OUTPUT); // to start Reset on High

digitalWrite(RESET, HIGH);

Serial.println(millis());



////

if (!rtc.begin()) {
Serial.println("Couldn't find RTC");
while (1);
}

//rtc.adjust(DateTime(__DATE__, __TIME__));

if(SD.begin(SS))

{
Serial.println("Karte gefunden.\n");
}

if 
(!SD.begin(SS))
 {
  Serial.println("Karte nicht gefunden!\r");
  return;
 }
}

void loop()
{
DateTime now = rtc.now();
pinMode(RESET,INPUT);//in der schleife die ganze Zeit im Input
signal0=analogRead(ADCPIN);

//Serial.println(signal);

if (signal0>1)//wenn signal größer, dann nehme den größten aus 10 messwerten auf und reset

{
maximum=signal0;

for (int n=0; n<10 ;n++)

{

signal0=analogRead(ADCPIN);//neu weil bei jedem durchgang neu ausgemessen werden soll

if(signal0>maximum)

{ 

maximum=signal0; 

} 
} //soviele Werte wie länge des pulses speichern

//speichere x Werte und suche den hächsten

//Serial.println(maximum);

histogram[maximum]++;
//Serial.println(maximum);
maximum=0;
i++;

//Serial.println(i);

pinMode(RESET, OUTPUT); // reset for peak detetor

digitalWrite(RESET, LOW);

digitalWrite(RESET,HIGH);

delay(10);

}



//test ausgabe histogramm
if (i==1)
{
Serial.println("Start:");
Serial.println(millis());
}
if(i>1000)//anzahl der aufgenommenen Werte

{
Serial.println("Ende:");
Serial.println(millis());

/*for(int j=0; j < CHANNELS; j++)

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
for(int n=0; n<CHANNELS; n++)  
    {
      dataString += ",";
      dataString += String(histogram[n]); 
    }
//Serial.println(dataString);



File dataFile = SD.open("/datalog.txt", FILE_WRITE);
    
      // if the file is available, write to it:
      if (dataFile) 
      {
        //digitalWrite(LED_yellow, HIGH);  // Blink for Dasa
        dataFile.println(dataString);  // write to SDcard (800 ms)     
        //digitalWrite(LED_yellow, LOW);          
        dataFile.close();

        for(int n=0; n<CHANNELS; n++)
        {

          histogram[n]=0;
        }   
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
