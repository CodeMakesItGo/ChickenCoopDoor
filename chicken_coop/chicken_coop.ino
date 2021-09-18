/* Chick coop door
 * Uses an Arduino Nano
 * to interface to a smart garage door controller 
 * Rev1.1 removed the need for Sync
 * Rev1.2 Fixed issue with reversing actuator direction
 */
#define VERSION "Chicken Coop Door 1.2\n"

//Pin assignments
#define TOGGLER_INPUT PD2 
#define CLOSED_SENSOR_D PD3 
#define MOTOR_CONTROL_1 PD4 
#define LED_OUTPUT PD5 
#define DOOR_STATUS_OUTPUT PD6
#define MOTOR_CONTROL_2 PD7 
#define CLOSED_SENSOR_A A1


#define DEBOUNCE    1500  // 1500 milli second
#define BLINK_RATE  1000  // 1 second blink rate
#define MOTOR_DELAY 250   // Short delay so motor doesn't cause voltage drop

static bool door_open = false;     // Motor output state of door

void setup() 
{
  //Pin Outputs
  pinMode(DOOR_STATUS_OUTPUT, OUTPUT);
  pinMode(LED_OUTPUT, OUTPUT);
  pinMode(MOTOR_CONTROL_1, OUTPUT);
  pinMode(MOTOR_CONTROL_2, OUTPUT);
  digitalWrite(LED_OUTPUT, HIGH);
  digitalWrite(MOTOR_CONTROL_1, LOW);
  digitalWrite(MOTOR_CONTROL_2, LOW);
  
  //Pin Inputs 
  pinMode(TOGGLER_INPUT, INPUT_PULLUP);
  pinMode(CLOSED_SENSOR_D, INPUT);
  pinMode(CLOSED_SENSOR_A, INPUT);
  
  // Start the Serial comms
  Serial.begin(9600);         
  delay(10);
  Serial.println(VERSION);

  //Delay 1 seconds for the garage sensor to start
  delay(1000);
 
  // on startup, if door is closed then initilaize the motor to close
  if(digitalRead(CLOSED_SENSOR_D) == LOW)
  {
    digitalWrite(MOTOR_CONTROL_1, HIGH);
    delay(MOTOR_DELAY);
    digitalWrite(MOTOR_CONTROL_2, HIGH);
    digitalWrite(DOOR_STATUS_OUTPUT, HIGH);
    door_open = false;
    Serial.println("Door is OPEN");
  }
  else
  {
    digitalWrite(MOTOR_CONTROL_1, LOW);
    delay(MOTOR_DELAY);
    digitalWrite(MOTOR_CONTROL_2, LOW);
    digitalWrite(DOOR_STATUS_OUTPUT, LOW);
    door_open = true;
    Serial.println("Door is CLOSED");
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

    //Analog input read of the door sensor (debug)
    Serial.println(analogRead(CLOSED_SENSOR_A));

    //indicate door state
    if(digitalRead(CLOSED_SENSOR_D) == HIGH)
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
  
  digitalWrite(MOTOR_CONTROL_1, door_open ? LOW : HIGH);
  delay(MOTOR_DELAY); 
  digitalWrite(MOTOR_CONTROL_2, door_open ? LOW : HIGH);
  
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
