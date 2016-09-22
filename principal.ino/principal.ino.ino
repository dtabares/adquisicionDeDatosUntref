/***************************************************************************
1. Establecer un rumbo en grados respecto del Norte magnético y mientras

esté en modo de navegación mantenga ese rumbo.

2. En caso de salirse del rumbo indicar hacia donde debe moverse para

corregir el rumbo.

3. Que permita establecer un plan de navegación con rumbos y tiempos.

Ejemplo: 30º/10 segundos, -60º/15 segundos y 90º/5 segundos. Además,

debe cumplir con el punto anterior de mantener el rumbo y avisar en

caso de salirse.

4. Indicar de manera visual proporcionalmente dependiendo de cuán lejos

esté del rumbo. Si está muy lejos, que la indicación sea más acentuada.

5. Comunicar la corrección que debe hacer mediante algún enlace hacia el

exterior del sistema.

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
int rumbosDeNavegacion[3]={43,70,180};
int tiemposDeNavegacion[3]={10,20,30};
boolean finalizado=false;


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
  /* Configuramos los puertos digitales 2 y 4 como salidas */
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);

  /* Inicializamos el puerto serial */
  Serial.begin(9600);
  Serial.println("Trabajo Practico - Sistema de Navegacion");Serial.println("");  
  
  /* Inicializamos el sensor */
  if(!mag.begin())
  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    while(1);
  }
}

void encenderLed(int salidaAnalogica){
  
  digitalWrite(salidaAnalogica, HIGH);
  
}

void apagarLed(int salidaAnalogica){
  
  digitalWrite(salidaAnalogica, LOW);
  
}

void navegar(int rumbo){
    
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
      apagarLed(2);
      apagarLed(4);
    }
    else
    {
      if (moduloDesvio < 180)
      {
        Serial.print("Hay que corregir el curso en "); Serial.print(moduloDesvio); Serial.print(" grados ");
        if (desvio > 0)
        {
          Serial.println("a la derecha");
          encenderLed(2);
        }
        else
        {
          Serial.println("a la izquierda");
          encenderLed(4);
        }
      }
      else
      {
        float desvioComplementario = 360 - moduloDesvio;
        Serial.print("Hay que corregir el curso en "); Serial.print(desvioComplementario); Serial.print(" grados ");
        if (desvio < 0)
        {
          Serial.println("a la derecha");
          encenderLed(2);
        }
        else
        {
          Serial.println("a la izquierda");
          encenderLed(4);
        }
      }
      
    }
    
    Serial.print("Rumbo (grados): "); Serial.println(moda);
    //Serial.print("Desvio (grados): "); Serial.println(desvio);
    delay(1000);
   
}

void navegarConPlanDeNavegacion(int rumbosDeNavegacion[3], int tiemposDeNavegacion[3]){
  
  for (int i = 0; i<3; i++){
    int contadorDeNavegacion = 0;
    Serial.println("---------------------------------------------------------");
    Serial.print("Navegando con rumbo: "); Serial.print(rumbosDeNavegacion[i]); Serial.print(" grados");Serial.print(" Tiempo: "); Serial.print(tiemposDeNavegacion[i]); Serial.print(" segundos");Serial.println("");
    Serial.println("---------------------------------------------------------");
    while (contadorDeNavegacion < tiemposDeNavegacion[i]){
      navegar(rumbosDeNavegacion[i]);
      contadorDeNavegacion++;
    }
  }
  Serial.println("-----------------------------");
  Serial.println("Plan de Navegacion Finalizado");
  Serial.println("-----------------------------");
  finalizado=true;
}

void loop(void) 
{
  
  if(contador > 30)
  {
    if (finalizado==false){
      navegarConPlanDeNavegacion(rumbosDeNavegacion,tiemposDeNavegacion);
    }
  }
  contador = contador +1;
  
}
