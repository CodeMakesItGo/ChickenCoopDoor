/* Chick coop door
 * Uses an Arduino Nano
 * to interface to a smart garage door controller 
 * Rev1.1 removed the need for feedback because the garage door app will sync the door location
 */
#define VERSION "Chicken Coop Door 1.1\n"

//Pin assignments
#define TOGGLER_INPUT PD2 
#define MOTOR_CONTROL PD4 
#define LED_OUTPUT PD5 

#define DEBOUNCE    1500  // 1500 milli second
#define BLINK_RATE  1000  // 1 second blink rate

static bool door_open = false;     // Motor output state of door

void setup() 
{
  // Start the Serial comms
  Serial.begin(9600);         
  delay(10);
  Serial.println(VERSION);

  //Pin Setup 
  pinMode(TOGGLER_INPUT, INPUT_PULLUP);
  pinMode(LED_OUTPUT, OUTPUT);
  pinMode(MOTOR_CONTROL, OUTPUT);
  digitalWrite(LED_OUTPUT, HIGH);
  digitalWrite(MOTOR_CONTROL, HIGH);
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
