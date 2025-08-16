#include <Wire.h>
#include <EEPROM.h> 
#include <LiquidCrystal_I2C.h>
#include <ThreeWire.h>  
#include <RtcDS1302.h>
#include <IRremote.h>
ThreeWire myWire(4,5,19); // IO, SCLK, CE
RtcDS1302<ThreeWire> rtc(myWire);
IRrecv IR(0);
#define pushbutton1 32
#define pushbutton2 33
#define pushbutton3 25    
#define pushbutton4 26           //Push button ON  D3
#define relay1 14
#define relay2 27
#define relay3 16
#define relay4 17 

#define EEPROM_SIZE 39

int isChangedMenuCounter = 0;
//Counter to change positions of pages
int menu_counter=1 ;       
int submenu1_counter=0;

//------Timer Set 
byte onhour;
byte onmin;
byte onsec;
int onyear;
byte onmonth;
byte onday;


int bulb1_setTime_indiv[] = {0,0,0,0,0,0};  
int bulb2_setTime_indiv[] = {0,0,0,0,0,0};
int bulb3_setTime_indiv[] = {0,0,0,0,0,0};
int bulb4_setTime_indiv[] = {0,0,0,0,0,0};

int relay_num;
int trace_auto_indiv[] = {0,0,0,0};

int count_for_grp_indiv=1;

int count_for_autoBulb_indiv[] = {0,0,0,0};
char bulb_status_indiv[][8] = {"", "", "", ""};

int trace_remote_for_lcd[] = {0,0,0,0};
int up = 2;             //Up button
int down = 12;           //Down button
int sel=13;             //Select button
int save=15; //Save button
bool isIRDecoded[] = {false, false, false, false};

bool isBtnPushed[] = {false, false, false, false};
int IR_moderator[] = {0,0,0,0};
int BTN_moderator[] = {0,0,0,0};

//---------Storage debounce function-----//
boolean current_up = LOW;          
boolean last_up=LOW;            
boolean last_down = LOW;
boolean current_down = LOW; 
boolean current_sel = LOW;
boolean last_sel = LOW;
boolean current_save = LOW;
boolean last_save = LOW;      

bool led1State = false;
bool led2State = false;
bool led3State = false;
bool led4State = false;            //red LED ON  D5
                 //integer to hold button state
LiquidCrystal_I2C lcd(0x27,16,4);
void setup() {
  EEPROM.begin(EEPROM_SIZE);
  IR.enableIRIn();
  rtc.Begin();
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);

  pinMode(pushbutton1, INPUT_PULLUP);
  pinMode(pushbutton2, INPUT_PULLUP);
  pinMode(pushbutton3, INPUT_PULLUP);
  pinMode(pushbutton4, INPUT_PULLUP);                 //button as input
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);                                   //LEDs     as output
  digitalWrite(relay1, HIGH); 
  digitalWrite(relay2, HIGH); 
  digitalWrite(relay3, HIGH); 
  digitalWrite(relay4, HIGH);    
    
  pinMode(up, INPUT_PULLUP);
  pinMode(down, INPUT_PULLUP);
  pinMode(sel, INPUT_PULLUP);
                     //set initial state as off
}
boolean debounce(boolean last, int pin)
{
  boolean current = digitalRead(pin);
 
  if (last != current)
    {
      delay(10);
      current = digitalRead(pin);
    }
  return current;
}
char bulb[][5] = {"","","",""};
int reset_lcd[]={0,0,0,0};int for_start_auto_indiv_lcd[] = {0,0,0,0};
void loop() {
  RtcDateTime now = rtc.GetDateTime();
  if(now.Hour() == 0 && now.Minute() == 0 && now.Second() == 0) {
    IR_moderator[0] = 0;IR_moderator[1] = 0;IR_moderator[2] = 0;IR_moderator[3] = 0;
    BTN_moderator[0] = 0;BTN_moderator[1] = 0;BTN_moderator[2] = 0;BTN_moderator[3] = 0;
  }
  RTC_DATE_AND_TIME();
  processIRRemoteCommands();
  processPushButtonInputs();
    if(menu_counter==1){
      lcd.setCursor(1,2);
      lcd.print("B1");
      lcd.setCursor(5,2);
      lcd.print("B2");  
      lcd.setCursor(9,2);
      lcd.print("B3");
      lcd.setCursor(13,2);
      lcd.print("B4");
    }
  if(isChangedMenuCounter == 1 && menu_counter==1){
    isChangedMenuCounter == 0;
    if(digitalRead(relay1)==0){
      lcd.setCursor(1,3);   
        lcd.print("ON ");
    }
    else{
      lcd.setCursor(1,3);   
        lcd.print("OFF");
    }
    if(digitalRead(relay2)==0){
      lcd.setCursor(5,3);   
        lcd.print("ON ");
    }
    else{
      lcd.setCursor(5,3);   
        lcd.print("OFF");
    }
    if(digitalRead(relay3)==0){
      lcd.setCursor(9,3);   
        lcd.print("ON ");
    }
    else{
      lcd.setCursor(9,3);   
        lcd.print("OFF");
    }
    if(digitalRead(relay4)==0){
      lcd.setCursor(13,3);   
        lcd.print("ON ");
    }
    else{
      lcd.setCursor(13,3);   
        lcd.print("OFF");
    }
  }

    if(digitalRead(relay1) == 0 && for_start_auto_indiv_lcd[0] == 0){
    if(menu_counter==1){
        lcd.setCursor(1,3);   
        lcd.print("ON ");
    }
    
    for_start_auto_indiv_lcd[0] ++;

  }
  if(digitalRead(relay2) == 0 && for_start_auto_indiv_lcd[1] == 0){
    if(menu_counter==1){
       lcd.setCursor(5,3);
        lcd.print("ON ");
    }
   
    for_start_auto_indiv_lcd[1] ++;

  }
  if(digitalRead(relay3) == 0 && for_start_auto_indiv_lcd[2] == 0){
    if(menu_counter==1){
      lcd.setCursor(9,3);
      lcd.print("ON ");
    }
    
    for_start_auto_indiv_lcd[2] ++;

  }
  if(digitalRead(relay4) == 0 && for_start_auto_indiv_lcd[3] == 0){
    if(menu_counter==1){
      lcd.setCursor(13,3);
     lcd.print("ON ");
    }
    
    for_start_auto_indiv_lcd[3] ++;

  }

  if(menu_counter==1){
      if(trace_auto_indiv[0]==1 && trace_remote_for_lcd[0] == 0 && trace_remote_for_lcd[1] == 0){
        if(trace_remote_for_lcd[1] == 0) trace_remote_for_lcd[1] = 1;
        if(trace_remote_for_lcd[1] == 1){
           lcd.setCursor(1,3);
           if(trace_auto_indiv[0] ==0)
            lcd.print("OFF");
        }
         if(trace_remote_for_lcd[1] == 0){
             lcd.setCursor(1,3);
              lcd.print("ON ");
        }
       
      }
      
      if(trace_auto_indiv[1]==1 &&  trace_remote_for_lcd[1] == 0 && trace_remote_for_lcd[2] == 0) {
        if(trace_remote_for_lcd[2] == 0) trace_remote_for_lcd[2] = 1;
        if(trace_remote_for_lcd[2] == 1){
          lcd.setCursor(5,3);
          if(trace_auto_indiv[1] ==0 )
          lcd.print("OFF");
           
        }
         if(trace_remote_for_lcd[2] == 0){
            lcd.setCursor(5,3);
           lcd.print("ON ");
        }
        
      }
      
      if(trace_auto_indiv[2]==1 && trace_remote_for_lcd[2] == 0 && trace_remote_for_lcd[3] == 0){
        if(trace_remote_for_lcd[3] == 0) trace_remote_for_lcd[3] = 1;
        if(trace_remote_for_lcd[3] == 1  ){
           lcd.setCursor(9,3);
           if(trace_auto_indiv[2] ==0)
           lcd.print("OFF"); 
        }
         if(trace_remote_for_lcd[3] == 0){
             lcd.setCursor(9,3);
             lcd.print("ON ");
        }
        
      }
      if(trace_auto_indiv[3]==1 && trace_remote_for_lcd[3] == 0 && trace_remote_for_lcd[0] == 0){
        if(trace_remote_for_lcd[0] == 0) trace_remote_for_lcd[0] = 1;
        if(trace_remote_for_lcd[0] == 1){
           lcd.setCursor(13,3);
           lcd.print("OFF");
        }
         if(trace_remote_for_lcd[0] == 0){
             lcd.setCursor(13,3);
             lcd.print("ON ");
        }
        
      }
  }
 

  count_for_grp_indiv= EEPROM.read(10);
  count_for_autoBulb_indiv[0] = EEPROM.read(11);
  count_for_autoBulb_indiv[1] = EEPROM.read(12);
  count_for_autoBulb_indiv[2] = EEPROM.read(13);
  count_for_autoBulb_indiv[3] = EEPROM.read(14);
  bulb1_setTime_indiv[0] = EEPROM.read(15);
  bulb1_setTime_indiv[1] = EEPROM.read(16);
  bulb1_setTime_indiv[2] = EEPROM.read(17);
  bulb1_setTime_indiv[3] = EEPROM.read(18);
  bulb1_setTime_indiv[4] = EEPROM.read(19);
  bulb1_setTime_indiv[5] = EEPROM.read(20);
  bulb2_setTime_indiv[0] = EEPROM.read(21);
  bulb2_setTime_indiv[1] = EEPROM.read(22);
  bulb2_setTime_indiv[2] = EEPROM.read(23);
  bulb2_setTime_indiv[3] = EEPROM.read(24);
  bulb2_setTime_indiv[4] = EEPROM.read(25);
  bulb2_setTime_indiv[5] = EEPROM.read(26);
  bulb3_setTime_indiv[0] = EEPROM.read(27);
  bulb3_setTime_indiv[1] = EEPROM.read(28);
  bulb3_setTime_indiv[2] = EEPROM.read(29);
  bulb3_setTime_indiv[3] = EEPROM.read(30);
  bulb3_setTime_indiv[4] = EEPROM.read(31);
  bulb3_setTime_indiv[5] = EEPROM.read(32);
  bulb4_setTime_indiv[0] = EEPROM.read(33);
  bulb4_setTime_indiv[1] = EEPROM.read(34);
  bulb4_setTime_indiv[2] = EEPROM.read(35);
  bulb4_setTime_indiv[3] = EEPROM.read(36);
  bulb4_setTime_indiv[4] = EEPROM.read(37);
  bulb4_setTime_indiv[5] = EEPROM.read(38);
  

  

  //THIS IS FOR BY INDIVIDUAL ONLY
  if(now.Hour() >= bulb1_setTime_indiv[0] && now.Hour() <=  bulb1_setTime_indiv[3]){
    if(now.Hour() ==  bulb1_setTime_indiv[0] ){
        if(now.Minute() ==  bulb1_setTime_indiv[1]){
          if(now.Second() ==  bulb1_setTime_indiv[2]){
              IR_moderator[0]=0;
              BTN_moderator[0]=0;
              
          }
        }
    }
  }
  if(now.Hour() >= bulb2_setTime_indiv[0] && now.Hour() <=  bulb2_setTime_indiv[3]){
    if(now.Hour() ==  bulb2_setTime_indiv[0] ){
        if(now.Minute() ==  bulb2_setTime_indiv[1]){
          if(now.Second() ==  bulb2_setTime_indiv[2]){
              IR_moderator[1]=0;
              BTN_moderator[1]=0;
              
          }
        }
    }
  }
  if(now.Hour() >= bulb3_setTime_indiv[0] && now.Hour() <=  bulb3_setTime_indiv[3]){
    if(now.Hour() ==  bulb3_setTime_indiv[0] ){
        if(now.Minute() ==  bulb3_setTime_indiv[1]){
          if(now.Second() ==  bulb3_setTime_indiv[2]){
              IR_moderator[2]=0;
              BTN_moderator[2]=0;
              
          }
        }
    }
  }
  if(now.Hour() >= bulb4_setTime_indiv[0] && now.Hour() <=  bulb4_setTime_indiv[3]){
    if(now.Hour() ==  bulb4_setTime_indiv[0] ){
        if(now.Minute() ==  bulb4_setTime_indiv[1]){
          if(now.Second() ==  bulb4_setTime_indiv[2]){
              IR_moderator[3]=0;
              BTN_moderator[3]=0;
              
          }
        }
    }
  }
  
   if(IR_moderator[0]==0  && BTN_moderator[0]==0 && count_for_autoBulb_indiv[0] == 1 ){
    automatic_indiv(0);
  }
  if(IR_moderator[1]==0 && BTN_moderator[1]==0  && count_for_autoBulb_indiv[1] == 1 ){
    automatic_indiv(1);
  }
  if(IR_moderator[2]==0 && BTN_moderator[2]==0  && count_for_autoBulb_indiv[2] == 1 ){
    automatic_indiv(2);
  }
  if(IR_moderator[3]==0 && BTN_moderator[3]==0  && count_for_autoBulb_indiv[3] == 1 ){
    automatic_indiv(3);
  }
 //
}

void automatic_indiv(int pos){
  RtcDateTime now = rtc.GetDateTime();
  
  
  if(trace_auto_indiv[2] == 1 && bulb4_setTime_indiv[3] == bulb1_setTime_indiv[0]) trace_auto_indiv[3] = 0;
  if(trace_auto_indiv[3] == 1 && bulb1_setTime_indiv[3] == bulb2_setTime_indiv[0]) trace_auto_indiv[0] = 0;

  if(pos==0)
  {
    relay_num = relay1;
    if(now.Hour() >= bulb1_setTime_indiv[0] && now.Hour() <= bulb1_setTime_indiv[3] ){
      
      if(BTN_moderator[0]==0 )trace_auto_indiv[0] = 1;
   
      if(trace_auto_indiv[3] == 1 && bulb1_setTime_indiv[3] == bulb4_setTime_indiv[0]) trace_auto_indiv[3] = 0;
      if(now.Hour() == bulb1_setTime_indiv[0] ){
        if(now.Minute() == bulb1_setTime_indiv[1] ){
          if(now.Second() < bulb1_setTime_indiv[2] ){
             digitalWrite(relay_num, HIGH); 
          }
          else digitalWrite(relay_num, LOW);
        }
        if(now.Minute() > bulb1_setTime_indiv[1] ){
          
           digitalWrite(relay_num, LOW);
        }
      }
      else digitalWrite(relay_num, LOW);

      if(now.Hour() == bulb1_setTime_indiv[3]  ){
        if(now.Minute() == bulb1_setTime_indiv[4] ){
          if(now.Second() < bulb1_setTime_indiv[5] ){
              digitalWrite(relay_num, LOW); 
          }
          else digitalWrite(relay_num, HIGH);
        }
        if(now.Minute() > bulb1_setTime_indiv[4] ){
          
           digitalWrite(relay_num, HIGH);
        }
      }
    }else {digitalWrite(relay_num, HIGH);trace_auto_indiv[0]=0;}
  }
   
  if(pos==1) 
  {
    relay_num = relay2;
    if(now.Hour() >= bulb2_setTime_indiv[0] && now.Hour() <= bulb2_setTime_indiv[3] ){
      
      if(BTN_moderator[1]==0)trace_auto_indiv[1] = 1;
      
      if(trace_auto_indiv[0] == 1 && bulb2_setTime_indiv[0] == bulb1_setTime_indiv[3]) trace_auto_indiv[0] = 0;

      if(now.Hour() == bulb2_setTime_indiv[0] ){
        if(now.Minute() == bulb2_setTime_indiv[1] ){
          if(now.Second() < bulb2_setTime_indiv[2] ){
             digitalWrite(relay_num, HIGH); 
          }
          else digitalWrite(relay_num, LOW);
        }
        if(now.Minute() > bulb2_setTime_indiv[1] ){
          
           digitalWrite(relay_num, LOW);
        }
      }
      else digitalWrite(relay_num, LOW);

      if(now.Hour() == bulb2_setTime_indiv[3]  ){
        if(now.Minute() == bulb2_setTime_indiv[4] ){
          if(now.Second() < bulb2_setTime_indiv[5] ){
              digitalWrite(relay_num, LOW); 
          }
          else digitalWrite(relay_num, HIGH);
        }
        if(now.Minute() > bulb2_setTime_indiv[4] ){
          
           digitalWrite(relay_num, HIGH);
        }
      }
    }else{ digitalWrite(relay_num, HIGH);trace_auto_indiv[1]=0;}
  }
 
  if(pos==2) 
   {
    relay_num = relay3;
    if(now.Hour() >= bulb3_setTime_indiv[0] && now.Hour() <= bulb3_setTime_indiv[3] ){
      
      
      if(BTN_moderator[2]==0)trace_auto_indiv[2] = 1;
  
      if(trace_auto_indiv[1] == 1 && bulb3_setTime_indiv[0] == bulb2_setTime_indiv[3]) trace_auto_indiv[1] = 0;
  
      if(now.Hour() == bulb3_setTime_indiv[0] ){
        if(now.Minute() == bulb3_setTime_indiv[1] ){
          if(now.Second() < bulb3_setTime_indiv[2] ){
             digitalWrite(relay_num, HIGH); 
          }
          else digitalWrite(relay_num, LOW);
        }
        if(now.Minute() > bulb3_setTime_indiv[1] ){
          
           digitalWrite(relay_num, LOW);
        }
      }
      else digitalWrite(relay_num, LOW);

      if(now.Hour() == bulb3_setTime_indiv[3]  ){
        if(now.Minute() == bulb3_setTime_indiv[4] ){
          if(now.Second() < bulb3_setTime_indiv[5] ){
              digitalWrite(relay_num, LOW); 
          }
          else digitalWrite(relay_num, HIGH);
        }
        if(now.Minute() > bulb3_setTime_indiv[4] ){
          
           digitalWrite(relay_num, HIGH);
        }
      }
    }else {digitalWrite(relay_num, HIGH);trace_auto_indiv[2]=0;}
  }
 
  if(pos==3) 
   {
    relay_num = relay4;
    if(now.Hour() >= bulb4_setTime_indiv[0] && now.Hour() <= bulb4_setTime_indiv[3] ){
  
      if(BTN_moderator[3]==0)trace_auto_indiv[3] = 1;
      if(trace_auto_indiv[2] == 1 && bulb4_setTime_indiv[0] == bulb3_setTime_indiv[3]) trace_auto_indiv[2] = 0;
      if(now.Hour() == bulb4_setTime_indiv[0] ){
        if(now.Minute() == bulb4_setTime_indiv[1] ){
          if(now.Second() < bulb4_setTime_indiv[2] ){
             digitalWrite(relay_num, HIGH); 
          }
          else digitalWrite(relay_num, LOW);
        }
        if(now.Minute() > bulb4_setTime_indiv[1] ){
          
           digitalWrite(relay_num, LOW);
        }
      }
      else digitalWrite(relay_num, LOW);

      if(now.Hour() == bulb4_setTime_indiv[3]  ){
        if(now.Minute() == bulb4_setTime_indiv[4] ){
          if(now.Second() < bulb4_setTime_indiv[5] ){
              digitalWrite(relay_num, LOW); 
          }
          else digitalWrite(relay_num, HIGH);
        }
        if(now.Minute() > bulb4_setTime_indiv[4] ){
          
           digitalWrite(relay_num, HIGH);
        }
      }
    }else{digitalWrite(relay_num, HIGH);trace_auto_indiv[3]=0;};
  } 
}


int reseter_if_indiv[] = {0,0,0,0};
int counter_push_HIGH_indiv[] = {0,0,0,0};
int counter_push_LOW_indiv[] = {0,0,0,0};

int moderator[] = {0,0,0,0};
int IREnable[] = {0,0,0,0};
void processPushButtonInputs() {
   
    
  // Your existing push button processing code
  // Add your existing code here for push button processing
 if (digitalRead(pushbutton1) == LOW && isIRDecoded[0] == true){
  if(IREnable[0] == 1){
      IREnable[0] =0;
    }
  isIRDecoded[0] = false;
 }
  if (digitalRead(pushbutton2) == LOW && isIRDecoded[1] == true){
    if(IREnable[1] == 1){
      IREnable[1] =0;
    }
    
   isIRDecoded[1] = false;
 }
  if (digitalRead(pushbutton3) == LOW && isIRDecoded[2] == true){
    if(IREnable[2] == 1){
      IREnable[2] =0;
    }
  isIRDecoded[2] = false;
 }
  if (digitalRead(pushbutton4) == LOW && isIRDecoded[3] == true){
    if(IREnable[3] == 1){
      IREnable[3] =0;
    }
  isIRDecoded[3] = false;
 }

if (digitalRead(pushbutton1) == LOW && isIRDecoded[0] == false  && IREnable[0] == 0 && moderator[0] == 0) {
   
        if (counter_push_HIGH_indiv[0] == 0 ){

      led1State = digitalRead(relay1);
      if(led1State == digitalRead(pushbutton1) && reseter_if_indiv[0] != 0)
      led1State = !led1State;

      if(reseter_if_indiv[0]==0)
      {
        reseter_if_indiv[0]+=1;
        led1State = !led1State;
      }
      counter_push_LOW_indiv[0] = 0;
      counter_push_HIGH_indiv[0]+=1;
    }
    if (menu_counter == 1){
        lcd.setCursor(1,3);
      
        lcd.print(led1State ? "ON " : "OFF");
    }
    if(trace_auto_indiv[0]==1)trace_auto_indiv[0] = 0;
    
    

  led1State = !led1State;
    
  BTN_moderator[0] = 1;
  isBtnPushed[0]=true;
     digitalWrite(relay1,  LOW); 
     moderator[0] = 1;
  }

  else if (digitalRead(pushbutton1) == HIGH && isIRDecoded[0] == false && IREnable[0] == 0){
  
      if (counter_push_LOW_indiv[0] == 0 ){
      
      led1State = digitalRead(relay1);
      if(led1State == digitalRead(pushbutton1) && reseter_if_indiv[0] != 0)
      led1State = !led1State;
      if(reseter_if_indiv[0]==0)
      {
        reseter_if_indiv[0]+=1;
        led1State = !led1State;
      }
      counter_push_LOW_indiv[0]+=1;
      counter_push_HIGH_indiv[0] = 0;
    }
    if (menu_counter == 1){
      if(!(led1State == 0 && trace_auto_indiv[0]==1))
          {
            lcd.setCursor(1,3);
            lcd.print(led1State ? "ON " : "OFF");
          }
       
      }
    if(trace_auto_indiv[0]==0)
    digitalWrite(relay1,  HIGH);
  

     isBtnPushed[0]=true;
     
    moderator[0] = 0;
    

  }
  if (digitalRead(pushbutton2) == LOW && isIRDecoded[1] == false && IREnable[1] == 0 && moderator[1] == 0)  {
    if (counter_push_HIGH_indiv[1] == 0){
      led2State = digitalRead(relay2);
      if(led2State == digitalRead(pushbutton2) && reseter_if_indiv[1] != 0)
      led2State = !led2State;
      if(reseter_if_indiv[1]==0)
      {
        reseter_if_indiv[1]+=1;
        led2State = !led2State;
      }
      counter_push_LOW_indiv[1] = 0;
      counter_push_HIGH_indiv[1]+=1;
    }
    if (menu_counter == 1){
        lcd.setCursor(5,3);
      lcd.print(led2State ? "ON " : "OFF");
    }
    if(trace_auto_indiv[1]==1)trace_auto_indiv[1] = 0;
    BTN_moderator[1] = 1;
 isBtnPushed[1]=true;
    digitalWrite(relay2,  LOW);
    moderator[1] = 1;
  }
  else if (digitalRead(pushbutton2) == HIGH && isIRDecoded[1] == false && IREnable[1] == 0){
    if (counter_push_LOW_indiv[1] == 0){
        
      led2State = digitalRead(relay2);
      if(led2State == digitalRead(pushbutton2) && reseter_if_indiv[1] != 0)
      led2State = !led2State;
      if(reseter_if_indiv[1]==0)
      {
        reseter_if_indiv[1]+=1;
        led2State = !led2State;
      }
      counter_push_HIGH_indiv[1] = 0;
        counter_push_LOW_indiv[1]+=1;

      
    }
      if (menu_counter == 1 ){
        if(!(led2State == 0 && trace_auto_indiv[1]==1))
          {
            lcd.setCursor(5,3);
            lcd.print(led2State ? "ON " : "OFF");
          }
    
    }
     isBtnPushed[1]=true;
     if(trace_auto_indiv[1]==0)
    digitalWrite(relay2,  HIGH);
    moderator[1] = 0;

  }
  if (digitalRead(pushbutton3) == LOW && isIRDecoded[2] == false && IREnable[2] == 0 && moderator[2] == 0) {
    if (counter_push_HIGH_indiv[2] == 0){
          led3State = digitalRead(relay3);
          if(led3State == digitalRead(pushbutton3) && reseter_if_indiv[2] != 0)
            led3State = !led3State;
            if(reseter_if_indiv[2]==0)
            {
              reseter_if_indiv[2]+=1;
              led3State = !led3State;
            }
          counter_push_LOW_indiv[2] = 0;
          counter_push_HIGH_indiv[2]+=1;
        }
        if (menu_counter == 1){
            lcd.setCursor(9,3);
          lcd.print(led3State ? "ON " : "OFF");
        }
    if(trace_auto_indiv[2]==1)trace_auto_indiv[2] = 0;
    BTN_moderator[2] = 1;
     isBtnPushed[2]=true;
    digitalWrite(relay3,  LOW);
    moderator[2] = 1;

  }
  else if (digitalRead(pushbutton3) == HIGH && isIRDecoded[2] == false && IREnable[2] == 0){
    
    if (counter_push_LOW_indiv[2] == 0){   
          led3State = digitalRead(relay3);

          if(led3State == digitalRead(pushbutton3) && reseter_if_indiv[2] != 0) 
            led3State = !led3State;
            if(reseter_if_indiv[2]==0 )
            {
              reseter_if_indiv[2]+=1;
              led3State = !led3State;
            }
          counter_push_LOW_indiv[2]+=1;
          counter_push_HIGH_indiv[2] = 0;
        }
        if (menu_counter == 1 ){
       
            
          if(!(led3State == 0 && trace_auto_indiv[2]==1))
          {
            lcd.setCursor(9,3);
            
            lcd.print(led3State ? "ON " : "OFF");
          }
            
        }
    
     isBtnPushed[2]=true;
     if(trace_auto_indiv[2]==0)
     digitalWrite(relay3,  HIGH);
     moderator[2] = 0;
 

  }
  if (digitalRead(pushbutton4) == LOW && isIRDecoded[3] == false && IREnable[3] == 0 && moderator[3] == 0) {
    if (counter_push_HIGH_indiv[3] == 0){
      led4State = digitalRead(relay4);
      if(led4State == digitalRead(pushbutton4) && reseter_if_indiv[3] != 0)
            led4State = !led4State;
            if(reseter_if_indiv[3]==0)
            {
              reseter_if_indiv[3]+=1;
              led4State = !led4State;
            }
      counter_push_LOW_indiv[3] = 0;
      counter_push_HIGH_indiv[3]+=1;
    }
    if (menu_counter == 1){
            lcd.setCursor(13,3);
          lcd.print(led4State ? "ON " : "OFF");
        }
    if(trace_auto_indiv[3]==1)trace_auto_indiv[3] = 0;
    BTN_moderator[3] = 1;
     isBtnPushed[3]=true;
    digitalWrite(relay4,  LOW);
    moderator[3] = 1;
  }
  else if (digitalRead(pushbutton4) == HIGH && isIRDecoded[3] == false && IREnable[3] == 0){
    if (counter_push_LOW_indiv[3] == 0){
            
          led4State = digitalRead(relay4);
          if(led4State == digitalRead(pushbutton4) && reseter_if_indiv[3] != 0)
            led4State = !led4State;
            if(reseter_if_indiv[3]==0)
            {
              reseter_if_indiv[3]+=1;
              led4State = !led4State;
            }
          counter_push_LOW_indiv[3]+=1;
          counter_push_HIGH_indiv[3] = 0;
        }
        if (menu_counter == 1){
          if(!(led4State == 0 && trace_auto_indiv[3]==1))
          {
            lcd.setCursor(13,3);
            lcd.print(led4State ? "ON " : "OFF");
          }
           
        }
     isBtnPushed[3]=true;
     if(trace_auto_indiv[3]==0)
    digitalWrite(relay4,  HIGH);
    moderator[3] = 0;

  }

}

void processIRRemoteCommands() {
  
   if (IR.decode()) {
  
    Serial.println(IR.decodedIRData.decodedRawData, HEX);

  if (IR.decodedIRData.decodedRawData == 0xFE01FF00) { // NUM 1 BUTTON
      IR_moderator[0] = 1;
      if(count_for_grp_indiv == 1){
        if(trace_auto_indiv[0] == 1 && digitalRead(pushbutton1) == 1 && digitalRead(relay1) == 0 && led1State == 0) trace_remote_for_lcd[0] = 1;
       if(trace_auto_indiv[0] == 1 && digitalRead(pushbutton1) == 1 && digitalRead(relay1) == 1 && led1State == 1) trace_remote_for_lcd[0] = 0;
      }
    
      led1State = !led1State;


     isIRDecoded[0] = true;

     if (digitalRead(pushbutton1) == LOW ){
        IREnable[0] = 1;
      }
      lcd.setCursor(1,3);
      lcd.print(led1State ? "OFF" : "ON ");
      digitalWrite(relay1, led1State ? HIGH : LOW);

    }

     if (IR.decodedIRData.decodedRawData == 0xFD02FF00) { //NUM 2 BUTTON
     IR_moderator[1] = 1;
      if(trace_auto_indiv[1] == 1 && digitalRead(pushbutton2) == 1 && digitalRead(relay2) == 0 && led2State == 0) trace_remote_for_lcd[1] = 1;
      if(trace_auto_indiv[1] == 1 && digitalRead(pushbutton2) == 1 && digitalRead(relay2) == 1 && led2State == 1) trace_remote_for_lcd[1] = 0;
      led2State = !led2State;  
      isIRDecoded[1] = true;
      if (digitalRead(pushbutton2) == LOW  ){
        IREnable[1] = 1;
      
      }
      lcd.setCursor(5,3);
  lcd.print(led2State ? "OFF" : "ON ");
      digitalWrite(relay2, led2State ? HIGH : LOW);

    }

    if (IR.decodedIRData.decodedRawData == 0xFC03FF00){ // NUM 3 BUTTON
    IR_moderator[2] = 1;
    if(trace_auto_indiv[2] == 1 && digitalRead(pushbutton3) == 1 && digitalRead(relay3) == 0 && led3State == 0) trace_remote_for_lcd[2] = 1;
    if(trace_auto_indiv[2] == 1 && digitalRead(pushbutton3) == 1 && digitalRead(relay3) == 1 && led3State == 1) trace_remote_for_lcd[2] = 0;
      led3State = !led3State; 
      isIRDecoded[2] = true;
      if (digitalRead(pushbutton3) == LOW ){
        IREnable[2] = 1;
      }
      lcd.setCursor(9,3);
      lcd.print(led3State ? "OFF" : "ON ");
      digitalWrite(relay3, led3State ? HIGH : LOW);

    }

   if (IR.decodedIRData.decodedRawData == 0xF708FF00){ // NUM 8 BUTTON
   IR_moderator[3] = 1;
    if(trace_auto_indiv[3] == 1 && digitalRead(pushbutton4) == 1 && digitalRead(relay4) == 0 && led4State == 0) trace_remote_for_lcd[3] = 1;
    if(trace_auto_indiv[3] == 1 && digitalRead(pushbutton4) == 1 && digitalRead(relay4) == 1 && led4State == 1) trace_remote_for_lcd[3] = 0;
      led4State = !led4State; 
      isIRDecoded[3] = true;
      if (digitalRead(pushbutton4) == LOW ){
        IREnable[3] = 1;
      }
      lcd.setCursor(13,3);
  lcd.print(led4State ? "OFF" : "ON ");
      digitalWrite(relay4, led4State ? HIGH : LOW);

    }
    IR.resume();

  }
}

void RTC_DATE_AND_TIME(){
  RtcDateTime now = rtc.GetDateTime();
    onhour = now.Hour();
    onmin = now.Minute();
    onsec = now.Second();
    onyear = now.Year();
    onmonth = now.Month();
    onday = now.Day();

    
  current_up = debounce(last_up, up);  //Debounce for Up button
  current_down = debounce(last_down, down);   //Debounce for Down button
  current_sel = debounce(last_sel, sel);      //Debounce for Select  button
  current_save = debounce(last_save, save);      //Debounce for Save  button

    //----Page counter function to move pages----//
if(submenu1_counter==0){

//Page Up
    if (last_up== LOW && current_up == HIGH){ 
      lcd.clear();                       
      if(menu_counter==1){             
      menu_counter= 2; 
      
      }
      else if(menu_counter==2){
      
      menu_counter= 3;  }
      else if(menu_counter==3){
      
      menu_counter= 6;  
      }
      
      else if (menu_counter==6) menu_counter=1;
  }
  
    last_up = current_up;

    //Page Up
  if (last_down== LOW && current_down == HIGH){ 
      lcd.clear();                       
      if(menu_counter==1){             
      menu_counter= 2; 
      
      }
      else if(menu_counter==2){
      
      menu_counter= 3;  
      }
      else if(menu_counter==3){
      
      menu_counter= 6;  }
    
      else if (menu_counter==6) menu_counter=1;
  }
  
    last_down = current_down;

}

//------- Switch function to write and show what you want---// 
  switch (menu_counter) {
   
    case 1:{  
      lcd.setCursor(1,0);
      lcd.print("Date: ");
      lcd.setCursor(7,0);
      if (now.Month()< 10){
        lcd.print("0");
      }
      lcd.print(String(now.Month()));
      lcd.setCursor(9,0);
      lcd.print(":");
      lcd.setCursor(10, 0);
      if (now.Day()< 10){
        lcd.print("0");
      }
      lcd.print(String(now.Day()));
      lcd.setCursor(12,0);
      lcd.print(":");
      lcd.setCursor(13,0);
      lcd.print(String(now.Year()));
    
      lcd.setCursor(1,1);
      lcd.print("Time: ");
      lcd.setCursor(7,1);
      if (now.Hour()< 10){
        lcd.print("0");
      }
      lcd.print(String(now.Hour()));
      lcd.setCursor(9,1);
      lcd.print(":");
      lcd.setCursor(10, 1);
      if (now.Minute()< 10){
        lcd.print("0");
      }
      lcd.print(String(now.Minute()));
      lcd.setCursor(12,1);
      lcd.print(":");
      lcd.setCursor(13,1);
      if (now.Second()< 10){
        lcd.print("0");
      }
      lcd.print(String(now.Second()));
      //lcd.print(timeStr);      
    }
    break;

    case 2: {
      isChangedMenuCounter = 1;
      
      lcd.setCursor(0,0);
      lcd.print("TIME SET");

      lcd.setCursor(4,1);  
      if(onhour<10){
        lcd.print("0");
      }
      lcd.print(onhour);
      lcd.setCursor(7,1);
      if(onmin<10){
        lcd.print("0");
      }
      lcd.print(onmin);
      lcd.setCursor(10,1);
      if(onsec<10){
        lcd.print("0");
      }
      lcd.print(onsec);
      
     if (last_sel== LOW && current_sel == HIGH){ 
        if(submenu1_counter <3){                    
             submenu1_counter ++;                         
          }
        else{                                       
             submenu1_counter=1;
           }
     }
     last_sel=current_sel;                      

     if(submenu1_counter==1){                                         
        lcd.setCursor(5,1);          
        lcd.cursor();
        delay(50);
        lcd.noCursor();
        delay(50);         
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 
            if(onhour < 23){
               onhour ++;
               rtc.SetDateTime(RtcDateTime(now.Year(), now.Month(), now.Day(), onhour, onmin, onsec));
            }
            else{
                onhour =0;
                rtc.SetDateTime(RtcDateTime(now.Year(), now.Month(), now.Day(), onhour, onmin, onsec));
            }
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
            if(onhour >0){
              onhour --; 
              rtc.SetDateTime(RtcDateTime(now.Year(), now.Month(), now.Day(), onhour, onmin, onsec));
            }
            else{
              onhour=23;
              rtc.SetDateTime(RtcDateTime(now.Year(), now.Month(), now.Day(), onhour, onmin, onsec));
            }
        }
        last_down=current_down;
    }
    lcd.setCursor(6,1);
    lcd.print(":");

     if(submenu1_counter==2){
        lcd.setCursor(8,1);         
        lcd.cursor();
        delay(50);
        lcd.noCursor();
        delay(50);       
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 
            if(onmin < 59){
               onmin ++;
               rtc.SetDateTime(RtcDateTime(now.Year(), now.Month(), now.Day(), onhour, onmin, onsec));
            }
            else{
               onmin =0;
               rtc.SetDateTime(RtcDateTime(now.Year(), now.Month(), now.Day(), onhour, onmin, onsec));
            }
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
          if(onmin >0){
            onmin --; 
            rtc.SetDateTime(RtcDateTime(now.Year(), now.Month(), now.Day(), onhour, onmin, onsec));
          }
          else{
            onmin=59;
            rtc.SetDateTime(RtcDateTime(now.Year(), now.Month(), now.Day(), onhour, onmin, onsec));
          }
        }
        last_down=current_down;
    }
    lcd.setCursor(9,1);
    lcd.print(":");
    
    if(submenu1_counter==3){
      lcd.setCursor(11,1);
      lcd.cursor();
      delay(50);
      lcd.noCursor();
      delay(50);                
      //Move item + or -
      if (last_up== LOW && current_up == HIGH){  //Up 
          if(onsec < 59){
            onsec ++;
            rtc.SetDateTime(RtcDateTime(now.Year(), now.Month(), now.Day(), onhour, onmin, onsec));
          }
          else{
            onsec = 0;
            rtc.SetDateTime(RtcDateTime(now.Year(), now.Month(), now.Day(), onhour, onmin, onsec));
        }
      }
      last_up=current_up;
      
      if(last_down== LOW && current_down == HIGH){//Down
        if(onsec >0){
          onsec --; 
          rtc.SetDateTime(RtcDateTime(now.Year(), now.Month(), now.Day(), onhour, onmin, onsec));
        }
        else{
          onsec=59;
          rtc.SetDateTime(RtcDateTime(now.Year(), now.Month(), now.Day(), onhour, onmin, onsec));
        }
      }
      last_down=current_down;
    }
   
    //Parmeter Saved
    if (last_save== LOW && current_save == HIGH){
        rtc.SetDateTime(RtcDateTime(now.Year(), now.Month(), now.Day(), onhour, onmin, onsec));

        lcd.clear();
        lcd.setCursor(5,1);
        lcd.print("SAVED!");
        delay(100);
        lcd.clear();
        submenu1_counter=0;
        menu_counter=1;
    }
    last_save = current_save;
    }
    break;
    case 3: {
      isChangedMenuCounter = 1;
      lcd.setCursor(0,0);
      lcd.print("DATE SET");

      lcd.setCursor(4,1);  
      if(onmonth<10){
        lcd.print("0");
      }
      lcd.print(onmonth);
      lcd.setCursor(7,1);
      if(onday<10){
        lcd.print("0");
      }
      lcd.print(onday);

      lcd.setCursor(10,1);
     
      lcd.print(onyear,DEC);
      
     if (last_sel== LOW && current_sel == HIGH){ 
        if(submenu1_counter <3){                    
             submenu1_counter ++;                         
          }
        else{                                       
             submenu1_counter=1;
           }
     }
     last_sel=current_sel;                      

     if(submenu1_counter==1){                                         
        lcd.setCursor(5,1);          
        lcd.cursor();
        delay(50);
        lcd.noCursor();
        delay(50);         
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 
            if(onmonth < 12){
               onmonth ++;
               rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
            }
            else{
                onmonth =1;
                rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
            }
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
            if(onmonth >1){
              onmonth --; 
              rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
            }
            else{
              onmonth=12;
              rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
            }
        }
        last_down=current_down;
    }
    lcd.setCursor(6,1);
    lcd.print(":");

     if(submenu1_counter==2){
        lcd.setCursor(8,1);         
        lcd.cursor();
        delay(50);
        lcd.noCursor();
        delay(50);     
        onyear=now.Year();
        onmonth = now.Month();
        onday = now.Day();
          
        
        //Move item + or -
        if(onyear%4==0){
          if(onmonth == 2){
            if(onday>29)onday=29;
            rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
            if (last_up== LOW && current_up == HIGH){  //Up 
            if(onday < 29){
               onday ++;
               rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
            }
            else{
               onday =1;
               rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
            }
            
        }
            last_up=current_up;
            if(last_down== LOW && current_down == HIGH){//Down
              if(onday >1){
                onday --; 
                rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
              }
              else{
                onday=29;
                rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
              }
            }
            last_down=current_down;
          }

           else if(onmonth==4 || onmonth ==6 || onmonth == 9 || onmonth == 11 ){
            if(onday>30)onday=30;
            rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
              if (last_up== LOW && current_up == HIGH){  //Up 
            if(onday < 30){
               onday ++;
               rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
            }
            else{
               onday =1;
               rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
            }
            
        }
            last_up=current_up;
            if(last_down== LOW && current_down == HIGH){//Down
              if(onday >1){
                onday --; 
                rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
              }
              else{
                onday=30;
                rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
              }
            }
            last_down=current_down;
          }

          else{
              if (last_up== LOW && current_up == HIGH){  //Up 
              
            if(onday < 31){
               onday ++;
               rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
            }
            else{
               onday =1;
               rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
            }
            
        }
            last_up=current_up;
            if(last_down== LOW && current_down == HIGH){//Down
              if(onday >1){
                onday --; 
                rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
              }
              else{
                onday=31;
                rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
              }
            }
            last_down=current_down;
          }
            
        }
        else{
          if(onmonth == 2){
            if(onday>28)onday=28;
            rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
              if (last_up== LOW && current_up == HIGH){  //Up 
            if(onday < 28){
               onday ++;
               rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
            }
            else{
               onday =1;
               rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
            }
            
        }
            last_up=current_up;
            if(last_down== LOW && current_down == HIGH){//Down
              if(onday >1){
                onday --; 
                rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
              }
              else{
                onday=28;
                rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
              }
            }
            last_down=current_down;
          }

           else if(onmonth==4 || onmonth ==6 || onmonth == 9 || onmonth == 11 ){
            if(onday>30)onday=30;
            rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
              if (last_up== LOW && current_up == HIGH){  //Up 
            if(onday < 30){
               onday ++;
               rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
            }
            else{
               onday =1;
               rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
            }
            
        }
            last_up=current_up;
            if(last_down== LOW && current_down == HIGH){//Down
              if(onday >1){
                onday --; 
                rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
              }
              else{
                onday=30;
                rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
              }
            }
            last_down=current_down;
          }

          else{
              if (last_up== LOW && current_up == HIGH){  //Up 
            if(onday < 31){
               onday ++;
               rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
            }
            else{
               onday =1;
               rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
            }
            
        }
            last_up=current_up;
            if(last_down== LOW && current_down == HIGH){//Down
              if(onday >1){
                onday --; 
                rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
              }
              else{
                onday=31;
                rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
              }
            }
            last_down=current_down;
          }
            
        }
    }
    lcd.setCursor(9,1);
    lcd.print(":");
    
    if(submenu1_counter==3){
      lcd.setCursor(13,1);
      lcd.cursor();
      delay(50);
      lcd.noCursor();
      delay(50);                
      //Move item + or -
      if (last_up== LOW && current_up == HIGH){  //Up 
          if(onyear < 2050){
            onyear ++;
            rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
          }
          else{
            onyear = 2023;
            rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
        }
      }
      last_up=current_up;
      
      if(last_down== LOW && current_down == HIGH){//Down
        if(onyear >2023){
          onyear --; 
          rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
        }
        else{
          onyear=2050;
          rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));
        }
      }
      last_down=current_down;
    }
   
    //Parmeter Saved
    if (last_save== LOW && current_save == HIGH){
        rtc.SetDateTime(RtcDateTime(onyear, onmonth, onday, onhour, onmin, onsec));

        lcd.clear();
        lcd.setCursor(5,1);
        lcd.print("SAVED!");
        delay(100);
        lcd.clear();
        submenu1_counter=0;
        menu_counter=1;
    }
    last_save = current_save;
    }
    break;
  
    case 6:{
      isChangedMenuCounter = 1;
      
      if(count_for_autoBulb_indiv[0] == 0) {
        if(!(submenu1_counter==1))
        strcpy(bulb_status_indiv[0], "Manual");
      }
         else{
          if(!(submenu1_counter==1)) strcpy(bulb_status_indiv[0], "Auto  ");
         } 
      if(count_for_autoBulb_indiv[1] == 0) {
        if(!(submenu1_counter==2))
        strcpy(bulb_status_indiv[1], "Manual");
      }
         else{
          if(!(submenu1_counter==2)) strcpy(bulb_status_indiv[1], "Auto  ");
         } 
      if(count_for_autoBulb_indiv[2] == 0) {
        if(!(submenu1_counter==3))
        strcpy(bulb_status_indiv[2], "Manual");
      }
         else{
          if(!(submenu1_counter==3)) strcpy(bulb_status_indiv[2], "Auto  ");
         } 
      if(count_for_autoBulb_indiv[3] == 0) {
        if(!(submenu1_counter==4))
        strcpy(bulb_status_indiv[3], "Manual");
      }
         else{
          if(!(submenu1_counter==4)) strcpy(bulb_status_indiv[3], "Auto  ");
         } 
  
      lcd.setCursor(0,0);
      lcd.print("BULB 1");
      lcd.setCursor(6,0);
      lcd.print(":");
      lcd.setCursor(8,0);
      lcd.print(bulb_status_indiv[0]);

      lcd.setCursor(0,1);
      lcd.print("BULB 2");
      lcd.setCursor(6,1);
      lcd.print(":");
      lcd.setCursor(8,1);
      lcd.print(bulb_status_indiv[1]);

      lcd.setCursor(0,2);
      lcd.print("BULB 3");
      lcd.setCursor(6,2);
      lcd.print(":");
      lcd.setCursor(8,2);
      lcd.print(bulb_status_indiv[2]);

      lcd.setCursor(0,3);
      lcd.print("BULB 4");
      lcd.setCursor(6,3);
      lcd.print(":");
      lcd.setCursor(8,3);
      lcd.print(bulb_status_indiv[3]);

      if (last_sel== LOW && current_sel == HIGH){ 
        if(submenu1_counter <4){                    
             submenu1_counter ++;                         
          }
        else{                                       
             submenu1_counter=1;
           }
     }
     last_sel=current_sel;  

      if(submenu1_counter==1){                                         
        if(count_for_autoBulb_indiv[0]==0){
            strcpy(bulb_status_indiv[0], "MANUAL"); 
        }
        else {
          strcpy(bulb_status_indiv[0], "AUTO  ");
        }
        
        
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 

            
            if(count_for_autoBulb_indiv[0]==0) count_for_autoBulb_indiv[0] = 1;
            else count_for_autoBulb_indiv[0] = 0;
            EEPROM.write(11, count_for_autoBulb_indiv[0]);EEPROM.commit();
           
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
            if(count_for_autoBulb_indiv[0]==0) count_for_autoBulb_indiv[0] = 1;
            else count_for_autoBulb_indiv[0] = 0;
            EEPROM.write(11, count_for_autoBulb_indiv[0]);EEPROM.commit();
        }
        last_down=current_down;
    }
    if(submenu1_counter==2){                                         
        
        if(count_for_autoBulb_indiv[1]==0){
            strcpy(bulb_status_indiv[1], "MANUAL");   
            
        
        }
        else {
          strcpy(bulb_status_indiv[1], "AUTO  ");
        }       
        
        
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 

            
            if(count_for_autoBulb_indiv[1]==0) count_for_autoBulb_indiv[1] = 1;
            else count_for_autoBulb_indiv[1] = 0;
            EEPROM.write(12, count_for_autoBulb_indiv[1]);EEPROM.commit();
           
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
            if(count_for_autoBulb_indiv[1]==0) count_for_autoBulb_indiv[1] = 1;
            else count_for_autoBulb_indiv[1] = 0;
            EEPROM.write(12, count_for_autoBulb_indiv[1]);EEPROM.commit();
        }
        last_down=current_down;
    }
    if(submenu1_counter==3){                                         
        
        if(count_for_autoBulb_indiv[2]==0){
            strcpy(bulb_status_indiv[2], "MANUAL");    
        
        }
        else {
          strcpy(bulb_status_indiv[2], "AUTO  ");
        }       
        
        
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 

            
            if(count_for_autoBulb_indiv[2]==0) count_for_autoBulb_indiv[2] = 1;
            else count_for_autoBulb_indiv[2] = 0;
            EEPROM.write(13, count_for_autoBulb_indiv[2]);EEPROM.commit();
           
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
            if(count_for_autoBulb_indiv[2]==0) count_for_autoBulb_indiv[2] = 1;
            else count_for_autoBulb_indiv[2] = 0;
            EEPROM.write(13, count_for_autoBulb_indiv[2]);EEPROM.commit();
        }
        last_down=current_down;
    }
    if(submenu1_counter==4){                                         
        if(count_for_autoBulb_indiv[3]==0){
        strcpy(bulb_status_indiv[3], "MANUAL");   
        
        }
        else {
          strcpy(bulb_status_indiv[3], "AUTO  ");
        }       
        
        
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 

            
            if(count_for_autoBulb_indiv[3]==0) count_for_autoBulb_indiv[3] = 1;
            else count_for_autoBulb_indiv[3] = 0;
            EEPROM.write(14, count_for_autoBulb_indiv[3]);EEPROM.commit();
           
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
            if(count_for_autoBulb_indiv[3]==0) count_for_autoBulb_indiv[3] = 1;
            else count_for_autoBulb_indiv[3] = 0;
            EEPROM.write(14, count_for_autoBulb_indiv[3]);EEPROM.commit();
        }
        last_down=current_down;
    }
     if (last_save== LOW && current_save == HIGH){
        lcd.clear();
        lcd.setCursor(5,1);
        lcd.print("SAVED!");
        delay(100);
        lcd.clear();
        submenu1_counter=0;
    
         if(count_for_autoBulb_indiv[0] == 1) menu_counter = 8;
        else if(count_for_autoBulb_indiv[1] == 1) menu_counter = 9;
        else if(count_for_autoBulb_indiv[2] == 1) menu_counter = 10;
        else if(count_for_autoBulb_indiv[3] == 1) menu_counter = 11;
        else menu_counter = 1;
       

    }
    last_save = current_save;
    }
    break;

    case 8:{
      isChangedMenuCounter = 1;
      lcd.setCursor(0,0);
      lcd.print("OPEN TIME FOR BULB 1");

      lcd.setCursor(4,1);  
      if(bulb1_setTime_indiv[0]<10){
        lcd.print("0");
      }
      lcd.print(bulb1_setTime_indiv[0]);
      lcd.setCursor(7,1);
      if(bulb1_setTime_indiv[1]<10){
        lcd.print("0");
      }
      lcd.print(bulb1_setTime_indiv[1]);
      lcd.setCursor(10,1);
      if(bulb1_setTime_indiv[2]<10){
        lcd.print("0");
      }
      lcd.print(bulb1_setTime_indiv[2]);

      lcd.setCursor(0,2);
      lcd.print("CLOSE TIME FOR BULB1");

      lcd.setCursor(4,3);  
      if(bulb1_setTime_indiv[3]<10){
        lcd.print("0");
      }
      lcd.print(bulb1_setTime_indiv[3]);
      lcd.setCursor(7,3);
      if(bulb1_setTime_indiv[4]<10){
        lcd.print("0");
      }
      lcd.print(bulb1_setTime_indiv[4]);
      lcd.setCursor(10,3);
      if(bulb1_setTime_indiv[5]<10){
        lcd.print("0");
      }
      lcd.print(bulb1_setTime_indiv[5]);
      
     if (last_sel== LOW && current_sel == HIGH){ 
        if(submenu1_counter <6){                    
             submenu1_counter ++;                         
          }
        else{                                       
             submenu1_counter=1;
           }
     }
     last_sel=current_sel;                      

     if(submenu1_counter==1){                                         
        lcd.setCursor(5,1);          
        lcd.cursor();
        delay(50);
        lcd.noCursor();
        delay(50);         
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 
            if(bulb1_setTime_indiv[0] < 23){
               bulb1_setTime_indiv[0] ++;

            }
            else{
                bulb1_setTime_indiv[0] =0;   
              
            }
            EEPROM.write(15, bulb1_setTime_indiv[0]);EEPROM.commit();
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
            if(bulb1_setTime_indiv[0] >0){
              bulb1_setTime_indiv[0] --; 
             
       
            }
            else{
              bulb1_setTime_indiv[0]=23;
            
            }
             EEPROM.write(15, bulb1_setTime_indiv[0]);EEPROM.commit();
        }
        last_down=current_down;
    }
    lcd.setCursor(6,1);
    lcd.print(":");

     if(submenu1_counter==2){
        lcd.setCursor(8,1);         
        lcd.cursor();
        delay(50);
        lcd.noCursor();
        delay(50);       
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 
            if(bulb1_setTime_indiv[1] < 59){
               bulb1_setTime_indiv[1] ++;
           
            }
            else{
               bulb1_setTime_indiv[1] =0;
           
            }
             EEPROM.write(16, bulb1_setTime_indiv[1]);EEPROM.commit();
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
          if(bulb1_setTime_indiv[1] >0){
            bulb1_setTime_indiv[1] --; 
        
          }
          else{
            bulb1_setTime_indiv[1]=59;
     
          }
          EEPROM.write(16, bulb1_setTime_indiv[1]);EEPROM.commit();
        }
        last_down=current_down;
    }
    lcd.setCursor(9,1);
    lcd.print(":");
    
    if(submenu1_counter==3){
      lcd.setCursor(11,1);
      lcd.cursor();
      delay(50);
      lcd.noCursor();
      delay(50);                
      //Move item + or -
      if (last_up== LOW && current_up == HIGH){  //Up 
          if(bulb1_setTime_indiv[2] < 59){
            bulb1_setTime_indiv[2] ++;
       
          }
          else{
            bulb1_setTime_indiv[2] = 0;
           
        }
        EEPROM.write(17, bulb1_setTime_indiv[2]);EEPROM.commit();
      }
      last_up=current_up;
      
      if(last_down== LOW && current_down == HIGH){//Down
        if(bulb1_setTime_indiv[2] >0){
          bulb1_setTime_indiv[2] --; 
       
        }
        else{
          bulb1_setTime_indiv[2]=59;
       
        }
        EEPROM.write(17, bulb1_setTime_indiv[2]);EEPROM.commit();
      }
      last_down=current_down;
    }
    if(submenu1_counter==4){                                         
        lcd.setCursor(5,3);          
        lcd.cursor();
        delay(50);
        lcd.noCursor();
        delay(50);         
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 
            if(bulb1_setTime_indiv[3] < 23){
               bulb1_setTime_indiv[3] ++;
            
            }
            else{
                bulb1_setTime_indiv[3] =0;
              
            }
            EEPROM.write(18, bulb1_setTime_indiv[3]);EEPROM.commit();
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
            if(bulb1_setTime_indiv[3] >0){
              bulb1_setTime_indiv[3] --; 
         
            }
            else{
              bulb1_setTime_indiv[3]=23;
        
            }
            EEPROM.write(18, bulb1_setTime_indiv[3]);EEPROM.commit();
        }
        last_down=current_down;
    }
    lcd.setCursor(6,3);
    lcd.print(":");

     if(submenu1_counter==5){
        lcd.setCursor(8,3);         
        lcd.cursor();
        delay(50);
        lcd.noCursor();
        delay(50);       
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 
            if(bulb1_setTime_indiv[4] < 59){
               bulb1_setTime_indiv[4] ++;
         
            }
            else{
               bulb1_setTime_indiv[4] =0;
           
            }
            EEPROM.write(19, bulb1_setTime_indiv[4]);EEPROM.commit();
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
          if(bulb1_setTime_indiv[4] >0){
            bulb1_setTime_indiv[4] --; 
      
          }
          else{
            bulb1_setTime_indiv[4]=59;
        
          }
          EEPROM.write(19, bulb1_setTime_indiv[4]);EEPROM.commit();
        }
        last_down=current_down;
    }
    lcd.setCursor(9,3);
    lcd.print(":");
    
    if(submenu1_counter==6){
      lcd.setCursor(11,3);
      lcd.cursor();
      delay(50);
      lcd.noCursor();
      delay(50);                
      //Move item + or -
      if (last_up== LOW && current_up == HIGH){  //Up 
          if(bulb1_setTime_indiv[5] < 59){
            bulb1_setTime_indiv[5] ++;
         
          }
          else{
            bulb1_setTime_indiv[5] = 0;
      
        }
        EEPROM.write(20, bulb1_setTime_indiv[5]);EEPROM.commit();
      }
      last_up=current_up;
      
      if(last_down== LOW && current_down == HIGH){//Down
        if(bulb1_setTime_indiv[5] >0){
          bulb1_setTime_indiv[5] --; 
    
        }
        else{
          bulb1_setTime_indiv[5]=59;
      
        }
        EEPROM.write(20, bulb1_setTime_indiv[5]);EEPROM.commit();
      }
      last_down=current_down;
    }
   
    //Parmeter Saved
    if (last_save== LOW && current_save == HIGH){
        lcd.clear();
        lcd.setCursor(5,1);
        lcd.print("SAVED!");
        delay(100);
        lcd.clear();
        Serial.println("State saved in flash memory");
        submenu1_counter=0;
     
        if(count_for_autoBulb_indiv[1] == 1) menu_counter = 9;
        else if(count_for_autoBulb_indiv[2] == 1) menu_counter = 10;
        else if(count_for_autoBulb_indiv[3] == 1) menu_counter = 11;
        else menu_counter = 1;
        
    }
    last_save = current_save;
    }
    break;

    case 9:{
      isChangedMenuCounter = 1;
      lcd.setCursor(0,0);
      lcd.print("OPEN TIME FOR BULB 2");

      lcd.setCursor(4,1);  
      if(bulb2_setTime_indiv[0]<10){
        lcd.print("0");
      }
      lcd.print(bulb2_setTime_indiv[0]);
      lcd.setCursor(7,1);
      if(bulb2_setTime_indiv[1]<10){
        lcd.print("0");
      }
      lcd.print(bulb2_setTime_indiv[1]);
      lcd.setCursor(10,1);
      if(bulb2_setTime_indiv[2]<10){
        lcd.print("0");
      }
      lcd.print(bulb2_setTime_indiv[2]);

      lcd.setCursor(0,2);
      lcd.print("CLOSE TIME FOR BULB2");

      lcd.setCursor(4,3);  
      if(bulb2_setTime_indiv[3]<10){
        lcd.print("0");
      }
      lcd.print(bulb2_setTime_indiv[3]);
      lcd.setCursor(7,3);
      if(bulb2_setTime_indiv[4]<10){
        lcd.print("0");
      }
      lcd.print(bulb2_setTime_indiv[4]);
      lcd.setCursor(10,3);
      if(bulb2_setTime_indiv[5]<10){
        lcd.print("0");
      }
      lcd.print(bulb2_setTime_indiv[5]);
      
     if (last_sel== LOW && current_sel == HIGH){ 
        if(submenu1_counter <6){                    
             submenu1_counter ++;                         
          }
        else{                                       
             submenu1_counter=1;
           }
     }
     last_sel=current_sel;                      

     if(submenu1_counter==1){                                         
        lcd.setCursor(5,1);          
        lcd.cursor();
        delay(50);
        lcd.noCursor();
        delay(50);         
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 
            if(bulb2_setTime_indiv[0] < 23){
               bulb2_setTime_indiv[0] ++;

            }
            else{
                bulb2_setTime_indiv[0] =0;   
              
            }
            EEPROM.write(21, bulb2_setTime_indiv[0]);EEPROM.commit();
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
            if(bulb2_setTime_indiv[0] >0){
              bulb2_setTime_indiv[0] --; 
             
       
            }
            else{
              bulb2_setTime_indiv[0]=23;
            
            }
             EEPROM.write(21, bulb2_setTime_indiv[0]);EEPROM.commit();
        }
        last_down=current_down;
    }
    lcd.setCursor(6,1);
    lcd.print(":");

     if(submenu1_counter==2){
        lcd.setCursor(8,1);         
        lcd.cursor();
        delay(50);
        lcd.noCursor();
        delay(50);       
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 
            if(bulb2_setTime_indiv[1] < 59){
               bulb2_setTime_indiv[1] ++;
           
            }
            else{
               bulb2_setTime_indiv[1] =0;
           
            }
             EEPROM.write(22, bulb2_setTime_indiv[1]);EEPROM.commit();
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
          if(bulb2_setTime_indiv[1] >0){
            bulb2_setTime_indiv[1] --; 
        
          }
          else{
            bulb2_setTime_indiv[1]=59;
     
          }
          EEPROM.write(22, bulb2_setTime_indiv[1]);EEPROM.commit();
        }
        last_down=current_down;
    }
    lcd.setCursor(9,1);
    lcd.print(":");
    
    if(submenu1_counter==3){
      lcd.setCursor(11,1);
      lcd.cursor();
      delay(50);
      lcd.noCursor();
      delay(50);                
      //Move item + or -
      if (last_up== LOW && current_up == HIGH){  //Up 
          if(bulb2_setTime_indiv[2] < 59){
            bulb2_setTime_indiv[2] ++;
       
          }
          else{
            bulb2_setTime_indiv[2] = 0;
           
        }
        EEPROM.write(23, bulb2_setTime_indiv[2]);EEPROM.commit();
      }
      last_up=current_up;
      
      if(last_down== LOW && current_down == HIGH){//Down
        if(bulb2_setTime_indiv[2] >0){
          bulb2_setTime_indiv[2] --; 
       
        }
        else{
          bulb2_setTime_indiv[2]=59;
       
        }
        EEPROM.write(23, bulb2_setTime_indiv[2]);EEPROM.commit();
      }
      last_down=current_down;
    }
    if(submenu1_counter==4){                                         
        lcd.setCursor(5,3);          
        lcd.cursor();
        delay(50);
        lcd.noCursor();
        delay(50);         
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 
            if(bulb2_setTime_indiv[3] < 23){
               bulb2_setTime_indiv[3] ++;
            
            }
            else{
                bulb2_setTime_indiv[3] =0;
              
            }
            EEPROM.write(24, bulb2_setTime_indiv[3]);EEPROM.commit();
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
            if(bulb2_setTime_indiv[3] >0){
              bulb2_setTime_indiv[3] --; 
         
            }
            else{
              bulb2_setTime_indiv[3]=23;
        
            }
            EEPROM.write(24, bulb2_setTime_indiv[3]);EEPROM.commit();
        }
        last_down=current_down;
    }
    lcd.setCursor(6,3);
    lcd.print(":");

     if(submenu1_counter==5){
        lcd.setCursor(8,3);         
        lcd.cursor();
        delay(50);
        lcd.noCursor();
        delay(50);       
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 
            if(bulb2_setTime_indiv[4] < 59){
               bulb2_setTime_indiv[4] ++;
         
            }
            else{
               bulb2_setTime_indiv[4] =0;
           
            }
            EEPROM.write(25, bulb2_setTime_indiv[4]);EEPROM.commit();
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
          if(bulb2_setTime_indiv[4] >0){
            bulb2_setTime_indiv[4] --; 
      
          }
          else{
            bulb2_setTime_indiv[4]=59;
        
          }
          EEPROM.write(25, bulb2_setTime_indiv[4]);EEPROM.commit();
        }
        last_down=current_down;
    }
    lcd.setCursor(9,3);
    lcd.print(":");
    
    if(submenu1_counter==6){
      lcd.setCursor(11,3);
      lcd.cursor();
      delay(50);
      lcd.noCursor();
      delay(50);                
      //Move item + or -
      if (last_up== LOW && current_up == HIGH){  //Up 
          if(bulb2_setTime_indiv[5] < 59){
            bulb2_setTime_indiv[5] ++;
         
          }
          else{
            bulb2_setTime_indiv[5] = 0;
      
        }
        EEPROM.write(26, bulb2_setTime_indiv[5]);EEPROM.commit();
      }
      last_up=current_up;
      
      if(last_down== LOW && current_down == HIGH){//Down
        if(bulb2_setTime_indiv[5] >0){
          bulb2_setTime_indiv[5] --; 
    
        }
        else{
          bulb2_setTime_indiv[5]=59;
      
        }
        EEPROM.write(26, bulb2_setTime_indiv[5]);EEPROM.commit();
      }
      last_down=current_down;
    }
   
    //Parmeter Saved
    if (last_save== LOW && current_save == HIGH){
        lcd.clear();
        lcd.setCursor(5,1);
        lcd.print("SAVED!");
        delay(100);
        lcd.clear();
        Serial.println("State saved in flash memory");
        submenu1_counter=0;
        if(count_for_autoBulb_indiv[2] == 1) menu_counter = 10;
        else if(count_for_autoBulb_indiv[3] == 1) menu_counter = 11;
        else menu_counter = 1;
        
    }
    last_save = current_save;
    }
    break;
    case 10:{
      isChangedMenuCounter = 1;
      lcd.setCursor(0,0);
      lcd.print("OPEN TIME FOR BULB 3");

      lcd.setCursor(4,1);  
      if(bulb3_setTime_indiv[0]<10){
        lcd.print("0");
      }
      lcd.print(bulb3_setTime_indiv[0]);
      lcd.setCursor(7,1);
      if(bulb3_setTime_indiv[1]<10){
        lcd.print("0");
      }
      lcd.print(bulb3_setTime_indiv[1]);
      lcd.setCursor(10,1);
      if(bulb3_setTime_indiv[2]<10){
        lcd.print("0");
      }
      lcd.print(bulb3_setTime_indiv[2]);

      lcd.setCursor(0,2);
      lcd.print("CLOSE TIME FOR BULB3");

      lcd.setCursor(4,3);  
      if(bulb3_setTime_indiv[3]<10){
        lcd.print("0");
      }
      lcd.print(bulb3_setTime_indiv[3]);
      lcd.setCursor(7,3);
      if(bulb3_setTime_indiv[4]<10){
        lcd.print("0");
      }
      lcd.print(bulb3_setTime_indiv[4]);
      lcd.setCursor(10,3);
      if(bulb3_setTime_indiv[5]<10){
        lcd.print("0");
      }
      lcd.print(bulb3_setTime_indiv[5]);
      
     if (last_sel== LOW && current_sel == HIGH){ 
        if(submenu1_counter <6){                    
             submenu1_counter ++;                         
          }
        else{                                       
             submenu1_counter=1;
           }
     }
     last_sel=current_sel;                      

     if(submenu1_counter==1){                                         
        lcd.setCursor(5,1);          
        lcd.cursor();
        delay(50);
        lcd.noCursor();
        delay(50);         
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 
            if(bulb3_setTime_indiv[0] < 23){
               bulb3_setTime_indiv[0] ++;

            }
            else{
                bulb3_setTime_indiv[0] =0;   
              
            }
            EEPROM.write(27, bulb3_setTime_indiv[0]);EEPROM.commit();
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
            if(bulb3_setTime_indiv[0] >0){
              bulb3_setTime_indiv[0] --; 
             
       
            }
            else{
              bulb3_setTime_indiv[0]=23;
            
            }
             EEPROM.write(27, bulb3_setTime_indiv[0]);EEPROM.commit();
        }
        last_down=current_down;
    }
    lcd.setCursor(6,1);
    lcd.print(":");

     if(submenu1_counter==2){
        lcd.setCursor(8,1);         
        lcd.cursor();
        delay(50);
        lcd.noCursor();
        delay(50);       
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 
            if(bulb3_setTime_indiv[1] < 59){
               bulb3_setTime_indiv[1] ++;
           
            }
            else{
               bulb3_setTime_indiv[1] =0;
           
            }
             EEPROM.write(28, bulb3_setTime_indiv[1]);EEPROM.commit();
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
          if(bulb3_setTime_indiv[1] >0){
            bulb3_setTime_indiv[1] --; 
        
          }
          else{
            bulb3_setTime_indiv[1]=59;
     
          }
          EEPROM.write(28, bulb3_setTime_indiv[1]);EEPROM.commit();
        }
        last_down=current_down;
    }
    lcd.setCursor(9,1);
    lcd.print(":");
    
    if(submenu1_counter==3){
      lcd.setCursor(11,1);
      lcd.cursor();
      delay(50);
      lcd.noCursor();
      delay(50);                
      //Move item + or -
      if (last_up== LOW && current_up == HIGH){  //Up 
          if(bulb3_setTime_indiv[2] < 59){
            bulb3_setTime_indiv[2] ++;
       
          }
          else{
            bulb3_setTime_indiv[2] = 0;
           
        }
        EEPROM.write(29, bulb3_setTime_indiv[2]);EEPROM.commit();
      }
      last_up=current_up;
      
      if(last_down== LOW && current_down == HIGH){//Down
        if(bulb3_setTime_indiv[2] >0){
          bulb3_setTime_indiv[2] --; 
       
        }
        else{
          bulb3_setTime_indiv[2]=59;
       
        }
        EEPROM.write(29, bulb3_setTime_indiv[2]);EEPROM.commit();
      }
      last_down=current_down;
    }
    if(submenu1_counter==4){                                         
        lcd.setCursor(5,3);          
        lcd.cursor();
        delay(50);
        lcd.noCursor();
        delay(50);         
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 
            if(bulb3_setTime_indiv[3] < 23){
               bulb3_setTime_indiv[3] ++;
            
            }
            else{
                bulb3_setTime_indiv[3] =0;
              
            }
            EEPROM.write(30, bulb3_setTime_indiv[3]);EEPROM.commit();
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
            if(bulb3_setTime_indiv[3] >0){
              bulb3_setTime_indiv[3] --; 
         
            }
            else{
              bulb3_setTime_indiv[3]=23;
        
            }
            EEPROM.write(30, bulb3_setTime_indiv[3]);EEPROM.commit();
        }
        last_down=current_down;
    }
    lcd.setCursor(6,3);
    lcd.print(":");

     if(submenu1_counter==5){
        lcd.setCursor(8,3);         
        lcd.cursor();
        delay(50);
        lcd.noCursor();
        delay(50);       
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 
            if(bulb3_setTime_indiv[4] < 59){
               bulb3_setTime_indiv[4] ++;
         
            }
            else{
               bulb3_setTime_indiv[4] =0;
           
            }
            EEPROM.write(31, bulb3_setTime_indiv[4]);EEPROM.commit();
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
          if(bulb3_setTime_indiv[4] >0){
            bulb3_setTime_indiv[4] --; 
      
          }
          else{
            bulb3_setTime_indiv[4]=59;
        
          }
          EEPROM.write(31, bulb3_setTime_indiv[4]);EEPROM.commit();
        }
        last_down=current_down;
    }
    lcd.setCursor(9,3);
    lcd.print(":");
    
    if(submenu1_counter==6){
      lcd.setCursor(11,3);
      lcd.cursor();
      delay(50);
      lcd.noCursor();
      delay(50);                
      //Move item + or -
      if (last_up== LOW && current_up == HIGH){  //Up 
          if(bulb3_setTime_indiv[5] < 59){
            bulb3_setTime_indiv[5] ++;
         
          }
          else{
            bulb3_setTime_indiv[5] = 0;
      
        }
        EEPROM.write(32, bulb3_setTime_indiv[5]);EEPROM.commit();
      }
      last_up=current_up;
      
      if(last_down== LOW && current_down == HIGH){//Down
        if(bulb3_setTime_indiv[5] >0){
          bulb3_setTime_indiv[5] --; 
    
        }
        else{
          bulb3_setTime_indiv[5]=59;
      
        }
        EEPROM.write(32, bulb3_setTime_indiv[5]);EEPROM.commit();
      }
      last_down=current_down;
    }
   
    //Parmeter Saved
    if (last_save== LOW && current_save == HIGH){
        lcd.clear();
        lcd.setCursor(5,1);
        lcd.print("SAVED!");
        delay(100);
        lcd.clear();
        Serial.println("State saved in flash memory");
        submenu1_counter=0;
      
       if(count_for_autoBulb_indiv[3] == 1) menu_counter = 11;
      else menu_counter = 1;
        
    }
    last_save = current_save;
    }
    break;
    case 11:{
      isChangedMenuCounter = 1;
      lcd.setCursor(0,0);
      lcd.print("OPEN TIME FOR BULB 4");

      lcd.setCursor(4,1);  
      if(bulb4_setTime_indiv[0]<10){
        lcd.print("0");
      }
      lcd.print(bulb4_setTime_indiv[0]);
      lcd.setCursor(7,1);
      if(bulb4_setTime_indiv[1]<10){
        lcd.print("0");
      }
      lcd.print(bulb4_setTime_indiv[1]);
      lcd.setCursor(10,1);
      if(bulb4_setTime_indiv[2]<10){
        lcd.print("0");
      }
      lcd.print(bulb4_setTime_indiv[2]);

      lcd.setCursor(0,2);
      lcd.print("CLOSE TIME FOR BULB4");

      lcd.setCursor(4,3);  
      if(bulb4_setTime_indiv[3]<10){
        lcd.print("0");
      }
      lcd.print(bulb4_setTime_indiv[3]);
      lcd.setCursor(7,3);
      if(bulb4_setTime_indiv[4]<10){
        lcd.print("0");
      }
      lcd.print(bulb4_setTime_indiv[4]);
      lcd.setCursor(10,3);
      if(bulb4_setTime_indiv[5]<10){
        lcd.print("0");
      }
      lcd.print(bulb4_setTime_indiv[5]);
      
     if (last_sel== LOW && current_sel == HIGH){ 
        if(submenu1_counter <6){                    
             submenu1_counter ++;                         
          }
        else{                                       
             submenu1_counter=1;
           }
     }
     last_sel=current_sel;                      

     if(submenu1_counter==1){                                         
        lcd.setCursor(5,1);          
        lcd.cursor();
        delay(50);
        lcd.noCursor();
        delay(50);         
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 
            if(bulb4_setTime_indiv[0] < 23){
               bulb4_setTime_indiv[0] ++;

            }
            else{
                bulb4_setTime_indiv[0] =0;   
              
            }
            EEPROM.write(33, bulb4_setTime_indiv[0]);EEPROM.commit();
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
            if(bulb4_setTime_indiv[0] >0){
              bulb4_setTime_indiv[0] --; 
             
       
            }
            else{
              bulb4_setTime_indiv[0]=23;
            
            }
             EEPROM.write(33, bulb4_setTime_indiv[0]);EEPROM.commit();
        }
        last_down=current_down;
    }
    lcd.setCursor(6,1);
    lcd.print(":");

     if(submenu1_counter==2){
        lcd.setCursor(8,1);         
        lcd.cursor();
        delay(50);
        lcd.noCursor();
        delay(50);       
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 
            if(bulb4_setTime_indiv[1] < 59){
               bulb4_setTime_indiv[1] ++;
           
            }
            else{
               bulb4_setTime_indiv[1] =0;
           
            }
             EEPROM.write(34, bulb4_setTime_indiv[1]);EEPROM.commit();
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
          if(bulb4_setTime_indiv[1] >0){
            bulb4_setTime_indiv[1] --; 
        
          }
          else{
            bulb4_setTime_indiv[1]=59;
     
          }
          EEPROM.write(34, bulb4_setTime_indiv[1]);EEPROM.commit();
        }
        last_down=current_down;
    }
    lcd.setCursor(9,1);
    lcd.print(":");
    
    if(submenu1_counter==3){
      lcd.setCursor(11,1);
      lcd.cursor();
      delay(50);
      lcd.noCursor();
      delay(50);                
      //Move item + or -
      if (last_up== LOW && current_up == HIGH){  //Up 
          if(bulb4_setTime_indiv[2] < 59){
            bulb4_setTime_indiv[2] ++;
       
          }
          else{
            bulb4_setTime_indiv[2] = 0;
           
        }
        EEPROM.write(35, bulb4_setTime_indiv[2]);EEPROM.commit();
      }
      last_up=current_up;
      
      if(last_down== LOW && current_down == HIGH){//Down
        if(bulb4_setTime_indiv[2] >0){
          bulb4_setTime_indiv[2] --; 
       
        }
        else{
          bulb4_setTime_indiv[2]=59;
       
        }
        EEPROM.write(35, bulb4_setTime_indiv[2]);EEPROM.commit();
      }
      last_down=current_down;
    }
    if(submenu1_counter==4){                                         
        lcd.setCursor(5,3);          
        lcd.cursor();
        delay(50);
        lcd.noCursor();
        delay(50);         
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 
            if(bulb4_setTime_indiv[3] < 23){
               bulb4_setTime_indiv[3] ++;
            
            }
            else{
                bulb4_setTime_indiv[3] =0;
              
            }
            EEPROM.write(36, bulb4_setTime_indiv[3]);EEPROM.commit();
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
            if(bulb4_setTime_indiv[3] >0){
              bulb4_setTime_indiv[3] --; 
         
            }
            else{
              bulb4_setTime_indiv[3]=23;
        
            }
            EEPROM.write(36, bulb4_setTime_indiv[3]);EEPROM.commit();
        }
        last_down=current_down;
    }
    lcd.setCursor(6,3);
    lcd.print(":");

     if(submenu1_counter==5){
        lcd.setCursor(8,3);         
        lcd.cursor();
        delay(50);
        lcd.noCursor();
        delay(50);       
        //Move item + or -
        if (last_up== LOW && current_up == HIGH){  //Up 
            if(bulb4_setTime_indiv[4] < 59){
               bulb4_setTime_indiv[4] ++;
         
            }
            else{
               bulb4_setTime_indiv[4] =0;
           
            }
            EEPROM.write(37, bulb4_setTime_indiv[4]);EEPROM.commit();
        }
        last_up=current_up;
        
        if(last_down== LOW && current_down == HIGH){//Down
          if(bulb4_setTime_indiv[4] >0){
            bulb4_setTime_indiv[4] --; 
      
          }
          else{
            bulb4_setTime_indiv[4]=59;
        
          }
          EEPROM.write(37, bulb4_setTime_indiv[4]);EEPROM.commit();
        }
        last_down=current_down;
    }
    lcd.setCursor(9,3);
    lcd.print(":");
    
    if(submenu1_counter==6){
      lcd.setCursor(11,3);
      lcd.cursor();
      delay(50);
      lcd.noCursor();
      delay(50);                
      //Move item + or -
      if (last_up== LOW && current_up == HIGH){  //Up 
          if(bulb4_setTime_indiv[5] < 59){
            bulb4_setTime_indiv[5] ++;
         
          }
          else{
            bulb4_setTime_indiv[5] = 0;
      
        }
        EEPROM.write(38, bulb4_setTime_indiv[5]);EEPROM.commit();
      }
      last_up=current_up;
      
      if(last_down== LOW && current_down == HIGH){//Down
        if(bulb4_setTime_indiv[5] >0){
          bulb4_setTime_indiv[5] --; 
    
        }
        else{
          bulb4_setTime_indiv[5]=59;
      
        }
        EEPROM.write(38, bulb4_setTime_indiv[5]);EEPROM.commit();
      }
      last_down=current_down;
    }
   
    //Parmeter Saved
    if (last_save== LOW && current_save == HIGH){
        lcd.clear();
        lcd.setCursor(5,1);
        lcd.print("SAVED!");
        delay(100);
        lcd.clear();
        Serial.println("State saved in flash memory");
        submenu1_counter=0;
        menu_counter=1; 
    }
    last_save = current_save;
    }
    break;
    }

 }

  
 

