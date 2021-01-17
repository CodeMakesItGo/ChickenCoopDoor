/* Chick coop door
 * Uses an Arduino Nano
 * to interface to a smart garage door controller 
 * https://amzn.to/3ikQDJ0
 */
#define VERSION "Chicken Coop Door 1.0\n"

//Pin assignments
#define TOGGLER PD2 
#define CLOSED_SENSOR PD3 
#define MOTOR_CONTROL PD4 
#define LED_OUTPUT PD5 

#define SYNC_TIME   30000 // sync with door sensor for truth
#define DEBOUNCE    1500  // 1500 milli second
#define BLINK_RATE  1000  // 1 second blink rate

static bool door_open = false;
static bool blink = true;
static unsigned long sync_time = 0;

void setup() 
{
  // Start the Serial comms
  Serial.begin(9600);         
  delay(10);
  Serial.println(VERSION);

  //Pin Setup 
  pinMode(TOGGLER, INPUT_PULLUP);
  pinMode(CLOSED_SENSOR, INPUT_PULLUP);
  pinMode(LED_OUTPUT, OUTPUT);
  pinMode(MOTOR_CONTROL, OUTPUT);
  digitalWrite(LED_OUTPUT, HIGH);
  digitalWrite(MOTOR_CONTROL, HIGH);

  //Delay 2 seconds for the garage sensor to start
  //Actuator will start to close the door for 2 seconds if it is open
  // but will switch back to open within this time. This is to midigate if
  // there is a reset while the door is closed. 
  delay(2000);
 
  //if door is closed then initilaize to closed
  if(digitalRead(CLOSED_SENSOR) == LOW)
  {
    digitalWrite(MOTOR_CONTROL, HIGH);
    door_open = false;
    Serial.println("Door is CLOSED");
  }
  else
  {
    digitalWrite(MOTOR_CONTROL, LOW);
    door_open = true;
    Serial.println("Door is OPEN");
  }
}

void blink_led()
{
  static unsigned long blink_time = 0;

  if(millis() - blink_time > BLINK_RATE)
  {
    blink_time = millis();
    blink = !blink;
    digitalWrite(LED_OUTPUT, blink ? HIGH : LOW);

    Serial.println(String(digitalRead(CLOSED_SENSOR)));
  }
}

bool door_request()
{
  static unsigned long timer = 0;

  bool rtn = false;
  if(digitalRead(TOGGLER) == LOW)
  {
    if(millis() - timer > DEBOUNCE)
    {
      rtn = true;
      Serial.println("Request received");
      timer = millis();
    }
  }
  else
  {
     timer = millis();
  }

  return rtn;
}

/// We only want to sync the door state when it is not moving
/// Because there are two controllers keeping state of the door
/// we want to be able to sync to the truth every once in a while
void sync_door_state()
{
  if(millis() - sync_time > SYNC_TIME)
  {
    sync_time = millis();
    
    if(digitalRead(CLOSED_SENSOR) == LOW)
    {
      door_open = false;
      Serial.println("Sync to close");
    }
    else
    {
      door_open = true;
      Serial.println("Sync to open");
    }
  }
}

void toggle_door()
{
  door_open = !door_open;
  
  digitalWrite(MOTOR_CONTROL, door_open ? LOW : HIGH);
  
  Serial.println("Door is " + String(door_open ? "opening" : "closing"));
}

void loop() 
{
  blink_led();
  sync_door_state();
  
  if(door_request())
  {
    //Ignore sync for the next 30 seconds while moving
    sync_time = millis();
    toggle_door();
  }
}
