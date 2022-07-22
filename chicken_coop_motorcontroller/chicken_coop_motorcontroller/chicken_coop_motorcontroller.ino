/* Chick coop door
 * Uses an Arduino Nano
 * to interface to a smart garage door controller 
 * Rev1.1 Changed sensor polarity 
 */
#define VERSION "Chicken Coop Door motor controlled 1.1\n"

//Pin assignments
#define TOGGLER_INPUT PD2 
#define CLOSED_SENSOR_D PD3 
#define OPENED_SENSOR_D 8 // D8
#define MOTOR_CONTROL_1 PD4 
#define LED_OUTPUT PD5 
#define DOOR_STATUS_OUTPUT PD6
#define MOTOR_CONTROL_2 PD7 


// Change these if your switches are not aligned perfectly with the door positions
#define DELAY_AFTER_OPENED 1000 //milli second delay after open trigger
#define DELAY_AFTER_CLOSED 1000 //milli second delay after close trigger

#define DEBOUNCE          1500  // 1500 milli second for toggle switch
#define SLOW_BLINK_RATE   1000  // 1 second blink rate for heartbeat
#define FAST_BLINK_RATE   100   // 100ms blink rate for unknown door state
#define MOTOR_DELAY       250   // Short delay so motor doesn't cause voltage drop

typedef enum {NONE, REQUEST_OPEN, REQUEST_CLOSE, OPENED, CLOSED} DOOR_STATES;

static DOOR_STATES door_state = NONE;  // Motor output state of door
static long door_delay_timer = 0;      // timer for delay the motor stop after open or closed feedback


void blink_led()
{
  static unsigned long blink_time = 0;
  static bool blink = true;

  if(millis() - blink_time > SLOW_BLINK_RATE)
  {
    blink_time = millis();
    blink = !blink;
    digitalWrite(LED_OUTPUT, blink ? HIGH : LOW);

    Serial.print("Open Sensor: ");
    Serial.println(digitalRead(OPENED_SENSOR_D));

    Serial.print("Close Sensor: ");
    Serial.println(digitalRead(CLOSED_SENSOR_D));

    Serial.print("Door State: ");
    Serial.println(door_state);
    
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

void stop_door(bool immediate = false)
{
  bool stop_motor = false;
  
  if(immediate)
  {
    door_state = NONE;
    stop_motor = true;
  }
  else
  {
    if(door_state == REQUEST_CLOSE && millis() - door_delay_timer >= DELAY_AFTER_CLOSED)
    {
      Serial.println("Door closed");
      door_state = CLOSED;
    }
    
    if(door_state == REQUEST_OPEN && millis() - door_delay_timer >= DELAY_AFTER_OPENED)
    {
      Serial.println("Door opened");
      door_state = OPENED;
    }

    if(door_state == OPENED || door_state == CLOSED)
    {
       stop_motor = true;
    }
  }

  // stop state should be both leads to the motor grounded
  if(stop_motor)
  {
    digitalWrite(MOTOR_CONTROL_1, HIGH);
    digitalWrite(MOTOR_CONTROL_2, LOW);
  }
  
}

void operate_door()
{
  static unsigned long blink_time = 0;
  static bool blink = true;
  
  //indicate door state is closed
  if(digitalRead(OPENED_SENSOR_D) == HIGH && 
     digitalRead(CLOSED_SENSOR_D) == LOW )
  {
    //On startup, if the door is closed then set it to closed without delay
    if(door_state == NONE)
       door_state = CLOSED;

    if(door_state == REQUEST_CLOSE)
      stop_door();
  }

  //indicate door state is opened
  else if(digitalRead(OPENED_SENSOR_D) == LOW &&
          digitalRead(CLOSED_SENSOR_D) == HIGH) 
          
  {
    //On startup, if the door is open then set it to open without delay
    if(door_state == NONE)
      door_state = OPENED;

    if(door_state == REQUEST_OPEN)
      stop_door();
  }
  
  //Door state is closed and opened, something is wrong, stop motor
  else if(digitalRead(OPENED_SENSOR_D) == LOW &&
          digitalRead(CLOSED_SENSOR_D) == LOW)
  {
    stop_door(true);
    if(millis() - blink_time > SLOW_BLINK_RATE)
    {
      blink_time = millis();
      blink = !blink;
      digitalWrite(DOOR_STATUS_OUTPUT, blink ? HIGH : LOW);
      Serial.println("Door Feedback Error!");
    }
  }
  
  //Door state is moving
  else  
  {
    //On startup, if the door is not opened or closed then open it
    if(door_state == NONE)
      door_state = REQUEST_OPEN;
      
    //update delay timer
    door_delay_timer = millis();
    
    if(millis() - blink_time > FAST_BLINK_RATE)
    {
      blink_time = millis();
      blink = !blink;
      digitalWrite(DOOR_STATUS_OUTPUT, blink ? HIGH : LOW);
    }
  }

  if(door_state == CLOSED)
      digitalWrite(DOOR_STATUS_OUTPUT, HIGH);

  if(door_state == OPENED)
      digitalWrite(DOOR_STATUS_OUTPUT, LOW);
  
  if(door_state == REQUEST_OPEN)
  {
    digitalWrite(MOTOR_CONTROL_1, HIGH);
    digitalWrite(MOTOR_CONTROL_2, HIGH);
  }

  if(door_state == REQUEST_CLOSE)
  {
    digitalWrite(MOTOR_CONTROL_1, LOW);
    digitalWrite(MOTOR_CONTROL_2, LOW);
  }
}

void toggle_door()
{
  switch(door_state)
  {
    case OPENED:
      door_state = REQUEST_CLOSE;
      Serial.println("Door is closing");
      break;
    case CLOSED:
      door_state = REQUEST_OPEN;
      Serial.println("Door is opening");
    break;
    case REQUEST_CLOSE:
      stop_door(true);
      delay(MOTOR_DELAY);
      door_state = REQUEST_OPEN;
      Serial.println("Door is opening");
      break;
    case REQUEST_OPEN:
      stop_door(true);
      delay(MOTOR_DELAY);
      door_state = REQUEST_CLOSE;
      Serial.println("Door is closing");
      break;
    default: 
      stop_door(true);
      Serial.println("Door request unknown");
      break;
  }
}

void setup() 
{
  //Pin Outputs
  pinMode(DOOR_STATUS_OUTPUT, OUTPUT);
  pinMode(LED_OUTPUT, OUTPUT);
  pinMode(MOTOR_CONTROL_1, OUTPUT);
  pinMode(MOTOR_CONTROL_2, OUTPUT);
  digitalWrite(LED_OUTPUT, HIGH);
  stop_door(true);
  
  //Pin Inputs 
  pinMode(TOGGLER_INPUT, INPUT_PULLUP);
  pinMode(CLOSED_SENSOR_D, INPUT);
  pinMode(OPENED_SENSOR_D, INPUT_PULLUP);
   
  // Start the Serial comms
  Serial.begin(9600);         
  delay(10);
  Serial.println(VERSION);

  //Delay 1 seconds for the garage sensor to start
  delay(1000);
}

void loop() 
{
  blink_led();
    
  if(door_request())
  {
    toggle_door();
  }
  operate_door();
}
