#include <Arduino.h>

const int pot_pin = PA0;
const int button_pin = PA1;
const int led_pin = PA5;

HardwareTimer timer(TIM2);
volatile unsigned long ticks = 0;
volatile bool button_pressed = false;
volatile unsigned long last_press = 0;

bool manual_mode = false;
bool led_on = false;
unsigned long last_tick = 0;
int adc_value = 0;
int percent = 0;
int blink_time = 0;

void button_interrupt() {
  if (millis() - last_press > 200) {
    last_press = millis();
    button_pressed = true;
  }
}

void timer_interrupt() {
  ticks++;
}

void setup() {
  pinMode(pot_pin, INPUT_ANALOG);
  pinMode(button_pin, INPUT_PULLUP);
  pinMode(led_pin, OUTPUT);
  analogReadResolution(12);
  Serial1.begin(115200);

  attachInterrupt(digitalPinToInterrupt(button_pin), button_interrupt, FALLING);
  timer.setOverflow(100000, MICROSEC_FORMAT); // 100 ms
  timer.attachInterrupt(timer_interrupt);
  timer.resume();
}

void loop() {
  if (button_pressed) {
    button_pressed = false;
    manual_mode = !manual_mode;
    Serial1.println(manual_mode ? "MANUAL" : "AUTO");
  }

  if (ticks == last_tick) return;
  last_tick = ticks;

  adc_value = analogRead(pot_pin);
  percent = adc_value * 100L / 4095;

  if (manual_mode) blink_time = 5;
  else if (percent < 40) blink_time = 0;
  else if (percent < 75) blink_time = 5;
  else blink_time = 1;

  if (blink_time == 0) led_on = false;
  else if (ticks % blink_time == 0) led_on = !led_on;
  digitalWrite(led_pin, led_on);

  if (ticks % 10 == 0) {
    Serial1.print("ADC: ");
    Serial1.print(adc_value);
    Serial1.print(" ; ");
    Serial1.print(percent);
    Serial1.print("% ; ");
    Serial1.print(manual_mode ? "MANUAL" : "AUTO");
    Serial1.print(" ; ");
    Serial1.println(percent < 40 ? "NORMAL" : percent < 75 ? "WARNING" : "CRITICAL");
  }
}
