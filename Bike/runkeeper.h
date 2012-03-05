/*
 * Cycling Pusher Runkeeper lib
 *
 *
 *  Created on: 5.03.2012
 *      Author: reefab
 */

#include <Arduino.h>
#include <HTTPClient.h>

byte serverIp[] = {
  74,50,63,142};
#define apiServer "api.runkeeper.com"
#define apiUri "/fitnessActivities"

HTTPClient http_client(apiServer, serverIp);

boolean uploadResult(String startTimeStr, unsigned int totalDistance, unsigned int effectiveTime)
{
    String data = "{\"type\": \"Cycling\",\"start_time\": \"";
    data += startTimeStr;
    data += "\",\"notes\": \"Arduino powered stationary bike\",\"total_distance\": ";
    data += totalDistance;
    data += ",\"duration\": ";
    data += (int) (effectiveTime / 1000);
    data += "}";
    unsigned int bufSize = data.length() +1;
    char apiData[bufSize];
    data.toCharArray(apiData, bufSize);

    http_client_parameter apiHeaders[] = {
      { 
        "Authorization", accessToken      }
      ,
      { 
        "Content-Type", "application/vnd.com.runkeeper.NewFitnessActivity+json"      }
      ,
      {
        NULL, NULL      }
    };
    lcd.print("Uploading result");
    delay(500);
    FILE* result = http_client.postURI(apiUri, NULL, apiData, apiHeaders);
    int returnCode = http_client.getLastReturnCode();
    lcd.clear();
    if (result!=NULL) {
      http_client.closeStream(result);  // this is very important -- be sure to close the STREAM
    } 
    else {
      lcd.print("failed to connect");
    }
    if (returnCode==201) {
      lcd.print("Data uploaded");
      lcd.setCursor(0, 1);
      lcd.print("Session Created");
      return true;
    } 
    else {
      lcd.print("ERROR: Server returned ");
      lcd.setCursor(0, 1);
      lcd.print(returnCode);
      return false;
    }
}
