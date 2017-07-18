#pragma once
#include <cstdint>
 #include <Arduino.h>
 #include <NeoPixelBrightnessBus.h>

#define LIGHT_MAX_VALUE         255

class EspurnaLight {
public:
  enum class Modes {RGB, White, Temperature};
  struct Rgb {
    unsigned char r;
    unsigned char g;
     unsigned char b ;
   };

   EspurnaLight();

   void SetColor(String value);
   void SetTemperature(String value);
   void SetWhite(String value);
   void SetBrightness(String value);

   void PrintStatus();

   Rgb GetColor() {return rgbBeforeBrightness;}
   unsigned int GetBrightness() {return brightness;}
   unsigned int GetWhite() {return white;}
   uint32_t GetTemperature() {return temperature;}

private:
  Modes mode;
  //Rgb rgb;

  Rgb rgbBeforeBrightness;
  unsigned int brightness;
  unsigned int white;
  uint32_t temperature;

  NeoPixelBrightnessBus<::NeoGrbFeature, ::NeoEsp8266BitBang800KbpsMethod>* strip;
  void Update();

  void ApplyBrightness(unsigned int r, unsigned int g, unsigned int b);


  unsigned int _lightColor[3] = {0};

  // FROM ESPURNA
  void color_string2array(const char * rgb, unsigned int * array);
  void color_temperature2array(unsigned int temperature, unsigned int * array);

};
