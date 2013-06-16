// vim: filetype=arduino
/*
 * Cycling Pusher Runkeeper lib
 *
 *
 *  Created on: 5.03.2012
 *      Author: reefab
 */

#define server "api.runkeeper.com"
#define status_inprogress "Uploading result"
#define status_failure "failed to connect"
#define status_data_uploaded "Data uploaded"
#define status_session_created "Session Created"
#define status_error_code "ERROR: Server returned "



boolean uploadResult(String startTimeStr, unsigned int totalDistance, unsigned long effectiveTime)
{
    if (client.connect(server, 80)) {
        Lcd.infoMessage(status_inprogress);
        Serial.println(F("connected"));
        client.print(F("POST /fitnessActivities HTTP/1.1\nHost: api.runkeeper.com\nContent-Type: application/vnd.com.runkeeper.NewFitnessActivity+json\nUser-Agent: Arduino/1.0\nAuthorization: "));
        client.println(accessToken);
        client.println();
        client.print(F("{\"type\": \"Cycling\", \"equipment\": \"Stationary Bike\", \"start_time\": \""));
        client.println(startTimeStr);
        client.print(F("\", \"total_distance\": "));
        client.println(totalDistance);
        client.print(F("duration\": "));
        client.println((int) (effectiveTime / 1000UL));
        client.print("}");
        delay(2000);
    } else {
        Serial.println(F("Conn. Failed"));
        Lcd.errorMessage(status_failure);
        client.stop();
        delay(1000);
    }
    Lcd.clear();

    // Extract status code to see if POST was succesful
    // Borrowed from https://github.com/interactive-matter/HTTPClient
    const char* statusPrefix = "HTTP/*.* ";
    const char* statusPtr = statusPrefix;
    char c = '\0';
    int statusCode = 0;
    int iState = 0;
    Serial.println();
    while (client.available()) {
      c = client.read();
      Serial.print(c);
      if (c != -1) {
        switch(iState) {
            case 0:
                // We haven't reached the status code yet
                if ( (*statusPtr == '*') || (*statusPtr == c) ) {
                    // This character matches, just move along
                    statusPtr++;
                    if (*statusPtr == '\0'){
                        // We've reached the end of the prefix
                        iState = 1;
                    }
                } else {
                    Lcd.errorMessage(status_failure);
                }
                break;
            case 1:
                if (isdigit(c)) {
                    // This assumes we won't get more than the 3 digits we
                    // want
                    statusCode = statusCode*10 + (c - '0');
                } else {
                    // We've reached the end of the status code
                    // We could sanity check it here or double-check for ' '
                    // rather than anything else, but let's be lenient
                    iState = 2;
                }
                break;
            case 2:
                // We're just waiting for the end of the line now
                break;
            };
        }
    }
    Serial.println();

    if (!client.connected()) {
        client.stop();
    }

    if (statusCode == 201) {
        Lcd.infoMessage(status_data_uploaded);
        Lcd.setSecondLine(status_session_created);
        Serial.println(status_session_created);
        return true;
    } else {
        Lcd.errorMessage(status_error_code);
        Lcd.setSecondLine((String) statusCode);
        Serial.print(F("Error code: "));
        Serial.println((String) statusCode);
        return false;
    }
}
