// vim: filetype=arduino
void saveProgress(String startTimeStr, unsigned int totalDistance, unsigned long effectiveTime, boolean erase=false) {

    unsigned int time = effectiveTime / 1000UL;
    int str_size = startTimeStr.length();

    // To indicate a save is present
    if (erase) {
        EEPROM.write(0, 0);
    } else {
        EEPROM.write(0, 1);
    }

    EEPROM.write(1, highByte(time));
    EEPROM.write(2, lowByte(time));
    EEPROM.write(3, highByte(totalDistance));
    EEPROM.write(4, lowByte(totalDistance));
    EEPROM.write(5, str_size);

    for (int i = 0;i < str_size;i++)
    {
        EEPROM.write(6 + i, startTimeStr.charAt(i));
    }

}

void eraseProgress() {
    saveProgress("", 0, 0, true);
}

boolean savePresent() {
    return (EEPROM.read(0) > 0);
}

unsigned long getSavedTime() {
    unsigned int time;
    time = word(EEPROM.read(1), EEPROM.read(2));
    return time * 1000UL;
}

unsigned int getSavedDistance() {
    return word(EEPROM.read(3), EEPROM.read(4));
}

String getSavedStartTimeStr() {
    int str_size = EEPROM.read(5);
    String str_time = "";
    for (int i = 0;i < str_size;i++)
    {
        str_time += char(EEPROM.read(6 + i));
    }
    return str_time;
}
