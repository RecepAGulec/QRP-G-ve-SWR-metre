
/*
   ------------------------------------------------------------------------
  Orijinal yazlımı:  "PH2LB LICENSE" (Revision 1) :
   Orijinal yazılımdan farklı olarak, SPI OLED yerine I2C OLED kullanılmıştır.
   Okunurluğu atrrıtmak üzere font düzenlemeleri yapılmıştır.
   Devrede Breune SWR köprüsü kullanılmıştır. Uygun bir örnek https://www.zl2pd.com/SWRmeter.html
  yer almaktadır.
   SWR köprüsünün FWD çıkış A0'a, REF çıkışı ise A1'e bağlanmıştır. uygulama devrenize göre
  gerekirse A0 ve A1'i yer değiştirebilirsiniz.
  ARDUINO BAĞLANTILARI
  A0    SWR köprüsü FWD
  A1    SWR köprüsü REF
  A2    Pil gerilimi
  A4    OLED SDA
  A5    OLED CLK
  D8    Yüksek SWR LED'i

   ------------------------------------------------------------------------
*/
// Kütüphaneler ekleniyor
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128      // OLED göstergenin piksel olarak genişliği
#define SCREEN_HEIGHT 64      // OLED göstergenin piksel olarak yüksekliği
#define OLED_RESET     -1     // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C   // OLED göstergenin i2c adresi
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // OLED gösterge ayarlanıyor.
#define HIGHSWRLED 7          // Yüksek SWR ledinin bağlanacağı pin

// Voltmetre değişkenleri
float temp = 0.0;
float r1 = 8260.0;                   // R1 direncinin ohm cinsinden değeri - Kendi devrenizdeki değerleri yazın.
float r2 = 4686.0;                   // R2 direncinin ohm cinsinden değeri - Kendi devrenizdeki değerleri yazın.
float input_voltage = 0.0;

/////////////////////////////////////////// SETUP //////////////////////////////////////////////////////////////////////////
void setup()   {
  Serial.begin(9600);

  pinMode(HIGHSWRLED, OUTPUT);
  digitalWrite(HIGHSWRLED, HIGH);   // LED'i yak
  delay(200);                       // 0.2 saniye (200 milisaniye)bekle
  digitalWrite(HIGHSWRLED, LOW);    // LED'i söndür
  delay(200);                       // 0.2 saniye bekle
  digitalWrite(HIGHSWRLED, HIGH);   // LED'i yak
  delay(200);                       // 0.2 saniye bekle
  digitalWrite(HIGHSWRLED, LOW);    // LED'i söndür
  delay(200);                       // 0.2 saniye bekle

  display.begin(SSD1306_SWITCHCAPVCC);    // Gösterge hazırlanıyor
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
  }
  display.display();
  delay(2000);                            // 2 saniye bekle
  display.clearDisplay();                 // Tamponu temizle
  display.setTextSize(2);                 // Metin boyutunu 2 olarak ayarla
  display.setTextColor(SSD1306_WHITE);    // Yazı rengi beyaz
  display.setCursor(0, 0);                // Sol üst köşeden başlanacak
  display.println(" * TA2EI *");          //
  display.println("__________");          //
  display.println();                      //
  display.setTextSize(1);                 // Metin boyutunu 1 olarak ayarla
  display.println("QRP power/swr metre"); //
  display.display();                      //
  delay(5000);                            //
  display.setTextSize(2);                 // Metin boyutunu 2 olarak ayarla

}
//////////////////////////// SETUP SONU //////////////////////////////
// Pil gerilimini ölçme yordamı //////////////////////////////////////
void volc()
{
  int analog_value = analogRead(A2);
  temp = (analog_value * 5.0) / 1024.0;
  input_voltage = temp / (r2 / (r1 + r2));
  delay(100);
}
int counter = 0;
// ****************** GÖSTERGE  *************************************
void updateDisplay(double pForward, double pReflected, long swr)
{
  // Clear the buffer.
  display.clearDisplay();               // Göstergeyi sil
  display.setTextColor(WHITE);          // Yazı rengi beyaz
  display.setCursor(0, 0);              // Sol üst köşeye konumlan
  display.clearDisplay();               // 
  display.setCursor(0, 0);              // 
  display.print("FWD:");                //
  display.print(pForward);              //
  display.println("W");                 //
  display.print("REF:");                //
  display.print(pReflected);            //
  display.println("W");                 //
  display.print("SWR:");                //
  if (swr >= 10)
  {
    display.print(swr / 10);
    display.print(".");
    display.print(swr % 10);
    display.println(":1");
  }
  else
  {
    display.println("No sig");
  }
  display.print("PiL:");                //
  display.print(input_voltage);         //
  display.println(" v");
  display.display();
}

//////////////////////// ANA DÖNGÜ ////////////////////////////////////////////////////
void loop()
{
  // Giden ve yansıyan güç gerilim değerleri okuma ************************************
  long forward = analogRead(1);       // A1 analog girişini oku ve okunan değeri LONG tipindeki forward değişkeninde sakla
  long reflected = analogRead(0);     // A0 analog girişini oku ve okunan değeri LONG tipindeki reflected değişkeninde sakla
  // SWR hesaplaması *****************************************************************
  long swr = forward + reflected;     // forward ve reflected değişkenlerini topla veLONG tipindeki swr değişkeninde sakla
  swr = swr * 10;                     // swr değişkenini 10'la çarp
  long temp = forward - reflected;    // forward değişkeninden reflected değişkenin çıkar ve sonucu LONG tipindeki temp değişkeninde sakla
  if (forward <= reflected)           // eğer forward değeri reflected değerine eşit ya da küçük ise
    temp = 1;                         // temp değişkeninin değerini 1 yap

  swr = swr / temp;
  if (swr > 100)
    swr = 100;

  if (swr > 20)
    digitalWrite(HIGHSWRLED, HIGH);   // SWR 20'den büyükse LED yanar
  else
    digitalWrite(HIGHSWRLED, LOW);    // SWR 20'den küçükse LED sönüktür
  // Giden gücün ölçülmesi *************************************************************
  double pForward = ((double)forward / 1024.0 * 5.0); // make it voltage
  pForward = (pForward * 7.07 * 0.707);
  pForward = pForward * pForward ;
  pForward = pForward / 27;

  // if there is a signal compensate for measurement error
  // used a FT817 (measured the output with a quality digital power meter)
  // and made a compensation function. This may be need to be ajusted
  // based on your implementation of the Breune directional coupler
  if (pForward > 0.01)
    pForward -= (2.0 * pForward) / 15.0 - (2.0 / 15.0); 
  // Yansıyan gücün ölçülmesi *****************************************************
  double pReflected = ((double)reflected / 1024.0 * 5.0); // make it voltage
  pReflected = (pReflected * 7.07 * 0.707);
  pReflected = pReflected * pReflected ;
  pReflected = pReflected / 27;
  // again signal compensation
  if (pReflected > 0.01)
    pReflected -= (2.0 * pReflected) / 15.0 - (2.0 / 15.0);

  updateDisplay(pForward, pReflected, swr);
  //updateLog(pForward, pReflected, swr, forward, reflected);
  //updateCSV(pForward, pReflected, swr, forward, reflected);
  delay(500);
  volc();
}
