/***************************************************************************
  This is a library example for the HMC5883 magnentometer/compass

  Designed specifically to work with the Adafruit HMC5883 Breakout
  http://www.adafruit.com/products/1746
 
  *** You will also need to install the Adafruit_Sensor library! ***

  These displays use I2C to communicate, 2 pins are required to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Kevin Townsend for Adafruit Industries with some heading example from
  Love Electronics (loveelectronics.co.uk)
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the version 3 GNU General Public License as
 published by the Free Software Foundation.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 ***************************************************************************/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
#include <QuickStats.h>

/* Assign a unique ID to this sensor at the same time */
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);
int contador = 0;
int error = 2;
int rumbo = 43;
float declinationAngle = 0.68;
const int MAXMUESTREO = 20;
float muestras[MAXMUESTREO];
QuickStats stats;

void displaySensorDetails(void)
{
  sensor_t sensor;
  mag.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" uT");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" uT");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" uT");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

void setup(void) 
{
  Serial.begin(9600);
  Serial.println("HMC5883 Magnetometer Test"); Serial.println("");
  
  /* Initialise the sensor */
  if(!mag.begin())
  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    while(1);
  }
  
  /* Display some basic information on this sensor */
  //displaySensorDetails();
}

void loop(void) 
{
  
  if(contador > 30)
  {
    int i = 0;
    while (i < MAXMUESTREO)
    {
      /* Get a new sensor event */ 
      sensors_event_t event; 
      mag.getEvent(&event);
      float heading = atan2(event.magnetic.y, event.magnetic.x);
      heading += declinationAngle;
      // Correct for when signs are reversed.
      if(heading < 0)
        heading += 2*PI;
        
      // Check for wrap due to addition of declination.
      if(heading > 2*PI)
        heading -= 2*PI;
       
      // Convert radians to degrees for readability.
      float rumboActual = heading * 180/M_PI;
      
      muestras[i] = rumboActual;
      i++;
    }

    float moda = stats.mode(muestras,MAXMUESTREO);


    float desvio = rumbo - moda;
    float moduloDesvio = abs(desvio);

    if (moduloDesvio < error)
    {
      Serial.println("Estamos en curso");
    }
    else
    {
      if (moduloDesvio < 180)
      {
        Serial.print("Hay que corregir el curso en "); Serial.print(moduloDesvio); Serial.print(" grados ");
        if (desvio > 0)
        {
          Serial.println("a la derecha");
        }
        else
        {
          Serial.println("a la izquierda");
        }
      }
      else
      {
        float desvioComplementario = 360 - moduloDesvio;
        Serial.print("Hay que corregir el curso en "); Serial.print(desvioComplementario); Serial.print(" grados ");
        if (desvio < 0)
        {
          Serial.println("a la derecha");
        }
        else
        {
          Serial.println("a la izquierda");
        }
      }
      
    }
    
    Serial.print("Rumbo (grados): "); Serial.println(moda);
    //Serial.print("Desvio (grados): "); Serial.println(desvio);
    delay(1000);
    
  }
  contador = contador +1;
}
