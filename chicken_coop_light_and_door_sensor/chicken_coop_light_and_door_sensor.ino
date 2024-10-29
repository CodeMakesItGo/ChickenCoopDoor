/* Chick coop door
 * Uses an Arduino Nano
 * to interface to a smart garage door controller 
 * Rev1.1 removed the need for Sync
 * Rev1.2 Fixed issue with reversing actuator direction
 * Rev1.3 Added external LEDs for power and door
 * Rev1.4 Fixed door close on startup
 * Rev1.5 Add ext LED outputs to PCB rev2.3
 */
#define VERSION "Chicken Coop Door Solar 1.5\n"

//Custom pins
#define EXT_DOOR_LED 10
#define EXT_POWER_LED 9

#define DOOR_SAFETY 13
#define PHOTO_SENSOR A5

//Pin assignments
#define TOGGLER_INPUT PD2 
#define CLOSED_SENSOR_D PD3 
#define MOTOR_CONTROL_1 PD4 
#define LED_OUTPUT PD5 
#define DOOR_STATUS_OUTPUT PD6
#define MOTOR_CONTROL_2 PD7 
#define CLOSED_SENSOR_A A1

#define PHOTO_COUNT 10    // photo sensor debounce
#define OBST_COUNT  10    // obstruction debounce 
#define RETRY_CLOSE 15000 // Since there is no open trigger, retry to close the door in 15 sec.
#define DEBOUNCE    1500  // 1500 milli second
#define BLINK_RATE  1000  // 1 second blink rate
#define FADE_RATE   50    // 20ms fade rate
#define MAX_BRIGHT  40    // PWM max brightness
#define MOTOR_DELAY 250   // Short delay so motor doesn't cause voltage drop

static bool door_open = false;     // Motor output state of door
static bool door_closing = false;
static bool day_light = false;     // Photo sensor
static bool door_obstuction = false; // Door safety

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
  pinMode(DOOR_SAFETY, INPUT_PULLUP);
  pinMode(PHOTO_SENSOR, INPUT_PULLUP);
 
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

  // on startup get the photo sensor state, default is false
  if(digitalRead(PHOTO_SENSOR) == HIGH)
  {
    day_light = true;
  }

  if(door_open != day_light)
  {
    Serial.println("Photo sensor says to toggle door.");
    toggle_door();
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
    //Serial.println(analogRead(CLOSED_SENSOR_A));

    //indicate door state
    if(digitalRead(CLOSED_SENSOR_D) == HIGH)
    {
      digitalWrite(DOOR_STATUS_OUTPUT, HIGH);
      digitalWrite(EXT_DOOR_LED, HIGH);
      //analogWrite(EXT_DOOR_LED, MAX_BRIGHT);
      if(door_open)
      {
        Serial.println("Door is CLOSED");
      }
      door_open = false;
    }
    else
    {
      digitalWrite(DOOR_STATUS_OUTPUT, LOW);
      digitalWrite(EXT_DOOR_LED, LOW);
      //analogWrite(EXT_DOOR_LED, 0);
      if(!door_open)
      {
        Serial.println("Door is OPEN");
      }
      door_open = true;
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

      if(door_closing)
      {
        door_open = !door_open;
        door_closing = false;
      }

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
  if(door_closing && door_obstuction)
  {
    door_open = !door_open;
  }

  digitalWrite(MOTOR_CONTROL_1, !door_open ? LOW : HIGH);
  delay(MOTOR_DELAY); 
  digitalWrite(MOTOR_CONTROL_2, !door_open ? LOW : HIGH);

  door_closing = door_open;
  
  Serial.println("Door is " + String(door_closing ? "closing" : "opening"));
}

void photo_sensor()
{
  static unsigned long timer = 0;
  static int photo_count = 0;

  if(door_obstuction)
  {
    // Ignore photo sensor while in door obstruction retry
    return;
  }

  if(digitalRead(PHOTO_SENSOR) != day_light)
  {
    photo_count++;
    if(photo_count > PHOTO_COUNT)
    {
      day_light = digitalRead(PHOTO_SENSOR);
      Serial.println("Photo sensor says it is " + String(day_light ? "day" : "night"));
      
      if(door_open != day_light)
      {
        toggle_door();  
      }
    }
  }
  else
  {
    photo_count = 0;
  }
}

void door_safety()
{
  static unsigned long timer = 0;
  static int obst_count = 0;
  
  // When the door is closed the sensor will be high, stop checking
  if(!door_obstuction && digitalRead(CLOSED_SENSOR_D) == LOW && door_closing)
  {
      if(digitalRead(DOOR_SAFETY) == LOW)
      {
        obst_count++;
        if(obst_count > OBST_COUNT)
        {
          obst_count = 0;
          door_obstuction = true;
          Serial.println("Door Obstruction!");
          toggle_door();
          timer = millis();
        }
      }
      else
      {
        obst_count = 0;
      }
  }
  else
  {
    door_closing = false;
    if(door_obstuction && (millis() - timer > RETRY_CLOSE))
    {
      Serial.println("Retry Close Door");
      door_obstuction = false;
      toggle_door();
    }
  }
}

void loop() 
{
  blink_led();
  fade_led();
  photo_sensor();
  door_safety();
    
  if(door_request())
  {
    toggle_door();
  }
}
