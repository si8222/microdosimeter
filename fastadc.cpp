#include <SPI.h>
#include "FS.h"
#include <SD.h>
#include "RTClib.h"
#include <driver/i2s.h>
const int chipSelect = SS;

#define I2S_SAMPLE_RATE (277777) // Max sampling frequency = 277.777 kHz

#define ADC_INPUT (ADC1_CHANNEL_4) //pin 32

#define I2S_DMA_BUF_LEN (1024)
#define AVERAGE_EVERY_N_SAMPLES (5)


#define CHANNELS 512 // number of channels in buffer for histogram, including negative numbers 9-Bit

#define RESET 27// SD2


void i2sInit(){

i2s_config_t i2s_config = {

.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),

.sample_rate = I2S_SAMPLE_RATE, // The format of the signal using ADC_BUILT_IN

.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // is fixed at 12bit, stereo, MSB

.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,

.communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),

.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,

.dma_buf_count = 8,

.dma_buf_len = I2S_DMA_BUF_LEN,

.use_apll = false,

.tx_desc_auto_clear = false,

.fixed_mclk = 0

};

if(ESP_OK != i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL)){

Serial.printf("Error installing I2S. Halt!");

while(1);

}

if(ESP_OK != i2s_set_adc_mode(ADC_UNIT_1, ADC_INPUT)){

Serial.printf("Error setting up ADC. Halt!");

while(1);

}

if(ESP_OK != adc1_config_channel_atten(ADC_INPUT, ADC_ATTEN_DB_6)){

Serial.printf("Error setting up ADC attenuation. Halt!");

while(1);

}



if(ESP_OK != i2s_adc_enable(I2S_NUM_0)){

Serial.printf("Error enabling ADC. Halt!");

while(1);

}

Serial.printf("I2S ADC setup ok\n");

}


RTC_DS3231 rtc;


uint16_t u_sensor, maximum=0,threshold=10, i=0,j=0,signal0=0,signal1=0,maxi=0,highest=0;

uint16_t histogram[CHANNELS];





void setup() 

{

i2sInit();
for(int n=0; n<CHANNELS; n++)
{

histogram[n]=0;
}


Serial.begin(115200);

Serial.println("Start");

pinMode(RESET, OUTPUT); // reset for peak detetor

digitalWrite(RESET, LOW);

pinMode(RESET, INPUT);



//

pinMode(RESET, OUTPUT); // to start Reset on High

digitalWrite(RESET, HIGH);



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
size_t bytes_read;

uint16_t buffer[I2S_DMA_BUF_LEN] = {0};

DateTime now = rtc.now();
pinMode(RESET,INPUT);//in der schleife die ganze Zeit im Input
signal0=i2s_read(I2S_NUM_0, &buffer, sizeof(buffer), &bytes_read, 15);

//Serial.println(signal);

if (signal0>1)//wenn signal größer, dann nehme den größten aus 10 messwerten auf und reset

{
maximum=signal0;

for (int n=0; n<5 ;n++)

{

signal0=i2s_read(I2S_NUM_0, &buffer, sizeof(buffer), &bytes_read, 15);//neu weil bei jedem durchgang neu ausgemessen werden soll

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

if(i>1000)//anzahl der aufgenommenen Werte oder umrechnen auf Zeit
{
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