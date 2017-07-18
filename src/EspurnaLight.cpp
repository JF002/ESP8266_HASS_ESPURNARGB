 #include "EspurnaLight.hpp"

EspurnaLight::EspurnaLight() {
  strip = new NeoPixelBrightnessBus<::NeoGrbFeature, ::NeoEsp8266BitBang800KbpsMethod>(1,4);
  brightness = 255;
}

 void EspurnaLight::SetColor(String value) {
   this->color_string2array(value.c_str(), _lightColor);
   Serial.println("New Color : " + String(_lightColor[0]) + ',' + String(_lightColor[1]) + ',' + String(_lightColor[2]));
   this->mode = Modes::RGB;

   rgbBeforeBrightness.r = _lightColor[0];
   rgbBeforeBrightness.g = _lightColor[1];
   rgbBeforeBrightness.b = _lightColor[2];
   ApplyBrightness(_lightColor[0], _lightColor[1], _lightColor[2]);
   Update();
 }

 void EspurnaLight::SetTemperature(String value) {
   char * p = (char *)value.c_str();
   unsigned int temperature = atoi(p);
   unsigned int convertedTemperature = 1000000.0 / temperature;
   Serial.println("Converted temperature : " + String(convertedTemperature));
   color_temperature2array(convertedTemperature, _lightColor);
   rgbBeforeBrightness.r = _lightColor[0];
   rgbBeforeBrightness.g = _lightColor[1];
   rgbBeforeBrightness.b = _lightColor[2];
   ApplyBrightness(_lightColor[0], _lightColor[1], _lightColor[2]);
   Serial.println("New Color : " + String(_lightColor[0]) + ',' + String(_lightColor[1]) + ',' + String(_lightColor[2]));
   this->mode = Modes::Temperature;
   this->temperature = temperature;
   Update();
 }

void EspurnaLight::SetBrightness(String value) {
  char * p = (char *)value.c_str();
  unsigned int newBrightness = atoi(p);

  this->brightness = newBrightness;
  ApplyBrightness(rgbBeforeBrightness.r, rgbBeforeBrightness.g, rgbBeforeBrightness.b);

  Serial.println("New Color : " + String(_lightColor[0]) + ',' + String(_lightColor[1]) + ',' + String(_lightColor[2]));
  Update();
}

void EspurnaLight::ApplyBrightness(unsigned int r, unsigned int g, unsigned int b) {
  uint16_t scale;
  if(this->brightness == 0)
    scale = 0;
  else if (this->brightness == 255)
    scale = 255;
  else
    scale = (((uint16_t)this->brightness << 8) - 1) / 255;


  _lightColor[0] = (r * scale) >> 8;
  _lightColor[1] = (g * scale) >> 8;
  _lightColor[2] = (b * scale) >> 8;
}

void EspurnaLight::SetWhite(String value) {
  char * tok = (char*) value.c_str();
  _lightColor[0] = atoi(tok);
  _lightColor[1] = _lightColor[0];
  _lightColor[2] = _lightColor[0];

  rgbBeforeBrightness.r = _lightColor[0];
  rgbBeforeBrightness.g = _lightColor[1];
  rgbBeforeBrightness.b = _lightColor[2];

  ApplyBrightness(_lightColor[0], _lightColor[1], _lightColor[2]);

  Serial.println("New Color : " + String(_lightColor[0]) + ',' + String(_lightColor[1]) + ',' + String(_lightColor[2]));
  this->mode = Modes::White;
  this->white = _lightColor[0];
  Update();
}

void EspurnaLight::Update() {
  strip->SetPixelColor(0,RgbColor(_lightColor[0],_lightColor[1],_lightColor[2]));
  strip->Show();
}

void EspurnaLight::PrintStatus() {
  switch(this->mode) {
    case Modes::RGB:
      Serial.println("Mode : RGB (" + String(rgbBeforeBrightness.r) + ", " + String(rgbBeforeBrightness.g) + ", " + String(rgbBeforeBrightness.b) + ")");
      break;
    case Modes::Temperature: Serial.println("Mode : Temperature (" + String(temperature) + ")"); break;
    case Modes::White: Serial.println("Mode : White (" + String(white) + ")"); break;
  }
}


/* -------------------------------------------------------------------------- */


void EspurnaLight::color_string2array(const char * rgb, unsigned int * array) {
  char * p = (char *) rgb;
  char * tok;
  tok = strtok(p, ",");
  array[0] = atoi(tok);
  tok = strtok(NULL, ",");

  // if there are more than one value assume R,G,B
  if (tok != NULL) {
      array[1] = atoi(tok);
      tok = strtok(NULL, ",");
      if (tok != NULL) {
          array[2] = atoi(tok);
      } else {
          array[2] = 0;
      }
  }
}



void EspurnaLight::color_temperature2array(unsigned int temperature, unsigned int * array) {

    // Force boundaries and conversion
    temperature = constrain(temperature, 1000, 40000) / 100;

    // Calculate colors
    unsigned int red = (temperature <= 66)
        ? LIGHT_MAX_VALUE
        : 329.698727446 * pow((temperature - 60), -0.1332047592);
    unsigned int green = (temperature <= 66)
        ? 99.4708025861 * log(temperature) - 161.1195681661
        : 288.1221695283 * pow(temperature, -0.0755148492);
    unsigned int blue = (temperature >= 66)
        ? LIGHT_MAX_VALUE
        : ((temperature <= 19)
            ? 0
            : 138.5177312231 * log(temperature - 10) - 305.0447927307);

    // Save values
    array[0] = constrain(red, 0, LIGHT_MAX_VALUE);
    array[1] = constrain(green, 0, LIGHT_MAX_VALUE);
    array[2] = constrain(blue, 0, LIGHT_MAX_VALUE);
}
