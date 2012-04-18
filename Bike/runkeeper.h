/*
 * Cycling Pusher Runkeeper lib
 *
 *
 *  Created on: 5.03.2012
 *      Author: reefab
 */

byte serverIp[] = {
  74,50,63,142};
#define apiServer "api.runkeeper.com"
#define apiUri "/fitnessActivities"
#define json_str1 "{\"type\": \"Cycling\",\"start_time\": \""
#define json_str2 "\",\"total_distance\": "
#define json_str3 ",\"duration\": "
#define header_str1 "Authorization"
#define header_str2 "Content-Type"
#define header_str3 "application/vnd.com.runkeeper.NewFitnessActivity+json"
#define status_inprogress "Uploading result"
#define status_failure "failed to connect"
#define status_data_uploaded "Data uploaded"
#define status_session_created "Session Created"
#define status_error_code "ERROR: Server returned "

HTTPClient http_client(apiServer, serverIp);

boolean uploadResult(String startTimeStr, unsigned int totalDistance, unsigned long effectiveTime)
{
  String data; 
  data += json_str1;
  data += startTimeStr;
  data += json_str2;
  data += (int) totalDistance;
  data += json_str3;
  data += (int) (effectiveTime / 1000UL);
  data += "}";
  unsigned int bufSize = data.length() +1;
  char apiData[bufSize];
  data.toCharArray(apiData, bufSize);

  http_client_parameter apiHeaders[] = {
    { 
      header_str1, accessToken          }
    ,
    { 
      header_str2, header_str3      }
    ,
    {
      NULL, NULL          }
  };
  lcd.print(status_inprogress);
  delay(500);
  http_client.debug(-1);
  FILE* result = http_client.postURI(apiUri, NULL, apiData, apiHeaders);
  int returnCode = http_client.getLastReturnCode();
  lcd.clear();
  if (result!=NULL) {
    http_client.closeStream(result);  // this is very important -- be sure to close the STREAM
  } 
  else {
    lcd.print(status_failure);
  }
  if (returnCode==201) {
    lcd.print(status_data_uploaded);
    lcd.setCursor(0, 1);
    lcd.print(status_session_created);
    return true;
  } 
  else {
    lcd.print(status_error_code);
    lcd.setCursor(0, 1);
    lcd.print(returnCode);
    return false;
  }
}

