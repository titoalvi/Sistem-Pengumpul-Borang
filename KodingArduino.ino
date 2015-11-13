/* PROTOTIPE KUNCI PINTU OTOMATIS BERBASIS ARDUINO DAN RFID 
 * Desember 2014 - AJI PRAKOSO
 * 
 * Menggunakan Library MFRC522
 * https://github.com/miguelbalboa/rfid
 * Dibuat oleh Miguel Balboa (circuitito.com)
 */

#include <EEPROM.h> // Memanggil Library EEPROM
#include <SPI.h> // Memanggil Library SPI 
#include <MFRC522.h> // Memanggil Library MFRC522
#include <Servo.h> // Memanggil Library Servo

boolean match = false;
boolean programMode = false;
byte storedCard[4];
byte readCard[4];
//04 26 15 A5
//byte masterCard[4] = {0x82,0xfd,0x54,0xc5}; // UID kartu yang diijinkan masuk
byte masterCard[4] = {0x04,0x26,0x15,0xa5};

int r = 4; // Pin Red pada LED RGB
int g = 3; // Pin Green pada LED RGB
int b = 2; // Pin Blue pada LED RGB
int buzz = 5; // Pin data untuk buzzer
Servo servo1; // Servo kita beri nama "servo1"

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600); 
  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(buzz, OUTPUT);
  pinMode(r, OUTPUT);
  pinMode(g, OUTPUT);
  pinMode(b, OUTPUT);
  servo1.attach(6);
  digitalWrite(b, HIGH);
  servo1.write(20);
}

void loop () 
{
  int successRead;  
  do 
  {
    successRead = getID(); 
  }
  
  while (!successRead); 
  
  if (isMaster(readCard)) // Jika kartu yang di didekatkan ke pembaca adalah MasterCard (EC9FE97) Maka akan muncul :
  { 
    Serial.println("BENARCUY");
    digitalWrite(b, LOW); // LED Yang tadinya berwarna biru (stand by) dimatikan, berubah menjadi
    digitalWrite(g, HIGH); // warna hijau
    
    servo1.write(90); // Posisi servo akan berubah menjadi 90 derajat (posisi buka kunci)
    //delay(2500); // Setelah delay selama 2500ms (2,5 detik),
    digitalWrite(b, HIGH); // Lampu LED akan berubah kembali menjadi berwarna biru, dan
    digitalWrite(g, LOW);
    servo1.write(0); // Posisi servo akan kembali ke 0 derajat (posisi mengunci).
  }
  
  else // Jika kartu yang didekatkan ke pembaca BUKAN MasterCard (EC9FE97) Maka akan muncul :
  {
    Serial.println("SALAHCUY");
    digitalWrite(r, HIGH); // LED akan berubah menjadi berwarna merah,
    digitalWrite(b, LOW);
    digitalWrite(b, HIGH); // lampu LED akan kembali menjadi berwarna biru.
    digitalWrite(r, LOW);
  }
}

int getID() 
{
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  { 
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return 0;
  }
  //HATI2
  for (byte i = 0; i < mfrc522.uid.size; i++) {  
    readCard[i] = mfrc522.uid.uidByte[i];
  }

  mfrc522.PICC_HaltA();
  return 1;
}

void readID( int number ) {
  int start = (number * 4 ) - 3;
  for ( int i = 0; i < 4; i++ ) {
    storedCard[i] = EEPROM.read(start+i);
  }
}

boolean checkTwo ( byte a[], byte b[] ) {
  if ( a[0] != NULL ) 
    match = true; 
  for ( int k = 0; k < 4; k++ ) { 
    if ( a[k] != b[k] )
      match = false;
  }
  if ( match ) { 
    return true; 
  }
  else  {
    return false; 
  }
}

boolean isMaster( byte test[] ) {
  if ( checkTwo( test, masterCard ) )
    return true;
  else
    return false;
}
