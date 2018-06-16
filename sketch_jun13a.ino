
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display


float my_vcc_const = 1.1;

int pin_swgnd = 7;
int pin_swin = 6;
int pin_butgnd = 10;
int pin_butin = 11;
int pin_buzgnd = 8;
int pin_buzvcc = 9;
int pin_relay = 13;
int pin_input = A1;

float k = 22.22;


int mode = 0;
float voltage_t = 0.0;
long time = 0;
boolean last_bt;

boolean btnState, btnFlag;

long readVcc() {
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);
#else
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // measuring
  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both
  long result = (high << 8) | low;
  result = my_vcc_const * 1023 * 1000 / result; // расчёт реального VCC
  return result; // возвращает VCC
}

boolean swread()
{
  if (!digitalRead(pin_swin)) {
    return true;
  } else return false;
}

boolean btread()
{
  if (!digitalRead(pin_butin)) {
    return true;
  } else return false;
}

float readIn()
{
  float vcc = float(readVcc()) * 0.001;
  delay(2);
  float result = (float(analogRead(pin_input)) * ((float)vcc / 1023.0)) * (float)k;
  return result;
}


void doIn()
{
  digitalWrite(pin_relay,HIGH);  
}

void doOut()
{
  digitalWrite(pin_relay,LOW);  
}

void setup()
{
  pinMode(pin_relay, OUTPUT);
  digitalWrite(pin_relay, LOW);

  pinMode(pin_swgnd, OUTPUT);
  digitalWrite(pin_swgnd, LOW);
  pinMode(pin_swin, INPUT_PULLUP);

  pinMode(pin_butgnd, OUTPUT);
  digitalWrite(pin_butgnd, LOW);
  pinMode(pin_butin, INPUT_PULLUP);


  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  pinMode(pin_buzgnd, OUTPUT);
  pinMode(pin_buzvcc, OUTPUT);

  digitalWrite(pin_buzgnd, LOW);
  digitalWrite(pin_buzvcc, LOW);

  lcd.print("ready");
  lcd.setCursor(0, 1);
  lcd.print("swich sw1 to on");
  tone(pin_buzvcc, 200, 400);
  delay(500);
  tone(pin_buzvcc, 400, 500);

}


// mode 0: start work input value
// mode 1: work started

void loop()
{


  if (mode == 0)
  {
    while (!swread()) {}

    while (swread())
    {
      lcd.clear();

      if (btread())
      {
        voltage_t = voltage_t + 0.1;
       lcd.print("V+: ");
        lcd.print(readIn());
       lcd.setCursor(0, 1);
       lcd.print("VT: ");
       lcd.print(voltage_t);
        delay(800);
      }
      else
      {
        voltage_t = voltage_t + 0.5;
        lcd.print("V+: ");
        lcd.print(readIn());
        lcd.setCursor(0, 1);
        lcd.print("VT: ");
        lcd.print(voltage_t);
        delay(200);
      }

    }
    last_bt = btread();
    mode = 1;
  }

  if (mode == 1)
  {
    lcd.clear();
    lcd.print("Ok, on ");
    lcd.print(voltage_t);
    lcd.print("V");
    lcd.setCursor(0, 1);
    lcd.print("we disable it:-)");
    doIn();
    tone(pin_buzvcc, 600, 200);
    delay(200);
    tone(pin_buzvcc, 400, 300);
    delay(4500);
    lcd.clear();
    mode = 2;
  }
  if(mode == 2)
  {
    long times = millis();
        float now = readIn();
    while((now < voltage_t) and (last_bt == btread())){
    lcd.clear();
    lcd.print("N:");
    now = readIn();
    lcd.print(now);
    lcd.print(" G:");
    lcd.print(voltage_t);
    lcd.setCursor(0,1);
    lcd.print("Time: ");
    time = (millis() - times) / 1000;
    lcd.print(time);
    delay(1000);
    }
    mode = 3;
  }
  if(mode == 3)
  {
    doOut();
    tone(pin_buzvcc,440,100);
    delay(100);
    tone(pin_buzvcc,500,200);
    delay(200);
    tone(pin_buzvcc,150,300);
    while(!swread())
    {
    lcd.clear();
    lcd.print("V+: ");
    lcd.print(readIn());
    lcd.setCursor(0,1);
    lcd.print(time);
    delay(500);
    }
  }
  lcd.clear();
  lcd.print("Workout");
  lcd.setCursor(0,1);
  lcd.print("Pls reboot sys");
  while (1) {}

}
