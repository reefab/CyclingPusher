// vim: set ft=arduino:
// Number of 'wheel' turns needed to recalculate speed/total distance
#define INTERVAL 5
// Minimal number of millisecond between hall switch changes to prevent bounce
#define HALL_RES 100
// Timeout in seconds, if the hall switch is not activated during this time, pause everything
#define TIMEOUT 15
// Minimum distance for a valid activity (in meters)
#define MIN_DISTANCE 500
// least amount of effective time for a valid activity (in secs)
#define MIN_TIME 60
// Amount of inactive time before either automatic upload or discarding of current session
#define MAX_TIME 60
// Meters "travelled" by pedal turns (if the hall sensor is on the pedal)
#define METER_PER_TURN 6
// Time in seconds before the backlight of the LCD is switched off if there is no activity
/* #define DISPLAYSLEEP 60 */
// Change the data displayed on the second line of the lcd every X seconds
/* #define CHANGESECONDlINE 3 */
// Save session data every X seconds
/* #define SAVEiNTERVAL 120 */
// Beep shortly every X meters
/* #define beepInterval 5000 */
// For the breakout, you can use any 2 or 3 pins
// These pins will also work for the 1.8" TFT shield
#define TFT_CS     D2
#define TFT_RST    0  // you can also connect this to the Arduino reset
// in which case, set this #define pin to 0!
#define TFT_DC     D3
// TFT rotation
#define TFT_ROTATION 3
// Pin for the hall sensor
#define HALL_PIN D0
// Diag LEd
#define DIAG_LED D4
