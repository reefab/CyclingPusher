#include <EEPROM.h>

void saveProgress(String startTimeStr, unsigned int totalDistance, unsigned long effectiveTime, unsigned int offset=1) {

  unsigned int time = effectiveTime / 1000UL;
  int str_size = startTimeStr.length();

  // To indicate a save is present
  EEPROM.write(0, 1);

  EEPROM.write(offset, highByte(time));
  EEPROM.write(offset + 1, lowByte(time));
  EEPROM.write(offset + 2, highByte(totalDistance));
  EEPROM.write(offset + 3, lowByte(totalDistance));
  EEPROM.write(offset + 4, str_size);

  for (int i = 0;i <= str_size;i++) 
  {
    EEPROM.write(offset + 5 + i, startTimeStr.charAt(i));
  }

}

void eraseProgress(unsigned int offset=1) {  
  EEPROM.write(offset, 0);
  EEPROM.write(offset + 1, 0);
  EEPROM.write(offset + 2, 0);
  EEPROM.write(offset + 3, 0);
  int str_size = EEPROM.read(offset + 4);
    EEPROM.write(offset + 4, 0);

  for (int i = 0;i <= str_size;i++) 
  {
    EEPROM.write(offset + 5 + i, 0);
  }
  // Finaly remove the byte that indicate a save is present
  EEPROM.write(0, 0);
}

boolean savePresent() {
  if (EEPROM.read(0) > 0) {
    return true;
  } 
  else {
    return false;
  }  
}

unsigned long getSavedTime(unsigned int offset=1) {
  unsigned int time;
  time = word(EEPROM.read(offset), EEPROM.read(offset + 1));
  return time * 1000UL;
}

unsigned int getSavedDistance(unsigned int offset=1) {
  return word(EEPROM.read(offset + 2), EEPROM.read(offset + 3));
}

String getSavedStartTimeStr(unsigned int offset=1) {
  int str_size = EEPROM.read(offset + 4);
  String str_time;
  for (int i = 0;i <= str_size;i++) 
  {
    str_time += char(EEPROM.read(offset + 5 + i));
  }
  return str_time;
}


