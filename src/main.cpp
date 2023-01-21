#include "Arduino.h"
#include <Servo.h>
#include <math.h>

Servo myservo; 
String serial_data; 

const double pivot_length = 0.04;
float servo_angle = 0.00; 
const int offset_angle = 8; 
const double max_servo_angle = 100; 
const double min_servo_angle = 8; 
float gripper_setpoint_angle = 0.0;

float actual_angle_rad = 0.0;

const byte numChars = 32; 
char receivedChars[numChars];
char receivedCmdChars[numChars];
char tempChars[numChars];  // temporary array for use when parsing
char tempCmdChars[numChars];  // temporary array for use when parsing
int SIZE = 1000;

// variables to hold the parsed data
char messageFromPC[numChars] = {0};
int integerFromPC = 0; 

unsigned int rcvd_cmd;

char dataStr[100] = "";
char buffer[7];

boolean newData = false;
boolean newCmdData = false;

void recvWithStartEndMarkers(){

    static boolean recvInProgress = false; 
    static boolean recvCmdInProgress = false;
    static byte ndx = 0;
    char startMarker = '[';
    char startCmdMarker = '{';
    char endMarker = ']';
    char endCmdMarker = '}';
    char rc; 

    while(Serial.available() && newData == false){
        rc = Serial.read();

        if(recvInProgress == true){
            if(rc != endMarker){
                receivedChars[ndx] = rc;
                ndx++;
                if(ndx >= numChars){
                    ndx = numChars - 1;
                }
            }
            else{
                receivedChars[ndx] = '\0';  // terminate string
                recvInProgress = false; 
                ndx = 0;
                newData = true;
            }
        }
        else if(rc == startMarker){
          recvInProgress = true;
        }else if(rc == startCmdMarker){
          recvCmdInProgress = true;
        }else if(recvCmdInProgress == true){
            if(rc != endCmdMarker){
                receivedCmdChars[ndx] = rc;
                ndx++;
                if(ndx >= numChars){
                    ndx = numChars - 1;
                }
            }
            else{
                receivedCmdChars[ndx] = '\0';  // terminate string
                recvCmdInProgress = false; 
                ndx = 0;
                newCmdData = true;
            }
        }
    }

}

void showParsedData(){

    // Serial.print("Rcvd gripper setpoint angle: ");
    // Serial.println(gripper_setpoint_angle);

}

void parseData(unsigned int mode){
  
  if(mode == 0){

    char * strtokIndx; 

    strtokIndx = strtok(tempChars, " ");
    gripper_setpoint_angle = atof(strtokIndx);

    // showParsedData();

  }else if(mode == 1){
    
    char * strtokIndxCmd;

    strtokIndxCmd = strtok(tempCmdChars, " ");
    rcvd_cmd = atoi(strtokIndxCmd);
    
    // Serial.println(rcvd_cmd);

  }
  
}

void setup(){

  Serial.begin(115200);

  myservo.attach(9);

  servo_angle = myservo.read();


}


void gripper_servo_control(float gripper_setpoint){

    float gripper_angle = gripper_setpoint * (180/M_PI);


    if(gripper_angle > max_servo_angle){

        gripper_angle = max_servo_angle;

    }

    if(gripper_angle < min_servo_angle){
        
        gripper_angle = min_servo_angle;

    }
  
  servo_angle = 90 - gripper_angle;   

//   Serial.print("Calculated servo angle: ");
//   Serial.println(servo_angle);

  myservo.write((int)servo_angle);

}

void writeActualPosition(){
    
  char msg[50];
  char buff[8];  

  
  strcpy(msg,"");

  actual_angle_rad = 90 - myservo.read() * M_PI/180;     // current gripper servo angle 

  dtostrf(actual_angle_rad,8,6,buff);
  strcat(msg,buff);
  
//   Serial.println(msg);
     
}


void loop(){

    recvWithStartEndMarkers();

    if(newData == true){
        strcpy(tempChars, receivedChars);
            // this temporary copy is necessary to protect the original data
            // because strtok() used in parseData() replaces the commas with \0
        parseData(0);
        // showParsedData();
        newData = false;
    }
    
    if(newCmdData == true){
        
        strcpy(tempCmdChars, receivedCmdChars);

        parseData(1);

        newCmdData = false;

        if(rcvd_cmd == 0){
        writeActualPosition();
        }

    }

    gripper_servo_control(gripper_setpoint_angle);

}




