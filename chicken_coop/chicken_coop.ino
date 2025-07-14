/* Chick coop door
 * Uses an Arduino Nano
 * to interface to a smart garage door controller 
 * Rev1.1 removed the need for Sync
 * Rev1.2 Fixed issue with reversing actuator direction
 * Rev1.3 Added external LEDs for power and door
 * Rev1.4 Fixed door close on startup
 * Rev1.5 Add ext LED outputs to PCB rev2.3
 * Rev1.6 Updated toggler debounce and moved to 115200 baud
 * Rev1.7 Updated relay switch process and increased to 1000ms
 */
#define VERSION "Chicken Coop Door 1.7\n"

//Custom pins
#define EXT_DOOR_LED 8
#define EXT_POWER_LED 9


//Pin assignments
#define TOGGLER_INPUT PD2 
#define CLOSED_SENSOR_D PD3 
#define MOTOR_CONTROL_1 PD4 // When off, 12V out
#define LED_OUTPUT PD5 
#define DOOR_STATUS_OUTPUT PD6
#define MOTOR_CONTROL_2 PD7  //When off, GND out
#define CLOSED_SENSOR_A A1


#define TOGGLE_DEBOUNCE    10  // 1000 milli second
#define DOOR_RATE   1000  // 1 second test rate
#define TOGGLE_RATE 100  // 1 second test rate
#define FADE_RATE   50    // 20ms fade rate
#define MAX_BRIGHT  40    // PWM max brightness
#define MOTOR_DELAY 1000   // Short delay so motor doesn't cause voltage drop

static bool door_open = false;     // Motor output state of door
static unsigned char counter = 0;  //toggle counter

void setup() 
{
  // Start the Serial comms
  Serial.begin(115200);         
  delay(10);
  Serial.println(VERSION);
  
  //Custom
  pinMode(EXT_POWER_LED, OUTPUT);
  pinMode(EXT_DOOR_LED, OUTPUT);

  analogWrite(LED_OUTPUT, MAX_BRIGHT); 
  analogWrite(EXT_POWER_LED, MAX_BRIGHT); 
  
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

  analogWrite(LED_OUTPUT, 0); 
  analogWrite(EXT_POWER_LED, 0); 
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

void door_led()
{
  static unsigned long door_time = 0;

  if(millis() - door_time > DOOR_RATE)
  {
    door_time = millis();
   
    //Analog input read of the door sensor (debug)
    //Serial.println(analogRead(CLOSED_SENSOR_A));

    //indicate door state
    if(digitalRead(CLOSED_SENSOR_D) == HIGH)
    {
      digitalWrite(DOOR_STATUS_OUTPUT, HIGH);
      digitalWrite(EXT_DOOR_LED, HIGH);
      Serial.println("Door is CLOSED");
    }
    else
    {
      digitalWrite(DOOR_STATUS_OUTPUT, LOW);
      digitalWrite(EXT_DOOR_LED, LOW);
      Serial.println("Door is OPEN");
    }
  }
}

bool door_request()
{
  static unsigned long timer = 0;
  
  static bool waiting_for_release = false;
  bool rtn = false;

  if(millis() - timer > TOGGLE_RATE)
  {
    timer = millis();
    bool pressed = digitalRead(TOGGLER_INPUT) == LOW;
    //Serial.println(counter);
    if(pressed)
    {
      counter++;
      if(counter > TOGGLE_DEBOUNCE && !waiting_for_release) 
      {
        rtn = true;
        waiting_for_release = true;
        Serial.println("Request received");
      }
    }
    else
    {
      if(counter > 0)
        counter--;
      else
        waiting_for_release = false;
    }
  }

  return rtn;
}

void toggle_door()
{
  door_open = !door_open;
  
  //Ground both outputs first then wait
  digitalWrite(MOTOR_CONTROL_1, HIGH);
  digitalWrite(MOTOR_CONTROL_2, LOW);
  
  delay(MOTOR_DELAY); 

  digitalWrite(MOTOR_CONTROL_1, door_open ? LOW : HIGH);
  digitalWrite(MOTOR_CONTROL_2, door_open ? LOW : HIGH);
  
  if(door_open)
    Serial.println("Request to open");
  else
    Serial.println("Request to close");
}

void loop() 
{
  door_led();
  fade_led();
    
  if(door_request())
  {
    toggle_door();
  }
}
