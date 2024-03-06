
#include <DIYables_IRcontroller.h>
#include <SPI.h>
#include <MFRC522.h>

//asocierea pinilor cu iesirea/intrarea fizica pentru o coordonare mai usoara
#define IR_RECEIVER_PIN 2
#define SS_PIN 10
#define RST_PIN 9
#define RED_LED 6
#define GREEN_LED 4
#define YELLOW_LED 3 
#define BUZZER 5

int trigPin = 7;
int echoPin = 8;  

//declararea variabilelor globale
float duration_us, distance_cm;
int threshold_distance = 15;
bool alarm_active = false;
bool unlock_attempt = false;
DIYables_IRcontroller_21 irController(IR_RECEIVER_PIN, 200); //asocierea senzorilor cu functiile acestora
MFRC522 rfid(SS_PIN, RST_PIN);
unsigned long previousMillis = 0;  
unsigned long now=0;
int currentstate = HIGH;
const long interval = 1000;
unsigned long previousMicro=0;

void setup() {
  Serial.begin(9600);
  pinMode(RED_LED, OUTPUT); //initializarea tuturor senzorilor
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT); 
  pinMode(BUZZER, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
 
  irController.begin();
  Serial.println("System initialized.\n");
  SPI.begin();
  rfid.PCD_Init(); 
}

void loop() {
armed();//apelarea functiei de armare
}
void armed(){
  Serial.println("System armed.");
  while(1){ //se repeta incontinuu pana la apasarea btonului de pe telecomanda, caz in care apeleaza alta functie
    unsigned long currentMillis = millis();
    Serial.println(currentMillis); //afisaeza timpul actual
  if (currentMillis - previousMillis >= 2500) {//functie de blink a unui led la intervale de 2.5 secunde.  
    previousMillis = currentMillis;//se foloseste aceasta functie pentru ca montajul meu are de facut undeva la 4-5 chestii simultan
    digitalWrite(RED_LED, HIGH);
    delay(100); //aici imi permit sa folosesc un delay de 0.1 secunde petru ca nu are efect foarte mare
    digitalWrite(RED_LED, LOW);
  }
  
  Key21 key = irController.getKey();//citeste intrarea de pe senzorul de ir
 if (key != Key21::NONE)//daca se ddetecteaza un buton apasat se apeleaza functia disable
 disableAlarm(key);
  digitalWrite(trigPin, HIGH); //se declanseaza un semnal pe o recventa ultrasonica
  
if(micros()-previousMicro>=10){ //se asteapta 10 microsecunde, desi aparent variabila previousmicro nu se schimba, dar am observat asta la scrierea comentariului, dar functioneaza si asa, asa ca aceasta linie nu prea are rost
  //delayMicroseconds(10);
  digitalWrite(trigPin, LOW);//se opreste semnalul
  duration_us = pulseIn(echoPin, HIGH);//se asculta ecoul semnalului emis mai devreme
  distance_cm = 0.017 * duration_us;//se calculeaza distanta de la care se intoarce ecoul
  Serial.print(distance_cm);
  Serial.println(" cm");//se afiseaza distanta
}
  if (distance_cm < threshold_distance) {//daca distanta scade sub prag, se activeaza alarma
    activateAlarm();//apelarea alarmei 
  }}
}

void unlockSystem() {//dezactivarea sistemului
  Serial.println("System unlocked.");
  analogWrite(BUZZER, 68); //se emite un semnal sonor usor identificabil
  delay(300);
  analogWrite(BUZZER, 0);

  digitalWrite(GREEN_LED, HIGH);//se aprinde ledul de recunoastere a acestei stari
  while(1){//se repeta functia pana cand se apasa un anumit buton, si se apeleaza alta functie
    
  Key21 key = irController.getKey();
  //se citeste intrarea de la telecomanda. daca intrarea este reprezwntata de +, se activeaza alarma
 if (key != Key21::NONE)
 { 
 if(key == Key21::KEY_VOL_PLUS) 
    {digitalWrite(GREEN_LED, LOW);//daca se apasa butonul corect, ledul albastru se stinge si se apeleaza functia de armare
    armed();}
  }}
}

unsigned long previousMilliss=0;
String getUID() {//functia pentru citirea id-ului cartelei rfid
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}
unsigned long c=0;
int greenstate=LOW;
void blinkRedLED(int interval) {//functie pentru a efectua acel joc de lumini atunci cand alarma este activa

  unsigned long currentMilliss = millis();
  
  if (currentMilliss - previousMilliss >= interval) {//aprindereea si stingerea ledurilor e alternativa, si se desfasoara la un anumit interval pre-declarat la apelarea functiei
    previousMilliss = currentMilliss;
  if(currentstate==LOW)
    {currentstate=HIGH;
    greenstate=LOW;}
    else
    {currentstate=LOW;
    greenstate=HIGH;}
    digitalWrite(RED_LED, currentstate);
    digitalWrite(GREEN_LED, greenstate);

  }
}
void disableAlarm(Key21 key){//functie de incepere a procesului de dezarmare
  if(key==Key21::KEY_VOL_MINUS)//aceasta functie incepe doar dupa apasarea butonului -, altfel se revine la modul anterior
  {
   
    Serial.println("Unlock process initiated. Enter the code within 10 seconds.");
    digitalWrite(YELLOW_LED, HIGH);   //se aprinde becul galben pentru a anunta acest mod
    if (checkCode()==1) {//se verifica daca a fost introdusa parola corecta, si se trece la scanarea cartelei rfid
      Serial.println("Enter RFID tag.");
      int RFID=scanRFID();
      if (RFID==1) {//daca cartela e corecta, se stinge ledul galben si se apeleaza functia de dezarmare
        digitalWrite(YELLOW_LED, LOW);

        unlockSystem();
      } else if(RFID==0) {//altfel se stinge ledul galben si se activeaza alarma
        digitalWrite(YELLOW_LED, LOW);
        activateAlarm();
      }
    } else {
      digitalWrite(YELLOW_LED, LOW);//daca e intodus codul gresit se activeaza alarma
      activateAlarm();
    }}
}
int checkCode() {//functia de verificare a parolei introduse de la telecomanda
  while(1){//se repeta pana cand 1. expira timpul sau 2. se detecteaza un input
  unsigned long now=millis();
  Serial.print("Enter code\n");
while(millis()-now<=10000)//limitarea timpului de introducere a parolei la 10 secunde
  { 

    Key21 edKey;
   edKey= irController.getKey();//se citeste intrarea de la telecomanda
   

    if(edKey!=Key21::NONE && edKey!=Key21::KEY_VOL_MINUS)//daca se detecteaza o intrarea si nu e tasta - (a fost necesara aceasta modificare intrucat
    {if (edKey == Key21::KEY_100_PLUS) {                  //de la ultima apasare si pana aici, placa reusea sa proceseze mai repede decat reusea utilizatorul sa ridice degetul de pe buton
        Serial.println("100+");                           //si astefl se interpreta un rezultat fals pozitiv. In cazul de fata, parola consta intr-un singur buton (100+)
        Serial.println("Code accepted.\n");
        return 1;       //se returneaza faptul ca a fost introdus codul corect
        }
     else break;}       //altfel se iese din bucla
    } Serial.println("Incorrect code. activating alarm\n");
        return 0;//daca se introduce codul gresit sau expira timpul, se returneaza 0, adica introducerea codului nu a fost efectuata cu succes
  }
}

int scanRFID() {//functie de citire a cartelei
  Serial.println("Tap RFID/NFC Tag on reader");
  unsigned long Now=millis();
    while(millis()<Now+10000)//utilizatorul poate introduce cartela in interval de 10 secunde
  {
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    Serial.println("RFID tag detected.\n");
    String scannedUID = getUID();
    if (scannedUID == " A3 08 FF 0D") //daca cartela are acest id se returneaza 1, adica succes
    {Serial.println("correct rfid. alarm disabling");
    return 1;}
    else {Serial.println("Wrong rfid. alarm starting");
    return 0;}//daca cartela scanata nu are id-ul declarat, se returneaza 0 adica fail
  }
  } 
  
    Serial.println("\nNo RFID tag detected in time. Alarm activated.");
    return 0; //daa expira timpul se returneaza 0
  
}
unsigned long previousTime=0;
int Buz=LOW;

void activateAlarm() {//functie de activare a alarmei
  Serial.println("ALARM ACTIVATED!");
  while(1){//se repeta pana cand se apeleaza functia de dezactivare

unsigned long currentTime=millis();
  if (currentTime - previousTime >= 1000) {//la fiecare secunda buzzer-ul emite un sunet intermitent
    previousTime = currentTime;
  if(Buz==LOW)
    Buz=HIGH;
    else
    Buz=LOW;
    digitalWrite(BUZZER, Buz);
  }
   
    blinkRedLED(1000);//se alterneaza aprinderea led-urilor
  Key21 key = irController.getKey();
 if (key != Key21::NONE)//daca se detecteaza un input, se stinge alarma si se trece la pasul de dezactivare
 {digitalWrite(BUZZER, LOW);
   digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
 disableAlarm(key);}
}
}


