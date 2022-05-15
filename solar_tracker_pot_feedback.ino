#include <SolarPosition.h>
#include <Time.h>
#include "max6675.h"

#include <Wire.h>
#include <DS3232RTC.h>

float angle_x,angle_y;

#define elevation_limit 60
#define azmith_limit  360
#define x_limit 180
#define y_limit  60
 //map(y,Yreal_max,Yreal_min,Ycommax_max,Ycommin_min)(550,500,0,30)
#define Yreal_max 550 //with y axis
#define Yreal_min 480
#define Ycommax_max 0 
#define Ycommin_min 60

//map(x,Xreal_max,Xreal_min,Xcommax_max,Xcommin_min)(555,735,180,-180)
#define Xreal_max 555 //with x axis
#define Xreal_min 735
#define Xcommax_max 180 
#define Xcommin_min -180

//ratio of the gears
#define ratio 3


//varibles to store the feedback values
const int pot=A3;
 const int pot_azm=A2;
 int val = 0;
 int val_azm=0;

//DC Motor(elevation) PINS
#define relay_1   8  //up
#define relay_2   9  //down

//stepper2 Pins (Azmuth) PINS
const int stepPin_2 = 7; 
const int dirPin_2 = 6;


//thrmo copuler pins and init
#define soPin 13              // SO = Serial Out
#define csPin 12              // CS = chip select CS pin
#define sckPin 11             // SCK = Serial Clock pin
MAX6675 Module(sckPin, csPin, soPin);


#define n 600 //delay for x-axis (stepper) for for loop 


 //some cities examples (EGYPT)
 //you can ignore this step if there is a GPS module to detect position
 
 //(latitude, longitude)
//SolarPosition place(31.1086578,29.0419507);//baniswaif
SolarPosition place(31.205753, 29.924526);//alex
//SolarPosition place(30.033333,31.233334);//cairo

//struct for the given position
SolarPosition_t savedPosition;



void setup() {
Serial.begin(9600); //serial rate
setSyncProvider(RTC.get);   // the function to get the time from the RTC
    
    //check if the RTC is connected proprley or not working
    if(timeStatus() != timeSet){
        Serial.println("Unable to sync with the RTC");}
    else{
        Serial.println("RTC has set the system time");
}


 //set the timeprovider if there is an RTC the provider will get a real time
SolarPosition::setTimeProvider(RTC.get/*now()*/);
//get the angles for place
savedPosition = place.getSolarPosition();


//make the pins of the motor as outputs
//stepper 1(elevation)
pinMode(relay_1,OUTPUT); 
pinMode(relay_2,OUTPUT);

//stepper2 Pins (Azmuth)
pinMode(stepPin_2,OUTPUT); 
pinMode(dirPin_2,OUTPUT);

//get a first read of the pots to pre-assign the variables to compeare
val = analogRead(pot);
val_azm = analogRead(pot_azm);
}

void loop() {
  //variables to store the RTC given data to be easier to use
 float elevation,azmith;
 
 //temp variable for thermocoupler
int tempsens = Module.readCelsius();

//map the pot value to use in compeare with RTC
float angle_y  = map(val, Yreal_max, Yreal_min, Ycommax_max, Ycommin_min);
float angle_x = map(val_azm,Xreal_max,Xreal_min,Xcommax_max,Xcommin_min);

//store the RTC values
elevation=place.getSolarPosition().elevation;
azmith=place.getSolarPosition().azimuth;


///////problem in mech structure//////
//loops to limit the value of the RTC so that 
//the wires wont be cut or the device wont turn around
if(azmith>=azmith_limit){azmith=azmith_limit;};
if(elevation>=elevation_limit){elevation=elevation_limit;};
if(azmith<=-azmith_limit){azmith=-azmith_limit;};
if(elevation<=-elevation_limit){elevation=-elevation_limit;};
if(elevation<15){elevation=15;};
if(elevation>60){elevation=60;};

azmith=azmith/2;

 //////////print in the monotor the date,time,real value,and the diff between the value and feedback
Serial.print(F("The sun was at an elevation of "));
  Serial.print((place.getSolarElevation()), 4);
  Serial.print(F(" and an azimuth of "));
  Serial.println(place.getSolarAzimuth(), 4);
  Serial.print(F("in place at "));
  Serial.print("clock::");
  Serial.print(hour());
  Serial.print(":");
  Serial.print(minute());
  Serial.print(":");
  Serial.print(second());
  Serial.println(' ');
  Serial.print(day());
  Serial.print(' ');
  Serial.print(month());
  Serial.print(' ');
  Serial.print(year());
  Serial.println();
  Serial.print("C = "); 
  Serial.println(tempsens);

  //this only work if the 4 values are in range 
  //if any pass the limit it will give a false value 
  //can work with all condetions using if loop to read if the limit is passed
  //change the print to the mas read posible 
  
  Serial.print("diff elev = "); 
  Serial.println(elev-angle_y);
  Serial.print("diff azm = "); 
  Serial.println(azm-angle_x);

  solar_stepper_control(azmith);  //function to control azmith
  solar_DC_control(elevation);  //function to control elevation
   //stop condetion for the relay
  digitalWrite(relay_1,HIGH);
  digitalWrite(relay_2,HIGH);
  //delay to be able to read on the montor
  delay(1000);
}






void solar_stepper_control(float azm)
{
  //if(((int)angle_x!=-(int)x_limit)||((int)angle_x!=(int)x_limit)){
  if((int)angle_x!=(int)azm)
  {
    if((int)angle_x>(int)azm)
    {
      digitalWrite(dirPin_2,HIGH);
      for(int i=0;i<3200;i++)
        {
          digitalWrite(stepPin_2,LOW);
          delayMicroseconds(n);
          digitalWrite(stepPin_2,HIGH);
          delayMicroseconds(n);
          
          val_azm = analogRead(pot_azm);
          angle_x =(map(val_azm,Xreal_max,Xreal_min,Xcommax_max,Xcommin_min)/ratio);
          if((angle_x<=-x_limit)||(angle_x>=x_limit))break;
          if((int)angle_x<=(int)azm)
          {
            break;
          }
          
        }
    
    }
    
    
  }
  if((int)angle_x!=(int)azm)
  {
    if((int)angle_x<(int)azm)
    {
      digitalWrite(dirPin_2,LOW);
      for(int i=0;i<3200;i++)
        {
          digitalWrite(stepPin_2,LOW);
          delayMicroseconds(n);
          digitalWrite(stepPin_2,HIGH);
          delayMicroseconds(n);
          
          val_azm = analogRead(pot_azm);
          angle_x =(map(val_azm,Xreal_max,Xreal_min,Xcommax_max,Xcommin_min)/ratio);
          if((angle_x<=-x_limit)||(angle_x>=x_limit))break;
          if((int)angle_x>=(int)azm)
          {
            break;
          }
          
        }
    
    }
   
  }
 // }
  
}

void solar_DC_control(float elev)
{
  
  if((int)angle_y!=abs((int)elev))
  {
    if((int)angle_y<(int)elev)
    {
      for(int i=0;(int)angle_y<(int)elev;){
          digitalWrite(relay_1,HIGH);
          digitalWrite(relay_2,LOW);
          val = analogRead(pot);
          //Serial.println("1=");
          //Serial.print(angle_y);
          angle_y =map(val, Yreal_max, Yreal_min, Ycommax_max, Ycommin_min);
          if((angle_y<=-y_limit)||(angle_y>=y_limit))
          {
            digitalWrite(relay_1,HIGH);
            digitalWrite(relay_2,HIGH);
            break;
          } 
          else if((int)angle_y>=abs((int)elev))
          {
             digitalWrite(relay_1,HIGH);
             digitalWrite(relay_2,HIGH);
            break;
          }
      
        }
    }
   else if((int)angle_y==abs((int)elev))
        {
          digitalWrite(relay_1,HIGH);
         digitalWrite(relay_2,HIGH);
        
        }
  else if((int)angle_y>(int)elev)
    {
      for(;;){
          digitalWrite(relay_1,LOW);
          digitalWrite(relay_2,HIGH);
          //Serial.println("1=");
          //Serial.print(angle_y);
          val = analogRead(pot);
          angle_y =map(val, Yreal_max, Yreal_min, Ycommax_max, Ycommin_min);
            if((angle_y<=-y_limit)||(angle_y>=y_limit))
            {
              digitalWrite(relay_1,HIGH);
              digitalWrite(relay_2,HIGH);
              break;
            }
            else if((int)angle_y<=abs((int)elev))
            {
               digitalWrite(relay_1,HIGH);
               digitalWrite(relay_2,HIGH);
              break;
            }
      
        }
        
    }
   
  }
  digitalWrite(relay_1,HIGH);
  digitalWrite(relay_2,HIGH);
  
}
