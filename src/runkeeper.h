/*
 * Cycling Pusher Runkeeper lib
 *
 *
 *  Created on: 5.03.2012
 *      Author: reefab
 */

IPAddress server(74,50,63,142);
#define status_inprogress "Uploading result"
#define status_failure "failed to connect"
#define status_data_uploaded "Data uploaded"
#define status_session_created "Session Created"
#define status_error_code "ERROR: Server returned "

boolean uploadResult(String startTimeStr, unsigned int totalDistance, unsigned long effectiveTime)
{
    String data = "{";
    data +=
        "\"type\": \"Cycling\","
        "\"start_time\": \"" + startTimeStr + "\"," +
        "\"total_distance\": " + totalDistance + "," +
        "\"duration\":" +  (int) (effectiveTime / 1000UL) +
    "}";

    Serial.print(data);

    if (client.connect(server, 80)) {
        lcd.print(status_inprogress);
        client.println("POST /fitnessActivities HTTP/1.1");
        client.println("HOST: api.runkeeper.com");
        client.println("User-Agent: Arduino/1.0");
        client.print("Authorization: ");
        client.println(accessToken);
        client.println("Content-Type: application/vnd.com.runkeeper.NewFitnessActivity+json");
        client.print("Content-Length: ");
        client.println(data.length());
        client.println();
        client.println(data);
        delay(500);
        lcd.clear();
    } else {
        lcd.print(status_failure);
        delay(1000);
    }

    delay(3000);

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
                    lcd.print(status_failure);
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
        lcd.print(status_data_uploaded);
        lcd.setCursor(0, 1);
        lcd.print(status_session_created);
        return true;
    } else {
        lcd.print(status_error_code);
        lcd.setCursor(0, 1);
        lcd.print(statusCode);
        return false;
    }
}
