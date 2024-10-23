/* Chick coop door
 * Uses an Arduino Nano
 * to interface to a smart garage door controller 
 * Rev1.1 removed the need for Sync
 * Rev1.2 Fixed issue with reversing actuator direction
 * Rev1.3 Added external LEDs for power and door
 * Rev1.4 Fixed door close on startup
 * Rev1.5 Add ext LED outputs to PCB rev2.3
 */
#define VERSION "Chicken Coop Door 1.5\n"

//Custom pins
#define EXT_DOOR_LED 8
#define EXT_POWER_LED 9


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
#define FADE_RATE   50    // 20ms fade rate
#define MAX_BRIGHT  40    // PWM max brightness
#define MOTOR_DELAY 250   // Short delay so motor doesn't cause voltage drop

static bool door_open = false;     // Motor output state of door

void setup() 
{
  // Start the Serial comms
  Serial.begin(9600);         
  delay(10);
  Serial.println(VERSION);
  
  //Custom
  pinMode(EXT_POWER_LED, OUTPUT);
  pinMode(EXT_DOOR_LED, OUTPUT);
  
  //Pin Outputs
  pinMode(DOOR_STATUS_OUTPUT, OUTPUT);
  pinMode(LED_OUTPUT, OUTPUT);
  pinMode(MOTOR_CONTROL_1, OUTPUT);
  pinMode(MOTOR_CONTROL_2, OUTPUT);
  digitalWrite(MOTOR_CONTROL_1, HIGH);
  digitalWrite(MOTOR_CONTROL_2, LOW);

  //Delay 1 seconds for the garage sensor to start
  delay(1000);
  
  //Pin Inputs 
  pinMode(TOGGLER_INPUT, INPUT_PULLUP);
  pinMode(CLOSED_SENSOR_D, INPUT);
  pinMode(CLOSED_SENSOR_A, INPUT);
 
  // on startup, if door is closed then initilaize the motor to close
  if(digitalRead(CLOSED_SENSOR_D) == LOW)
  {
    digitalWrite(MOTOR_CONTROL_1, LOW);
    digitalWrite(MOTOR_CONTROL_2, LOW);
    digitalWrite(DOOR_STATUS_OUTPUT, LOW);
    door_open = true;
    Serial.println("Door is already OPEN");
  }
  else
  {
    digitalWrite(MOTOR_CONTROL_1, HIGH);
    digitalWrite(MOTOR_CONTROL_2, HIGH);
    digitalWrite(DOOR_STATUS_OUTPUT, HIGH);
    door_open = false;
    Serial.println("Door is already CLOSED");
  }

  
}

void fade_led()
{
  static unsigned long fade_time = 0;
  static int pwm = 0;
  

  if(millis() - fade_time > FADE_RATE)
  {
    fade_time = millis();

    pwm += 1;
    int pwm_out = pwm;

    if(pwm > MAX_BRIGHT && pwm < (MAX_BRIGHT * 2))
    {
      pwm_out = MAX_BRIGHT - (pwm - MAX_BRIGHT);
    }
    else if(pwm >= (MAX_BRIGHT * 2))
    {
      pwm = 0;
      pwm_out = 0;
    }

    analogWrite(LED_OUTPUT, pwm_out); 
    analogWrite(EXT_POWER_LED, pwm_out); 
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
    
    //Analog input read of the door sensor (debug)
    Serial.println(analogRead(CLOSED_SENSOR_A));

    //indicate door state
    if(digitalRead(CLOSED_SENSOR_D) == HIGH)
    {
      digitalWrite(DOOR_STATUS_OUTPUT, HIGH);
      digitalWrite(EXT_DOOR_LED, HIGH);
      //analogWrite(EXT_DOOR_LED, MAX_BRIGHT);
      Serial.println("Door is CLOSED");
    }
    else
    {
      digitalWrite(DOOR_STATUS_OUTPUT, LOW);
      digitalWrite(EXT_DOOR_LED, LOW);
      //analogWrite(EXT_DOOR_LED, 0);
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
  fade_led();
    
  if(door_request())
  {
    toggle_door();
  }
}
