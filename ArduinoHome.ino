#include <Wire.h>
#include "HTU21D.h"
#include <EEPROM.h>


HTU21D myHumidity;

void setup()
{
  Serial.begin(9600);

  myHumidity.begin();
}

void loop()
{
   delay(1000);
   createHistory(); 
  
  if(Serial.available() > 0){
    String str =  Serial.readStringUntil('\n');
    str.toLowerCase();
  
    if (str == "info")
      sendInfo();
    else if (str == "history")  
      sendHistory();
    else if(str.startsWith("set mint ")) {
      str.remove(0, 9);
      setMinTemp(str.toInt());
    }
    else if(str.startsWith("set maxt ")) {
      str.remove(0, 9);
      setMaxTemp(str.toInt());
    }
    else if(str.startsWith("set minh ")) {
      str.remove(0, 9);
      setMinHum(str.toInt());
    }
    else if(str.startsWith("set maxh ")) {
      str.remove(0, 9);
      setMaxHum(str.toInt());
    }
  }

  
  
  
}

void sendHeader(){
  Serial.print("Arduino");
  Serial.print("<br>");
  Serial.print("------------------------");
  Serial.print("<br>");
}

void sendInfo(){
  float humd = myHumidity.readHumidity();
  float temp = myHumidity.readTemperature();

  sendHeader();
  
  Serial.print("  Temperature : ");
  Serial.print(temp, 1);
  Serial.print("C");
  Serial.print("<br>");
  Serial.print("  Humidity : ");
  Serial.print(humd, 1);
  Serial.print("%");
  
  Serial.print("<br>");
  Serial.print("<br>");
  Serial.print("Normal temperature : ");
  Serial.print(getMinTemp());
  Serial.print(" - ");
  Serial.print(getMaxTemp());
  Serial.print("<br>");
  Serial.print("Normal humidity : ");
  Serial.print(getMinHum());
  Serial.print(" - ");
  Serial.print(getMaxHum());

  Serial.println();
}

/*-----------------------------------------*/
// store min max
/*-----------------------------------------*/

int getMinTemp(){
  int res;
  EEPROM.get( 0, res );
  return res;
}

void setMinTemp(int val){
  EEPROM.put( 0, val );
}

int getMinHum(){
  int res;
  EEPROM.get( 2, res );
  return res;
}

void setMinHum(int val){
  EEPROM.put( 2, val );
}

int getMaxTemp(){
  int res;
  EEPROM.get( 4, res );
  return res;
}

void setMaxTemp(int val){
  EEPROM.put( 4, val );
}

int getMaxHum(){
  int res;
  EEPROM.get( 6, res );
  return res;
}

void setMaxHum(int val){
  EEPROM.put( 6, val );
}

/*-------------------------------------*/
// history & check
/*-------------------------------------*/
void sendChk(String ht, float crr, int chk){
   Serial.print("Current ");
   Serial.print(ht);
   Serial.print(" ( ");
   Serial.print(crr);
   if (crr < chk) 
      Serial.print(" ) is less than minimum ( ");
   else 
      Serial.print(" ) is more than maximum ( ");
   Serial.print(chk);
   Serial.print(" )");
   Serial.println();
}

unsigned long lastHistoryTime = 0;
byte historyHourCnt = 0;
float hourT = 0.0;
float hourH = 0.0;
int historyT[24] = {-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000};
int historyH[24] = {-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000};
float prevT = 0.0;
float prevH = 0.0;




void createHistory(){
 
   if (millis() - lastHistoryTime > 600000 /*10min*/){
    lastHistoryTime = millis();
    
    historyHourCnt++;

    float crrT = myHumidity.readTemperature();
    float crrH = myHumidity.readHumidity();

    //check
    int chkVal;
    chkVal = getMinTemp();
    if( !isnan(chkVal) &&  crrT < chkVal && prevT > chkVal)
       sendChk("temperature", crrT, chkVal);
    
    chkVal = getMaxTemp();
    if( !isnan(chkVal) &&  crrT > chkVal && prevT < chkVal)
       sendChk("temperature", crrT, chkVal);

    chkVal = getMinHum();
    if( !isnan(chkVal) &&  crrH < chkVal && prevH > chkVal)
       sendChk("humidity", crrT, chkVal);
    
    chkVal = getMaxHum();
    if( !isnan(chkVal) &&  crrH > chkVal && prevH < chkVal)
       sendChk("humidity", crrT, chkVal);
    
    prevT = crrT;
    prevH = crrH;
    
    // history    
    hourH = hourH + crrH;
    hourT = hourT + crrT;

   
    if (historyHourCnt == 6){
        historyHourCnt = 0;
      
      for (int i = 23; i > 0 ; i--){
        historyT[i] = historyT[i-1];
        historyH[i] = historyH[i-1];
      }
      
      historyT[0] = (int)(hourT/6.0 * 10);
      historyH[0] = (int)(hourH /6.0 * 10);

      hourT = 0.0;
      hourH = 0.0;
    }
   }
}

void sendHistory(){
  
    int minT = historyT[0];
    int minH = historyH[0];
    int maxT = historyT[0];
    int maxH = historyH[0];
    
    for (int i = 1; i < 24; i++){
      if(historyH[i] != -1000){
          if (minT > historyT[i])
            minT = historyT[i];
          if (minH > historyH[i])
            minH = historyH[i];
          if (maxT < historyT[i])
            maxT = historyT[i];
          if (maxH < historyH[i])
            maxH = historyH[i];
      }
    }
    
    float lenH100 = maxH - minH;
    if (lenH100 == 0)
      lenH100 = 1;
      
    float lenT100 = maxT - minT;
    if (lenT100 == 0)
      lenT100 = 1;

    sendHeader();
    
    Serial.print("-- Humidity  -- <br>");
    Serial.print(minH / 10.0);
    Serial.print(" - ");
    Serial.print(maxH / 10.0);
    Serial.print("<br>");
    for (int i = 0; i < 24; i++){
      if(historyH[i] != -1000){
        Serial.print(historyH[i]/10.0);
        Serial.print(' ');
        
        int lenBar = ((historyH[i] - minH) / lenH100) * 20.0;
        for (int k=0; k < lenBar; k++){
          Serial.print('#');
        }
      
        Serial.print("<br>");
      }  
    }
    
    Serial.print("<br>");
    Serial.print("-- Temperature  -- <br>");
    Serial.print(minT / 10.0);
    Serial.print(" - ");
    Serial.print(maxT / 10.0);
    Serial.print("<br>");
    for (int i = 0; i < 24; i++){
      if(historyH[i] != -1000){
        Serial.print(historyT[i] / 10.0);
        Serial.print(' ');
        
        int lenBar = ((historyT[i] - minT) / lenT100) * 20;
        for (int k=0; k < lenBar; k++){
           Serial.print("#");
        }
        
        Serial.print("<br>");
      }
    }

    Serial.println();
  
}