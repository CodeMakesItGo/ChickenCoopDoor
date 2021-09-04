/* Chick coop door
 * Uses an Arduino Nano
 * to interface to a smart garage door controller 
 * Rev1.1 removed the need for Sync
 */
#define VERSION "Chicken Coop Door 1.1\n"

//Pin assignments
#define TOGGLER_INPUT PD2 
#define CLOSED_SENSOR PD3 
#define MOTOR_CONTROL PD4 
#define LED_OUTPUT PD5 
#define DOOR_STATUS_OUTPUT PD6

#define DEBOUNCE    1500  // 1500 milli second
#define BLINK_RATE  1000  // 1 second blink rate

static bool door_open = false;     // Motor output state of door

void setup() 
{
  //Pin Outputs
  pinMode(DOOR_STATUS_OUTPUT, OUTPUT);
  pinMode(LED_OUTPUT, OUTPUT);
  pinMode(MOTOR_CONTROL, OUTPUT);
  digitalWrite(LED_OUTPUT, HIGH);
  digitalWrite(MOTOR_CONTROL, HIGH);
  
  //Pin Inputs 
  pinMode(TOGGLER_INPUT, INPUT_PULLUP);
  pinMode(CLOSED_SENSOR, INPUT_PULLUP);
  
  // Start the Serial comms
  Serial.begin(9600);         
  delay(10);
  Serial.println(VERSION);

  //Delay 2 seconds for the garage sensor to start
  //Actuator will start to close the door for 2 seconds if it is open
  // but will switch back to open within this time. This is to midigate if
  // there is a reset while the door is closed. 
  delay(2000);
 
  // on startup, if door is closed then initilaize the motor to close
  if(digitalRead(CLOSED_SENSOR) == LOW)
  {
    digitalWrite(MOTOR_CONTROL, HIGH);
    digitalWrite(DOOR_STATUS_OUTPUT, HIGH);
    door_open = false;
    Serial.println("Door is CLOSED");
  }
  else
  {
    digitalWrite(MOTOR_CONTROL, LOW);
    digitalWrite(DOOR_STATUS_OUTPUT, LOW);
    door_open = true;
    Serial.println("Door is OPEN");
  }
}

void blink_led()
{
  static unsigned long blink_time = 0;
  static bool blink = true;

  if(millis() - blink_time > BLINK_RATE)
  {
    blink_time = millis();
    blink = !blink;
    digitalWrite(LED_OUTPUT, blink ? HIGH : LOW);

    //indicate door state
    if(digitalRead(CLOSED_SENSOR) == LOW)
    {
      digitalWrite(DOOR_STATUS_OUTPUT, HIGH);
      Serial.println("Door is CLOSED");
    }
    else
    {
      digitalWrite(DOOR_STATUS_OUTPUT, LOW);
      Serial.println("Door is OPEN");
    }
  }
}

bool door_request()
{
  static unsigned long timer = 0;

  bool rtn = false;
  if(digitalRead(TOGGLER_INPUT) == LOW)
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

void toggle_door()
{
  door_open = !door_open;
  
  digitalWrite(MOTOR_CONTROL, door_open ? LOW : HIGH);
  
  Serial.println("Door is " + String(door_open ? "opening" : "closing"));
}

void loop() 
{
  blink_led();
    
  if(door_request())
  {
    toggle_door();
  }
}
