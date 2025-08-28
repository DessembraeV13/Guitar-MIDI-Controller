#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32.h>
#include <HardwareSerial.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

//UART STM32=>ESP32
HardwareSerial SerialPort(2);

//MAC Address ESP8266
uint8_t broadcastAddress[] = {0xB4, 0xE6, 0x2D, 0x07, 0xEE, 0x43};

//Wyświetlacz
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//BLEMIDI_CREATE_DEFAULT_INSTANCE()
 BLEMIDI_CREATE_INSTANCE("AWESOME_INSTRUMENT", MIDI)

//pin wyboru połączenia
//BT 0 && Cable 0 - ESP-NOW 
//BT 1 && Cable 0 - Bluetooth
//Cable 1 - Cable
const int BTpin = 19;
const int Cablepin = 18;
const int ButtonLeft = 34;
const int ButtonCenter = 35;
const int ButtonRight = 32;

//TABLICE ZMIENNYCH
unsigned int FundamentalOld[6];
unsigned int FundamentalNew[6];
unsigned int MaxppOld[6];
unsigned int MaxppNew[6];
int NoteNew[6];
int NoteOld[6];
int Deltapp[6];
float FreqDelta[6];
//TABLICA Z ZAKRESAMI
static const int FreqNotes[46] = {
  82, 87, 93, 98, 104, 110, 116, 124, 130, 139, 147, 155, 165, 175, 185, 196, 208, 220, 233, 247, 262, 277, 293, 311, 330, 349, 370, 392, 
  415, 440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988, 1047, 1109
};
static int FreqRange[45];
static const int FreqTuning[6] = {82, 110, 147, 196, 247, 330};
static const uint8_t pepe[640] = {
    0x00, 0x00, 0x0e, 0x18, 0x00, 0x70, 0x1c, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x6f, 0xfa, 0x03, 0x3f, 0xf9, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x9f, 0xfc, 0x85, 0xff, 0xfe, 0x40, 0x00, 0x00, 
    0x00, 0x02, 0x7f, 0xff, 0x6b, 0xff, 0xff, 0xa0, 0x00, 0x00, 
    0x00, 0x05, 0xff, 0xff, 0xd7, 0xff, 0xff, 0xd0, 0x00, 0x00, 
    0x00, 0x0b, 0xff, 0xff, 0xef, 0xff, 0xff, 0xe8, 0x00, 0x00, 
    0x00, 0x0f, 0xff, 0xff, 0xdf, 0xff, 0xff, 0xf4, 0x00, 0x00, 
    0x00, 0x07, 0xff, 0xff, 0xdf, 0xff, 0xff, 0xf8, 0x00, 0x00, 
    0x00, 0x07, 0xff, 0xff, 0xbf, 0xff, 0xff, 0xfa, 0x00, 0x00, 
    0x00, 0xf7, 0xff, 0xff, 0xbf, 0xfe, 0x07, 0xfd, 0x00, 0x00, 
    0x01, 0x78, 0xff, 0xff, 0x3f, 0xe7, 0xfc, 0x7f, 0x00, 0x00, 
    0x01, 0xfe, 0x7f, 0xff, 0x7e, 0x3f, 0xff, 0xde, 0x80, 0x00, 
    0x07, 0xff, 0x9f, 0xff, 0x79, 0xff, 0xff, 0xfe, 0x80, 0x00, 
    0x0b, 0xff, 0xe3, 0xff, 0x6f, 0xff, 0xff, 0xff, 0xc0, 0x00, 
    0x0f, 0xff, 0xfe, 0x01, 0x3f, 0xfc, 0x07, 0xff, 0x20, 0x00, 
    0x20, 0x3f, 0xff, 0xfe, 0x7f, 0xe7, 0xf9, 0xff, 0x50, 0x00, 
    0x3f, 0xc7, 0xff, 0xfe, 0xff, 0x3f, 0xf0, 0x3f, 0xa8, 0x00, 
    0x30, 0x79, 0xff, 0xfd, 0xfd, 0xff, 0xe2, 0xcf, 0xb4, 0x00, 
    0x10, 0x0e, 0x7f, 0xfd, 0xf7, 0xff, 0xc2, 0x9f, 0xfc, 0x00, 
    0x52, 0x73, 0x9f, 0xfb, 0xcf, 0xff, 0xc6, 0x3f, 0xfa, 0x00, 
    0x62, 0x7c, 0xcf, 0xfb, 0x3f, 0xff, 0xc4, 0x3f, 0xfa, 0x00, 
    0x40, 0x7f, 0x73, 0xf5, 0xff, 0xff, 0x80, 0x7f, 0xfc, 0x00, 
    0x40, 0x7f, 0x9f, 0x73, 0xff, 0xff, 0x80, 0xff, 0xfd, 0x00, 
    0x40, 0xff, 0xe3, 0x87, 0xff, 0xff, 0x80, 0xff, 0xff, 0x00, 
    0x41, 0xff, 0xfe, 0x37, 0xff, 0xff, 0x81, 0xff, 0xfe, 0x00, 
    0x03, 0xff, 0xff, 0xfb, 0xff, 0xff, 0x83, 0xff, 0xfe, 0x80, 
    0x23, 0xff, 0xff, 0xfd, 0xff, 0xff, 0x8f, 0xff, 0xfe, 0x80, 
    0x37, 0xff, 0xff, 0xfe, 0xff, 0xff, 0x9f, 0xff, 0xff, 0x00, 
    0x29, 0xff, 0xff, 0xf3, 0x7f, 0xfe, 0x7f, 0xff, 0xff, 0x00, 
    0x0e, 0x7f, 0xff, 0x8d, 0xcf, 0xf9, 0xff, 0xff, 0xff, 0x00, 
    0x07, 0x9f, 0xf8, 0x7c, 0xf0, 0x0f, 0xff, 0xff, 0xff, 0x00, 
    0x0b, 0xe0, 0x3f, 0xfe, 0x7f, 0xff, 0xef, 0xff, 0xff, 0x00, 
    0x02, 0x7f, 0xff, 0xfd, 0xcf, 0xfe, 0x3f, 0xff, 0xff, 0x00, 
    0x03, 0x99, 0xff, 0xfe, 0x70, 0x01, 0xff, 0xff, 0xff, 0x40, 
    0x3b, 0xf3, 0xff, 0xff, 0x9f, 0xff, 0xff, 0xff, 0xff, 0x00, 
    0x51, 0xf7, 0x80, 0xff, 0xe7, 0xff, 0xff, 0xff, 0xff, 0x00, 
    0x7e, 0x1f, 0xff, 0x0f, 0xf9, 0xff, 0xff, 0xff, 0xff, 0x80, 
    0x7f, 0xff, 0xff, 0xf8, 0x1f, 0xc0, 0x3f, 0xff, 0xfe, 0x80, 
    0x4f, 0xfe, 0x07, 0x1f, 0xf0, 0x3f, 0xdf, 0xff, 0xfe, 0x80, 
    0x3b, 0xc3, 0xff, 0xe3, 0xff, 0xff, 0xdf, 0xff, 0xfe, 0x00, 
    0x07, 0xff, 0xc3, 0xfe, 0x00, 0xfc, 0x3f, 0xff, 0xfe, 0x00, 
    0x08, 0x1f, 0xf8, 0x1f, 0x7f, 0xdf, 0xbf, 0xff, 0xff, 0x00, 
    0x01, 0xff, 0xff, 0xe0, 0xbf, 0xfe, 0x7f, 0xff, 0xfd, 0x00, 
    0x01, 0x7f, 0xff, 0xff, 0x80, 0xe7, 0xff, 0xff, 0xfc, 0x00, 
    0x01, 0x7f, 0xff, 0xff, 0xff, 0x6f, 0xff, 0xff, 0xfe, 0x00, 
    0x00, 0x7f, 0xff, 0xff, 0xff, 0xbf, 0xff, 0xff, 0xf8, 0x00, 
    0x00, 0xbf, 0xff, 0xff, 0xff, 0xbf, 0xff, 0xff, 0xe0, 0x00, 
    0x00, 0x5f, 0xff, 0xff, 0xff, 0xbf, 0xff, 0xff, 0x14, 0x00, 
    0x00, 0x2f, 0xff, 0xff, 0xff, 0xbf, 0xff, 0xe1, 0xed, 0x00, 
    0x00, 0x20, 0xff, 0xff, 0xff, 0xbf, 0xf0, 0x1f, 0x9e, 0x00, 
    0x00, 0x97, 0x00, 0x00, 0x00, 0x70, 0x0f, 0xfe, 0x7f, 0x80, 
    0x01, 0x7b, 0xff, 0xff, 0xff, 0x6f, 0xff, 0xf9, 0xff, 0x00, 
    0x04, 0xfc, 0xff, 0xff, 0xff, 0xef, 0xff, 0x0f, 0xff, 0x00, 
    0x03, 0xff, 0x80, 0x60, 0x3f, 0x63, 0x81, 0xff, 0xff, 0x40, 
    0x17, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xc0, 
    0x2f, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0x80, 
    0x1f, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xa0, 
    0x5f, 0xff, 0xff, 0xff, 0xff, 0xef, 0xff, 0xff, 0xff, 0xa0, 
    0x7f, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xe0, 
    0x7f, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xc0, 
    0x7f, 0xff, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff, 0xff, 0xc0, 
    0x7f, 0xff, 0xff, 0xff, 0xff, 0xb7, 0xff, 0xff, 0xff, 0xe0, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x58, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00
};

typedef struct struct_message {
  int notenow;
  int velnow;
  int onoff;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}


//ZMIENNE
bool newData = false;
int TresholdOn = 1000;
int TresholdOff = 30;
int NoteOffset = 40;
//0 - velocity const
//1 - velocity adapt
//2 - tuner
int Mode = 0;
int ModePrev = -1;
//0 - Cable
//1 - Bluetooth
//2 - ESP-NOW
int Connection = 0;
int BTstate = 0;
int Cablestate = 0;

int ButtonLeftState = 1;
int ButtonCenterState = 1;
int ButtonRightState = 1;
int ButtonLeftStatePrev = 1;
int ButtonCenterStatePrev = 1;
int ButtonRightStatePrev = 1;

void IRAM_ATTR left()
{
  Mode = 0;
}
void IRAM_ATTR center()
{
  Mode = 1;
}
void IRAM_ATTR right()
{
  Mode = 2;
}

void setup() {
  pinMode(BTpin, INPUT);
  pinMode(Cablepin, INPUT);
  pinMode(ButtonLeft, INPUT_PULLUP);
  pinMode(ButtonCenter, INPUT_PULLUP);
  pinMode(ButtonRight, INPUT_PULLUP);
  attachInterrupt(ButtonLeft, left, FALLING);
  attachInterrupt(ButtonCenter, center, FALLING);
  attachInterrupt(ButtonRight, right, FALLING);
  SerialPort.begin(115200, SERIAL_8N1, 16, 17); // 115200 bps, 8 bitów danych, brak parzystości, 1 bit stopu, piny 16 (RX) i 17 (TX)
  Serial.begin(115200); // połączenie z komputerem
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  BTstate = digitalRead(BTpin);
  Cablestate = digitalRead(Cablepin);
  if(BTstate == 0 && Cablestate == 0)
    Connection = 2;
  if(BTstate == 1 && Cablestate == 0)
    Connection = 1;
  if(Cablestate == 1)
    Connection = 0;
  switch (Connection){
    case 1:
      MIDI.begin();
      break;
    case 2: 
      // Set device as a Wi-Fi Station
      WiFi.mode(WIFI_STA);

      // Init ESP-NOW
      if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
      }

      // Once ESPNow is successfully Init, we will register for Send CB to
      // get the status of Trasnmitted packet
      esp_now_register_send_cb(OnDataSent);
  
      // Register peer
      memcpy(peerInfo.peer_addr, broadcastAddress, 6);
      peerInfo.channel = 0;  
      peerInfo.encrypt = false;
  
      // Add peer        
      if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
      }
      break;
    default:
      delay(5);
      break;
  }
  display.clearDisplay();
  display.drawBitmap(26, 0, pepe, 75, 64, 1);
  display.display();
  delay(2000);

  for(int i = 0; i<45; i++){
    FreqRange[i] = (FreqNotes[i] + FreqNotes[i+1])/2;
  }
}


void loop() {
  Wyswietl();
  ReadData();
  if(newData){
    if(Mode == 0 && Connection == 0){
      for(int i = 0; i < 6; i++){
        Deltapp[i] = MaxppNew[i] - MaxppOld[i];
        if(Deltapp[i] >= TresholdOn){
          Serial.write(128);
          Serial.write(NoteOld[i] + NoteOffset);
          Serial.write(127);
          Serial.write(144);
          Serial.write(NoteNew[i] + NoteOffset);
          Serial.write(127);
        }else if(MaxppNew[i] < MaxppOld[i] && MaxppNew[i] <= TresholdOff){
        //MIDI.sendNoteOff((NoteOld[i]+NoteOffset), 127, 1);
        }
      }
    }
    if(Mode == 0 && Connection == 1){
      for(int i = 0; i < 6; i++){
        Deltapp[i] = MaxppNew[i] - MaxppOld[i];
        if(Deltapp[i] >= TresholdOn){
          MIDI.sendNoteOff((NoteOld[i] + NoteOffset), 127, 1);
          MIDI.sendNoteOn((NoteNew[i] + NoteOffset), 127, 1);
        }else if(MaxppNew[i] < MaxppOld[i] && MaxppNew[i] <= TresholdOff){
        //MIDI.sendNoteOff((NoteOld[i]+NoteOffset), 127, 1);
        }
      }
    }
    if(Mode == 0 && Connection == 2){
      for(int i = 0; i < 6; i++){
        Deltapp[i] = MaxppNew[i] - MaxppOld[i];
        if(Deltapp[i] >= TresholdOn){
          myData.notenow = NoteOld[i] + NoteOffset;
          myData.velnow = 127;
          myData.onoff = 0;
          esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
          myData.notenow = NoteNew[i] + NoteOffset;
          myData.velnow = 127;
          myData.onoff = 1;
          esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
        }else if(MaxppNew[i] < MaxppOld[i] && MaxppNew[i] <= TresholdOff){
        //MIDI.sendNoteOff((NoteOld[i]+NoteOffset), 127, 1);
        }
      }
    }
    if(Mode == 1 && Connection == 0){
      for(int i = 0; i < 6; i++){
        Deltapp[i] = MaxppNew[i] - MaxppOld[i];
        if(Deltapp[i] >= TresholdOn){
          Serial.write(128);
          Serial.write(NoteOld[i] + NoteOffset);
          Serial.write(127);
          Serial.write(144);
          Serial.write(NoteNew[i] + NoteOffset);
          Serial.write(map(Deltapp[i], TresholdOn, 4095, 64, 127));
        }else if(MaxppNew[i] < MaxppOld[i] && MaxppNew[i] <= TresholdOff){
        //MIDI.sendNoteOff((NoteOld[i]+NoteOffset), 127, 1);
        }
      }
    }
    if(Mode == 1 && Connection == 1){
      for(int i = 0; i < 6; i++){
        Deltapp[i] = MaxppNew[i] - MaxppOld[i];
        if(Deltapp[i] >= TresholdOn){
          MIDI.sendNoteOff((NoteOld[i] + NoteOffset), 127, 1);
          MIDI.sendNoteOn((NoteNew[i] + NoteOffset), map(Deltapp[i], TresholdOn, 4095, 64, 127), 1);
        }else if(MaxppNew[i] < MaxppOld[i] && MaxppNew[i] <= TresholdOff){
          //MIDI.sendNoteOff((NoteOld[i]+NoteOffset), 127, 1);
        }
      }
    }
    if(Mode == 1 && Connection == 2){
      for(int i = 0; i < 6; i++){
        Deltapp[i] = MaxppNew[i] - MaxppOld[i];
        if(Deltapp[i] >= TresholdOn){
          myData.notenow = NoteOld[i] + NoteOffset;
          myData.velnow = 127;
          myData.onoff = 0;
          esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
          myData.notenow = NoteNew[i] + NoteOffset;
          myData.velnow = map(Deltapp[i], TresholdOn, 4095, 64, 127);
          myData.onoff = 1;
          esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
        }else if(MaxppNew[i] < MaxppOld[i] && MaxppNew[i] <= TresholdOff){
        //MIDI.sendNoteOff((NoteOld[i]+NoteOffset), 127, 1);
        }
      }
    }
    if(Mode == 2){
      for(int i = 0; i < 6; i++){
        FreqDelta[i] = FundamentalNew[i] - FreqTuning[i];
      }
    }
  }
  newData = false;
  for(int i = 0; i < 6 ; i++){
    if(Deltapp[i]>=TresholdOn){
      NoteOld[i] = NoteNew[i];
    }
    MaxppOld[i] = MaxppNew[i];
  }
}


void ReadData() {
if (SerialPort.available()) {
    // Odczytaj dane i przypisz do odpowiednich zmiennych
    sscanf(SerialPort.readStringUntil('\n').c_str(), "f0 = %u, m0 = %u", &FundamentalNew[0], &MaxppNew[0]);
    sscanf(SerialPort.readStringUntil('\n').c_str(), "f1 = %u, m1 = %u", &FundamentalNew[1], &MaxppNew[1]);
    sscanf(SerialPort.readStringUntil('\n').c_str(), "f2 = %u, m2 = %u", &FundamentalNew[2], &MaxppNew[2]);
    sscanf(SerialPort.readStringUntil('\n').c_str(), "f3 = %u, m3 = %u", &FundamentalNew[3], &MaxppNew[3]);
    sscanf(SerialPort.readStringUntil('\n').c_str(), "f4 = %u, m4 = %u", &FundamentalNew[4], &MaxppNew[4]);
    sscanf(SerialPort.readStringUntil('\n').c_str(), "f5 = %u, m5 = %u", &FundamentalNew[5], &MaxppNew[5]);

    FundamentalToNote(FundamentalNew, NoteNew, FreqRange);
    newData = true;
  }
}
void FundamentalToNote(unsigned int FundamentalNew[6], int NoteNew[6], const int FreqRange[45]) {
  for (int i = 0; i < 6; i++) {
    int note = 0;
    for (int j = 0; j < 45; j++) {
      if (FundamentalNew[i] >= FreqRange[j] && FundamentalNew[i] < FreqRange[j + 1]) {
        note = j;  // Przypisz odpowiednią notę
        break;  // Przerwij pętlę po znalezieniu odpowiedniego zakresu
      }
    }
    NoteNew[i] = note;
  }
}

void Wyswietl(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  switch (Connection) {
    case 0:
      display.println(F("Connection: Cable"));
      break;
    case 1:
      display.println(F("Connection: Bluetooth"));
      break;
    default:
      display.println(F("Connection: ESP-NOW"));
      break;
  }
  switch (Mode){
    case 0:
      display.println(F("Mode: Const"));
      break;
    case 1:
      display.println(F("Mode: Adapt"));
      break;
    default:
      display.println(F("Mode: Tuner"));
      display.print(F("E:"));
      display.print(round(FundamentalNew[0]));
      display.println(F(" Target:82"));
      display.print(F("A:"));
      display.print(round(FundamentalNew[1]));
      display.println(F(" Target:110"));
      display.print(F("D:"));
      display.print(round(FundamentalNew[2]));
      display.println(F(" Target:147"));
      display.print(F("G:"));
      display.print(round(FundamentalNew[3]));
      display.println(F(" Target:196"));
      display.print(F("B:"));
      display.print(round(FundamentalNew[4]));
      display.println(F(" Target:247"));
      display.print(F("E:"));
      display.print(round(FundamentalNew[5]));
      display.println(F(" Target:330"));
      break;
  }
  display.display();
}