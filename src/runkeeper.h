// vim: filetype=arduino
/*
 * Cycling Pusher Runkeeper lib
 *
 *
 *  Created on: 5.03.2012
 *      Author: reefab
 */

IPAddress api_server;

boolean uploadResult(String startTimeStr, unsigned int totalDistance, unsigned long effectiveTime)
{
    Dns.getHostByName("api.runkeeper.com", api_server);
    Serial.println(api_server);

    while(!client.connected()) {
        Lcd.infoMessage("Connecting");
        client.connect(api_server, 80);
        delay(1000);
    }

    String duration = String(effectiveTime / 1000UL);
    String distance = String(totalDistance);
    Lcd.infoMessage("Uploading result");
    delay(500);
    client.print(F("POST /fitnessActivities HTTP/1.1\nHost: api.runkeeper.com\nContent-Type: application/vnd.com.runkeeper.NewFitnessActivity+json\nUser-Agent: Arduino/1.0\nAuthorization: "));
    client.println(accessToken);
    client.print(F("Content-Length: "));
    client.println(103 + startTimeStr.length() + distance.length() + duration.length());
    client.println();
    client.print(F("{\"type\": \"Cycling\", \"equipment\": \"Stationary Bike\", \"start_time\": \""));
    client.print(startTimeStr);
    client.print(F("\", \"total_distance\": "));
    client.print(distance);
    client.print(F(", \"duration\": "));
    client.print(duration);
    client.print("}");
    delay(2000);

    // Extract status code to see if POST was succesful
    // Borrowed from https://github.com/interactive-matter/HTTPClient
    const char* statusPrefix = "HTTP/*.* ";
    const char* statusPtr = statusPrefix;
    char c = '\0';
    int statusCode = 0;
    int iState = 0;
    while (client.connected()) {
      if (client.available()) {
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
                        Lcd.errorMessage("error");
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
                /*case 2:*/
                /*    // We're just waiting for the end of the line now*/
                /*    break;*/
                };
            }
        }
    }
    /*Lcd.infoMessage("Disconnecting");*/
    client.stop();

    if (statusCode == 201) {
        Lcd.infoMessage("Data uploaded");
        return true;
    } else {
        Lcd.errorMessage("Error: ");
        Lcd.setSecondLine((String) statusCode);
        Serial.println((String) statusCode);
        return false;
    }
}
