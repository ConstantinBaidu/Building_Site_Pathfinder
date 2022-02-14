#include <Zumo32U4.h>
#include "TurnSensor.h"
Zumo32U4Buzzer buzzer;
Zumo32U4LineSensors lineSensors;
Zumo32U4Motors motors;
Zumo32U4ButtonA buttonA;
Zumo32U4LCD lcd;
L3G gyro;

//Sensori e Limiti ----------------------------------------------------
//Serve ad istanziare i sensori ed i limiti dei sensori.
int16_t s1 = 0; //Sensore di Sinistra
int16_t s2 = 0; //Sensore di Centro Sinistra
int16_t s3 = 0; //Sensore di Centro
int16_t s4 = 0; //Sensore di Centro Destra
int16_t s5 = 0; //Sensore di Destra

int16_t thH = 800; //Limite Massimo (di Posizionamento su LINEA NERA)
int16_t thM = 200; //Limite Intermedio (di Variazione Posizione)
int16_t thL = 10;  //Limite Basso (di STOP)

//Variabili Utili ----------------------------------------------------
uint16_t msLeft = 100; //Velocità Motore di sinistra
uint16_t msRight = 100; //Velocità Motore di destra

bool over = true; //variabile usata per sapere se il robot è sulla linea
bool change = false; 
bool iterate = true;
String listaComandi = "A"; //La lista dei comandi pervenuti dall'app
char mode = "A"; //In quale modalità si trova il robot, tragitto o Joystick

#define NUM_SENSORS 5
unsigned int lineSensorValues[NUM_SENSORS]; //Array di sensori

//Metodi per l'uso del Joystick --------------------------------------
void forwarJoystick() { //Metodo movimento avanti
  motors.setSpeeds(msLeft, msRight);
}
void backwarJoystick() { //Metodo movimento indietro
  motors.setSpeeds(-msLeft, -msRight);
}
void rightJoystick() { //Metodo movimento gira a destra
  motors.setSpeeds(-msLeft, msRight);
}
void leftJoystick() { //Metodo movimento gira a sinistra
  motors.setSpeeds(msLeft, -msRight);
}
void fermoJoystick() { //Metoto di stop
  motors.setSpeeds(0, 0);
}

//Metodi di Aiuto -----------------------------------------------------

//Questo metodo ti da l'angolo attuale del robot
//dall'ultimo reset del sensore
int32_t getAngle() {
  return (((int32_t)turnAngle >> 16) * 360) >> 16;
}

//Metodo per la calibrazione dei sensori del colore del robot
//Serve a riconoscere la differenza tra nero(1000) e bianco(0)
void calibrateSensors()
{
  //Aspettare 1 secondo per la calibrazione Automatica
  //facendo ruotare il robot sulla linea nera
  delay(1000);
  for (uint16_t i = 0; i < 120; i++)
  {
    if (i > 30 && i <= 90) {
      motors.setSpeeds(-150, 150);
    } else {
      motors.setSpeeds(150, -150);
    }
    lineSensors.calibrate();
  }
  motors.setSpeeds(0, 0);
}

//Metodo che serve ad aggiornare i sensori
void updateLineSensorValues() {
  lineSensors.readCalibrated(lineSensorValues);
  s1 = lineSensorValues[0];
  s2 = lineSensorValues[1];
  s3 = lineSensorValues[2];
  s4 = lineSensorValues[3];
  s5 = lineSensorValues[4];
}

//Metodo che serve ad inizializzare i sensori
void initiateSensors() {
  updateLineSensorValues();
  lcd.clear();
  lcd.gotoXY(0, 0);
  lcd.print(s2);
  lcd.print("-");
  lcd.print(s3);
  lcd.gotoXY(0, 1);
  int16_t middle = s2 + s3 + s4 + s1 + s5;
  lcd.print(s4);
}

//Metodi di Movimento -------------------------------------------------

// Metodo di Movimento verso SINISTRA
void turnLeft() {
  /*turnSensorUpdate();
    int32_t angle = 0;

    while (angle <= 90) {
    if (s3 > 700) {
      break;
    }
    motors.setSpeeds(-100, 100);
    turnSensorUpdate();
    angle = getAngle();
    delay(200);
    initiateSensors();
    if (s3 > 250) {
      break;
    }
    }

    motors.setSpeeds(0, 0);
    angle = 0;
    turnSensorReset();*/

  //In caso si debba girare serve il sensore inizialmente a 0
  s3 = 0;
  motors.setSpeeds(-100, 100);
  delay(400);
  while (s3 < 300) {
    updateLineSensorValues();
  }
  motors.setSpeeds(0, 0);
}

//Metodo di Movimento verso DESTRA
void turnRight() {
  /*turnSensorUpdate();
    int32_t angle = 0;

    while (angle >= -90) {
    motors.setSpeeds(100, -100);
    turnSensorUpdate();
    angle = getAngle();
    delay(200);
    initiateSensors();
    if (s3 > 250) {
      break;
    }
    }

    motors.setSpeeds(0, 0);
    angle = 0;
    turnSensorReset();*/

  //In caso si debba girare serve il sensore inizialmente a 0
  s3 = 0;
  motors.setSpeeds(100, -100);
  delay(400);
  while (s3 < 300) {
    updateLineSensorValues();
  }
  motors.setSpeeds(0, 0);
}

//Metodi di Condizione ------------------------------------------------

//Metodo che indica che il Robot è sulla Linea
bool overTheLine() {
  if (s2 > thH && s4 > thH) {
    return true;
  } else {
    return false;
  }
  /*
     } else if((s1 > thH && s5 > thH) || (s1 > thH && s2 > thH) || (s4 > thH && s5 > thH)) {
    return true;
    }
  */
}
//Metodo che indica che il robot è fuori dalla linea (su spazio bianco)
bool outOfTheCircuit() {
  if (s1 < thL && s2 < thL && s3 < thL && s4 < thL && s5 < thL ) {
    return true;
  } else {
    return false;
  }
}

//Metodo serve a posizionare il robot sulla linea da seguire
//Centrare il movimento sulla linea
void centrateTheLine() {
  if ((s2 > thM && s2 < thH) && s4 < thM) {
    msLeft = msLeft - 20;
    msRight = msRight + 20;
  } else if (s2 < thM && (s4 > thM && s4 < thH)) {
    msLeft = msLeft + 10;
    msRight = msRight - 10;
  } else {
    msLeft = 100;
    msRight = 100;
  }
}

//Questo metodo serve a ritornare il comando in forma decodificata
//Decodifica da simboli a Numeri, rendendo più facile l'interpretazione
int ritornaComando(char comando) {
  if (comando == '-')
    return -1;
  else if (comando == '+')
    return -2;
  else if (comando == 'F')
    return -3;
  else
    return int(comando) - 48;
}

//Metodi Principale ---------------------------------------------------
//Metodo che trasforma il robot in una macchinina telecomandata
void joystick() {
  ledGreen(1);
  while (true) {
    switch (Serial1.read()) {
      case 'A':
        forwarJoystick();
        break;
      case 'I':
        backwarJoystick();
        break;
      case 'D':
        rightJoystick();
        break;
      case 'S':
        leftJoystick();
        break;
      case 's':
        fermoJoystick();
        break;
      case 'L': //Clacson e lucina
        ledYellow(1);
        buzzer.playFromProgramSpace(PSTR("!L16 cdecde"));
        while (buzzer.isPlaying());
        break;
      case 'l':
        ledYellow(0);
        break;
      case 'P':
        if (msRight < 400 && msLeft < 400) {
          msRight = msRight + 50;
          msLeft = msLeft + 50;
        }
        break;
      case 'M':
        if (msRight > 0 && msLeft > 0) {
          msRight = msRight - 50;
          msLeft = msLeft - 50;
        }
        break;
      case 'T': //entrare nella modalità ricerca tragitto
        mode = 'T';
        ledGreen(0);
        break;
    }
    if (mode == 'T') {
      break; //Fine While
    }
  }

}

//Metodo che serve a far muovere il robot sul tragitto ricevuto
void pathfinder() {
  //Attesa della lista dei comandi da eseguire
  while (listaComandi.charAt(0) != "C") { //Lettera C che indica i comandi da eseguire
    ledYellow(1);
    delay(500);
    ledYellow(0);
    delay(500);
    listaComandi = Serial1.readString();
    if (listaComandi.charAt(0) == 'C') {
      buzzer.playFromProgramSpace(PSTR("!L16 cdegreg4"));
      listaComandi = listaComandi.substring(1);
      //Serial.println(listaComandi);
      break;
    }
    if (listaComandi.charAt(0) == 'J') { //In caso si cambi Mode
      mode = 'J';
      break;
    }
  }

  //Carichiamo il primo comando da eseguire
  if (mode == 'T') {
    int comando = 0;
    if (listaComandi.length() > 0) {
      comando = ritornaComando(listaComandi.charAt(0));
      listaComandi = listaComandi.substring(1);
    }

    //Esecuzione del tragitto in base ai comandi registrati
    while (iterate) {

      if (Serial1.read() == 'J') {
        ledRed(1);
        mode = 'J';
        iterate = false;
        break;
      }

      if (comando == -1) {
        turnLeft();
        change = true;
      }
      if (comando == -2) {
        turnRight();
        change = true;
      }
      if (comando == -3) {
        iterate = false;
        ledYellow(1);
        ledGreen(1);
        ledRed(1);
        buzzer.playFromProgramSpace(PSTR("!L16 cdegreg4"));
        delay(2000);
        ledYellow(0);
        ledGreen(0);
        ledRed(0);
        mode == 'A';
        break;
      }
      if (comando == 0) {
        ledGreen(1);
        motors.setSpeeds(msLeft, msRight);
        delay(400);
        ledGreen(0);
        change = true;
      }
      if (overTheLine() && comando >= 1 && over) {
        comando = comando - 1;
        ledYellow(1);
        over = false;
      }
      if (!overTheLine() && comando >= 1 && !over) {
        ledYellow(0);
        over = true;
      }

      if (iterate and comando >= 1) {
        //forwardStreight();
        motors.setSpeeds(msLeft, msRight);
      } else {
        motors.setSpeeds(0, 0);
      }

      updateLineSensorValues();
      centrateTheLine();

      if (change) {
        comando = ritornaComando(listaComandi.charAt(0));
        listaComandi = listaComandi.substring(1);
        change = false;
      }
      if (outOfTheCircuit() && iterate) { //In caso si esca dal tragitto
        turnRight();
      }
    }
  }
  listaComandi = "A";
  iterate = true;
}


//Metodo di Inizializzazione del Robot --------------------------------
void setup() {

  //Inizializzazione Gyroscopio
  //turnSensorSetup();
  //delay(500);
  //turnSensorReset();

  //Inizializzazione Porta seriale Trasmissione Dati via BT
  Serial1.begin(9600);
  //pinMode(LED_BUILTIN, OUTPUT);

  //Inizializzazione la linea dei Sensori per il Rilevamento della linea nera
  lineSensors.initFiveSensors(); //Inizializza i 5 Sensori s1-s5
  buttonA.waitForButton(); //Attesa pulsante A per inizio Calibrazione
  calibrateSensors();
}

//Metodo Principale ---------------------------------------------------

void loop() {
  mode = Serial1.read();
  if (mode == 'J') {
    joystick();
  }
  if (mode == 'T') {
    pathfinder();
  }
  ledRed(1);
  delay(500);
  ledRed(0);
  delay(500);
}

//---------------------------------------------------------------------
