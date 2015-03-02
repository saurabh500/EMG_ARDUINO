  // reads analog input from the five inputs from your arduino board 
  // and sends it out via serial
  
  int value=0;
  
  boolean isGestureEnabled = false;
  
  // Click gesture states
  int
  STATE_START = 0,
  STATE_PIT0 = 1,
  STATE_PEAK0 = 2,
  STATE_PIT1 = 3;
  int CURRENT_STATE = STATE_START; 
  
  unsigned long startGestureTime = 0;
  unsigned long GESTURE_ALOTTED_TIME = 1500;
  
  boolean forward = false;
  boolean currentDirectionForward = true;
  const int 
  PWM_A   = 3,
  DIR_A   = 12,
  BRAKE_A = 9,
  SNS_A   = A0;
  
  int THRESHOLD = 35;
  int ledRed = 5;
  int ledGreen = 6;
  int ledYellow = 4;
  
  // The value required to be considered a muscle flex.
  int sustainedThreshold  = 60;
  int gestureRequiredThreshold = 150;
  
  // The running time that musle has been continously flexed.
  unsigned long elapsedTimeSustained = 0;
  unsigned long elapsedTimeSustainedStart = 0;
  unsigned long elapsedTimeSustainedRequired = 2 * 1000; // ms
  
  int QUARTER_SECOND = 250;
  int HALF_SECOND = 500;
  
  int delayTime = QUARTER_SECOND;
  int GESTURE_TIME = 1500;
  int GESTURE_INTERVAL = 50;
  
  
  int MAX_INTERVALS = 30;
  
  int size = GESTURE_TIME / GESTURE_INTERVAL;
  int recordedValues[30];
  
  int recordedIndex = 0;
  
  
  void setup()
  {
  
      value = 0;     
      pinMode(5, INPUT);    
      pinMode(ledRed, OUTPUT);
      pinMode(ledGreen, OUTPUT);  
      pinMode(ledYellow, OUTPUT);  
      pinMode(BRAKE_A, OUTPUT);  // Brake pin on channel A
      pinMode(DIR_A, OUTPUT);    // Direction pin on channel A
    
      // Open Serial communication
      Serial.begin(9600);
      Serial.println("Motor shield DC motor Test:\n");
  
  }
  
  int initializationValues[40];
  int initCounter = 0;
  
  void loop()
  {
    //spinMotor();
    value = analogRead(5);
    Serial.println(value);
    if(isGestureEnabled){
      digitalWrite(ledRed, HIGH);
    }else{
      digitalWrite(ledRed, LOW);
    }
    // While the gesture is not enabled, we check for the
    // hold muscle tension signal. This signal is manifested as
    // a series of sustained reading of greater than 50 input value
    // over an interval of 3 seconds.
    if(!isGestureEnabled)
    {
      if(value > sustainedThreshold || initCounter !=0)
      {
        Serial.println("Muscle tension detected");
        initializationValues[initCounter] = value;
        initCounter++;
        
        if(initCounter >= 40){
          unsigned long sum = 0;
          for(int i = 0 ; i < 40; i++){
             sum += initializationValues[i];
          }
          long average = sum/40;
          if(average > THRESHOLD){
             isGestureEnabled = true;
             startGestureTime = millis(); // set the initial time for when the gesture detection starts.
             Serial.println("Gesture start.");
             
          }else{
            isGestureEnabled = false;
          }
          initCounter = 0;

        }  
      }
      delayTime = 20;
    }else{
     // The Gesture is enabled
     
     unsigned long currentTime = millis();
     
     // if we have been in this state for greater than the alloted time, restart the gesture recognizer to the beginning.
     if(currentTime - startGestureTime >= GESTURE_ALOTTED_TIME)
     {
        // Reset back to looking for gesture enabled state
        isGestureEnabled = false;
        CURRENT_STATE = STATE_START;
        
        digitalWrite(ledYellow, HIGH);
        delay(1000);
        digitalWrite(ledYellow, LOW);
        Serial.println("Gesture Failed"); 
     }
     
     int newState = CURRENT_STATE;

     if(CURRENT_STATE == STATE_START)
     {
       // looking for 0 (beginning of wave)
       if(value == 0)
       {
         newState = STATE_PIT0; 
        Serial.println("Entering PIT0 state.");
       }
     }
     else if(CURRENT_STATE == STATE_PIT0)
     {
       if(value >= gestureRequiredThreshold)
       {
         newState = STATE_PEAK0;
        Serial.println("Entering PEAK0 state.");
       }

     }
     else if(CURRENT_STATE == STATE_PEAK0)
     {
        if(value == 0)
        {
          Serial.println("Gesture recognized"); 
          digitalWrite(ledGreen, HIGH);
          if(currentDirectionForward)
            spinMotor(true /* forward */, 1000);
         
          currentDirectionForward = !currentDirectionForward;
          delay(1000);
          digitalWrite(ledGreen, LOW);
          CURRENT_STATE = STATE_START;
          newState = STATE_START;
          isGestureEnabled = false;
        }
     }
     
     CURRENT_STATE = newState;
     
     
     
//     recordedValues[recordedIndex] = value;
//     recordedIndex++;
//     
//     if(recordedIndex == MAX_INTERVALS){
//        boolean result = isGestureRecognized(); 
//        if(result){
//           Serial.println("Gesture recognized"); 
//           digitalWrite(ledGreen, HIGH);
//           spinMotor(true /* forward */, 500);
//           spinMotor(false /* backward */, 500);
//           digitalWrite(ledGreen, LOW);
//           isGestureEnabled = false;
//        }else{
//           isGestureEnabled = false;
//           recordedIndex = 0; 
//           delayTime = 50;
//           digitalWrite(ledYellow, HIGH);
//           delay(1000);
//           digitalWrite(ledYellow, LOW);
//           Serial.println("Gesture Failed"); 
//        }
//     }
    }
    
    // wait for a bit to not overload the port
    delay(delayTime);
   
  }
  
  boolean isGestureRecognized(){
      
      for (int i = 20; i < (GESTURE_TIME / GESTURE_INTERVAL)-1; i++) {
        if(recordedValues[i] == 0 ) {
            return true;
        }      
      }
      
      return false;
  
  }
  
  void spinMotor(bool forward, int duration)
  {
    digitalWrite(BRAKE_A, LOW);  // setting brake LOW disable motor brake
    
    if(forward)
      digitalWrite(DIR_A, HIGH);   // setting direction to HIGH the motor will spin forward
    else
      digitalWrite(DIR_A, LOW);   // setting direction to HIGH the motor will spin forward
  
    analogWrite(PWM_A, 255);     // Set the speed of the motor, 255 is the maximum value
    
    delay(duration);
    Serial.print("current consumption of motor: ");
    Serial.println(analogRead(SNS_A));
  
    // now stop the motor by inertia, the motor will stop slower than with the brake function
    analogWrite(PWM_A, 0);       // turn off power to the motor
  }
  
  /*
  void spinMotor(){
    
    
  // Set the outputs to run the motor forward
  if(forward){
    digitalWrite(BRAKE_A, LOW);  // setting brake LOW disable motor brake
    digitalWrite(DIR_A, HIGH);   // setting direction to HIGH the motor will spin forward
  
    analogWrite(PWM_A, 255);     // Set the speed of the motor, 255 is the maximum value
  
    delay(2000);                 // hold the motor at full speed for 5 seconds
    Serial.print("current consumption at full speed: ");
    Serial.println(analogRead(SNS_A));
  
  // Brake the motor
  
    Serial.println("Start braking\n");
    // raising the brake pin the motor will stop faster than the stop by inertia
    digitalWrite(BRAKE_A, HIGH);  // raise the brake
    delay(500);
  
  // Set the outputs to run the motor backward
  
  //  Serial.println("Backward");
  //  digitalWrite(BRAKE_A, LOW);  // setting againg the brake LOW to disable motor brake
  //  digitalWrite(DIR_A, LOW);    // now change the direction to backward setting LOW the DIR_A pin
  //
  //  analogWrite(PWM_A, 255);     // Set the speed of the motor
  //
  //  delay(5000);
  //  Serial.print("current consumption backward: ");
  //  Serial.println(analogRead(SNS_A));
  
    // now stop the motor by inertia, the motor will stop slower than with the brake function
    analogWrite(PWM_A, 0);       // turn off power to the motor
  
    Serial.print("current brake: ");
    Serial.println(analogRead(A0));
  }else{
    digitalWrite(BRAKE_A, LOW);  // setting brake LOW disable motor brake
    digitalWrite(DIR_A, HIGH);   // setting direction to HIGH the motor will spin forward
  
    analogWrite(PWM_A, 255);     // Set the speed of the motor, 255 is the maximum value
  
  // Set the outputs to run the motor backward
  
    Serial.println("Backward");
    digitalWrite(BRAKE_A, LOW);  // setting againg the brake LOW to disable motor brake
    digitalWrite(DIR_A, LOW);    // now change the direction to backward setting LOW the DIR_A pin
  //
    analogWrite(PWM_A, 127);     // Set the speed of the motor
  //
    delay(1000);
    Serial.print("current consumption backward: ");
    Serial.println(analogRead(SNS_A));
  
    // now stop the motor by inertia, the motor will stop slower than with the brake function
    analogWrite(PWM_A, 0);       // turn off power to the motor
  
    Serial.print("current brake: ");
    Serial.println(analogRead(A0));
  }
  forward = !forward;
  //  Serial.println("End of the motor shield test with DC motors. Thank you!");
  }
  */
