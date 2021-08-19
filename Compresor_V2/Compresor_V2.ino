/*  
 *  Programa: Compresor de aire
 *  Version: 2.5 (Final v1.0)
 *  
 *  Autor: Ezequiel Ledesma
 *  Fecha: 13/08/2021
 *  Empresa: United Scale Arts
 *  
 *  Descripcion: Control de compresor con corte por funcionamiento continuo o mantenimiento.
 *  Interfaz con LCD 16x2 y uso de memoria SD.
 *  
 *  Notas: Los archivos para la sd tienen que nombrarse en formato 8.3 (nombre max 8 caracteres y extension max 3 caracteres)
 *  
 *  Funcionamiento:
 *  1 - Programa inicia pines, lcd y sd
 *  2 - Comprueba SD y archivos
 *  2.a Si no hay sd, espera a que se introduzca una
 *  2.b Si falta uno de los dos archivos, muestra por lcd lo que tiene guardado y da opciones de borrar todo o seguir con ese y crear el archivo que falte
 *  2.c Si existe mucha diferencia con los datos guardados (mas de 15 segundos), los muestra por lcd y da opciones de cual tomar como valido
 *  3 - Programa listo para usar
 *  3.a Si el compresor funciona mas de 10 minutos, se necesita presionar el boton RESET para seguir.
 *  3.b Si el compresor supera el tiempo de Service, se para el programa hasta mantener el boton SERVICE por mas de 2 segundos
 *  3.c En todo momento, al presionar el boton SERVICE por mas de 2 segundos reinicia dicha cuenta y reinicia ambos archivos en SD
 *  4 - Cada 10 segundos (si el motor esta andando) o si el motor pasa de andando a parado o si se apaga la llave, se guarda la cuenta del Service intercaladamente entre archivo 1 y 2, comprobando que la sd siga estando
 *  4.a Si la SD no esta, el programa se frena y espera a que se introduzca una
 *  4.b Al introducir nuevamente la SD, comprueba como en 2 (comprobacion de SD y archivos) pero teniendo en cuenta que la ultima cuenta que no llego a guardarse es la valida
 *  
 */

#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>

File myFile;

 /* Memoria SD
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
 */

const int rs = 10, en = 9, d4 = 8, d5 = 7, d6 = 6, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Entradas
#define BUTTON_SERVICE      A5               // Boton que reinicia la cuenta del service
#define LLAVE               A2               // Llave que apaga/enciende el programa
#define BUTTON_RESET        A3               // Boton que reinicia la cuenta del funcionamiento continuo
#define S_MOTOR             A4               // Signal que se recibe del compresor cuando esta en funcionamiento

// Salidas
#define BACKLIGHT       3                    // Luz backlight del LCD 16x2
#define CONTACTOR       2                    // Contactor que habilita al compresor funcionar
#define LED_OFF         A1                   // Led indicador de apagado
#define LED_ON          A0                   // Led indicador de encendido

// Macros
#define MOTOR_ANDANDO       !digitalRead(S_MOTOR)
#define SERVICE_PRESIONADO  digitalRead(BUTTON_SERVICE)
#define LLAVE_ON            digitalRead(LLAVE)
#define LLAVE_OFF           !digitalRead(LLAVE)
#define RESET_PRESIONADO    digitalRead(BUTTON_RESET)
#define PRENDER_MOTOR       digitalWrite(CONTACTOR, LOW);
#define APAGAR_MOTOR        digitalWrite(CONTACTOR, HIGH);

// Tiempos
#define STOP_TIMEOUT    600000                //600000 (10 min) (1 - 1ms) Tiempo maximo en funcionamiento continuo
#define SERVICE_TIMEOUT 108000000             //108000000 (30 hs) (1 - 1ms) Tiempo maximo de funcionamiento

// Variables de estado
#define INICIO          0
#define REPOSO          1
#define STOP            2
#define OFF             3
#define SERVICE         4

// Variables Globales
uint8_t estado = INICIO;                      // Variable para cambiar de estados
volatile boolean timer_flag = false;          // Flag de la constante de tiempo
uint32_t timer_count = 0;                     // Contador para la constante de tiempo
boolean timer_display_flag = true;            // Flag para mostrar el display
uint32_t timer_display_count = 0;             // Contador para mostrar el display
boolean timer_sd_flag = false;                // Flag para guardar en SD
uint32_t timer_sd_count = 0;                  // Contador para guardar en SD
boolean arch_flag = true;                     // Variable para alternar en guardar entre comp1.txt y comp2.txt (true = comp1.txt)
unsigned long previousMillis = 0;             // Variable para poder usar un Timer por software (con millis())
const long interval = 10;                     // Ticks del Timer por software (10 -> 10ms/tick)
uint32_t fix = 0;                             // Variable para fixear cuenta de stop y service (para tener en cuenta el tiempo que se pierde guardando en sd o escribiendo LCD, y no se atrase)
#define DISPLAY_REFRESH 1000/interval         // Cada cuantos segundos se refresca el diplay
#define SD_REFRESH 10000/interval             // Cada cuantos segundos se guarda la cuenta de Service en SD
#define SERVICE_DELAY 2000/interval           // Cuantos segundos hay que mantener el boton Service para reiniciar la cuenta
boolean service_flag = false;                 // Flag para resetear service en caso de mantener el boton presionado
uint32_t service_delay = 0;                   // Contador para resetear el service en caso de mantener el boton presionado
uint32_t motorCount = 0;                      // Variable usada para llevar la cuenta del funcionamiento continuo
uint32_t motorServiceCount = 0;               // Variable usada para llevar la cuenta del Service
boolean motor_andando_flag = false;           // Flag para guardar cuando el motor pase de andando a parado

// Funciones
void pin_init();
boolean load_sd();
void softTimer();
void update_estado();
void save_sd();


void setup() {
  lcd.begin(16, 2);
  lcd.print("Iniciando LCD... ");
  pin_init();
  lcd.setCursor(0,1);
  lcd.print("Iniciando SD... ");
  lcd.clear();
  while(!load_sd());
  lcd.clear();
  delay(500);
}


void loop() {
  softTimer();
  update_estado();
}


void softTimer() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    if(MOTOR_ANDANDO && estado==REPOSO) fix += (currentMillis - previousMillis);
    previousMillis = currentMillis;
    timer_flag = true;
    timer_display_count++;
    timer_sd_count++;
    if (timer_display_count == DISPLAY_REFRESH) {
      timer_display_count = 0;
      timer_display_flag = true;
    }
    if (timer_sd_count == SD_REFRESH) {
      timer_sd_count = 0;
      timer_sd_flag = true;
    }
    if(service_flag) {
      if(service_delay) service_delay--;
    }
  }
}


void update_estado() {
  static uint16_t ledCount = 0;

  if (!timer_flag) return;
  timer_flag = false;
  
  switch(estado) {

    case INICIO:
      motorCount = 0;
      estado = REPOSO;
      break;

    // Compresor andando normalmente, contando el tiempo de service y controlando que no supere el tiempo maximo de funcionamiento continuo
    case REPOSO:
      if (LLAVE_OFF) {
        save_sd();
        estado = OFF;
      }
      else if (motorServiceCount >= SERVICE_TIMEOUT) {
        save_sd();
        estado = SERVICE;
      }
      else if (motorCount >= STOP_TIMEOUT) {
        save_sd();
        estado = STOP;
      }
      else {
        if (MOTOR_ANDANDO) {
          motor_andando_flag = true;
          motorCount+=fix;
          motorServiceCount+=fix;
          fix=0;
        }
        else motorCount = 0;
        PRENDER_MOTOR
        digitalWrite(LED_OFF, LOW);
        digitalWrite(LED_ON, HIGH);
      }
      if(!timer_display_flag) break;
      timer_display_flag = false;
      lcd.setCursor(0, 0);
      lcd.print("Motor ");
      if(!motorCount) lcd.print("OFF");
      else {
        lcd.print("ON ");
        lcd.print((uint32_t)(motorCount/60000));
        lcd.print("m ");
        lcd.print((uint32_t)((motorCount%60000)/1000));
        lcd.print("s");
      }
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("Serv ");
      lcd.print((uint32_t)((SERVICE_TIMEOUT-motorServiceCount)/3600000));
      lcd.print("h ");
      lcd.print((uint32_t)(((SERVICE_TIMEOUT-motorServiceCount)%3600000)/60000));
      lcd.print("m ");
      lcd.print((uint32_t)(((SERVICE_TIMEOUT-motorServiceCount)%60000)/1000));
      lcd.print("s");
      lcd.print("                ");
      break;

   // Si el compresor supero el tiempo maximo de funcionamiento continuo
   case STOP:
      APAGAR_MOTOR
      digitalWrite(LED_OFF, LOW);
      ledCount++;
      if(ledCount == 50) {
        ledCount = 0;
        digitalWrite(LED_ON, !digitalRead(LED_ON));
      }
      if (LLAVE_OFF) estado = OFF;
      else if (RESET_PRESIONADO) {
        motorCount = 0;
        estado = REPOSO;
      }
      if(!timer_display_flag) break;
      timer_display_flag = false;
      lcd.setCursor(0, 0);
      lcd.print("STOP. Precaucion");
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("Esperando reset!");
      lcd.print("                ");
      break;

   // Si el compresor supero el tiempo maximo de funcionamiento
   case SERVICE:
      static boolean text_flash = false;
      APAGAR_MOTOR
      ledCount++;
      if(ledCount == 50) {
        ledCount = 0;
        digitalWrite(LED_OFF, !digitalRead(LED_OFF));
        digitalWrite(LED_ON, !digitalRead(LED_ON));
        text_flash = !text_flash;
      }
      if (LLAVE_OFF) estado = OFF;
      if(text_flash) {
        digitalWrite(BACKLIGHT, LOW);
        break;
      }
      digitalWrite(BACKLIGHT, HIGH);
      if(!timer_display_flag) break;
      timer_display_flag = false;
      lcd.setCursor(0, 0);
      lcd.print("NECESITA SERVICE");
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("Call Mechatronic");
      lcd.print("                ");
      break;

   // Si la llave esta apagada
   case OFF:
      analogWrite(BACKLIGHT,10);
      APAGAR_MOTOR
      digitalWrite(LED_OFF, HIGH);
      digitalWrite(LED_ON, LOW);
      if(LLAVE_ON) {
        motorCount = 0;
        digitalWrite(BACKLIGHT, HIGH);
        estado = REPOSO;
      }
      if(!timer_display_flag) break;
      timer_display_flag = false;
      lcd.setCursor(0, 0);
      lcd.print("APAGADO");
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      timer_sd_flag = false;
      service_flag = false;
      break;

   default:
      estado = INICIO;
      break;
  }

  if( (timer_sd_flag && MOTOR_ANDANDO) || (!MOTOR_ANDANDO && motor_andando_flag) ) {
    motor_andando_flag = false;
    timer_sd_flag = false;
    save_sd();
  }

  // Debounce con delay de boton de servicio
  if(SERVICE_PRESIONADO && !service_flag) {
    service_flag = true;
    service_delay = SERVICE_DELAY;
  }
  else if (SERVICE_PRESIONADO && service_flag && !service_delay) {
    estado = INICIO;
    motorCount = 0;
    motorServiceCount = 0;
    digitalWrite(BACKLIGHT, HIGH);
    APAGAR_MOTOR
    digitalWrite(LED_OFF, HIGH);
    digitalWrite(LED_ON, HIGH);
    for(uint8_t i = 0; i < 2; i++) {
      save_sd();
    }
  }
  if (!SERVICE_PRESIONADO && service_flag) {
    service_flag = false;
  }
}


void save_sd() {
  // Guardado de datos en SD
  if(arch_flag) {
    arch_flag = false;
    myFile = SD.open("comp1.txt", O_WRITE);
  }
  else {
    arch_flag = true;
    myFile = SD.open("comp2.txt", O_WRITE);
  }
  if(myFile) {
    myFile.write((byte *) &motorServiceCount, 4);
    myFile.close();
  }
  else {
    lcd.setCursor(0,0);
    lcd.print("Error GUARDANDO");
    lcd.setCursor(0,1);
    lcd.print("el archivo en SD");
    digitalWrite(BACKLIGHT, HIGH);
    APAGAR_MOTOR
    digitalWrite(LED_OFF, HIGH);
    digitalWrite(LED_ON, LOW);
    motorCount = 0;
    delay(400);
    while(!load_sd());
    previousMillis = millis();
  }
}


boolean load_sd() {
  delay(300);
  // Si no se pudo inicializar la SD
  if (!SD.begin(4)) {
    lcd.setCursor(0,0);
    lcd.print("Error al INICIAR");
    lcd.setCursor(0,1);
    lcd.print("la SD...        ");
    return false;
  }

  uint32_t comp1 = 0, comp2 = 0;
  boolean arch_status1 = false, arch_status2 = false;
  // Si ya existe el archivo 1
  if (SD.exists("comp1.txt")) {
    arch_status1 = true;
    myFile = SD.open("comp1.txt");
    if(myFile) {
      while(myFile.available()) {
        myFile.read((byte *) &comp1, 4);
        break;
      }
      myFile.close();
    }
    else {
      lcd.setCursor(0,0);
      lcd.print("Error al ABRIR 1");
      lcd.setCursor(0,1);
      lcd.print("el archivo en SD");
      return false;
    }
  }
  // Si ya existe el archivo 2
  if (SD.exists("comp2.txt")) {
    arch_status2 = true;
    myFile = SD.open("comp2.txt");
    if(myFile) {
      while(myFile.available()) {
        myFile.read((byte *) &comp2, 4);
        break;
      }
      myFile.close();
    }
    else {
      lcd.setCursor(0,0);
      lcd.print("Error al ABRIR 2");
      lcd.setCursor(0,1);
      lcd.print("el archivo en SD");
      return false;
    }
  }

  if(arch_status1 && (motorServiceCount > comp1)) comp1 = motorServiceCount;
  if(arch_status2 && (motorServiceCount > comp1)) comp2 = motorServiceCount;

  if(arch_status1 && arch_status2) {         // Comprobar que no haya error entre los archivos, tomar el mayor como valido
    if(comp1 > SERVICE_TIMEOUT) comp1 = 0;
    if(comp2 > SERVICE_TIMEOUT) comp2 = 0;
    if( ((comp1>=comp2) && (comp1-comp2 > 15000)) || ((comp2>=comp1) && (comp2-comp1 > 15000)) )   { // 15000 -> 15 seg de dif
      lcd.setCursor(0,0);
      lcd.print("Elegir 1-Serv ");
      lcd.print((uint32_t)((SERVICE_TIMEOUT-comp1)/3600000));
      lcd.print("h ");
      lcd.print((uint32_t)(((SERVICE_TIMEOUT-comp1)%3600000)/60000));
      lcd.print("m ");
      lcd.print((uint32_t)(((SERVICE_TIMEOUT-comp1)%60000)/1000));
      lcd.print("s");
      lcd.print(" - Boton Service");
      lcd.setCursor(0,1);
      lcd.print("Elegir 2-Serv ");
      lcd.print((uint32_t)((SERVICE_TIMEOUT-comp2)/3600000));
      lcd.print("h ");
      lcd.print((uint32_t)(((SERVICE_TIMEOUT-comp2)%3600000)/60000));
      lcd.print("m ");
      lcd.print((uint32_t)(((SERVICE_TIMEOUT-comp2)%60000)/1000));
      lcd.print("s");
      lcd.print(" - Boton Reset");
      while(1) {
        if(SERVICE_PRESIONADO) {
          motorServiceCount = comp1;
          break;
        }
        if(RESET_PRESIONADO) {
          motorServiceCount = comp2;
          break;
        }
        lcd.scrollDisplayLeft();
        delay(500);
      }
    }
    else if (comp1 >= comp2) motorServiceCount = comp1;
    else motorServiceCount = comp2;
  }
  else if(arch_status1 && !arch_status2) {   // Autotest: Si existe el 1 pero no el 2, error que paso con el 2?
    lcd.setCursor(0,0);
    lcd.print("Falta archivo 2 - Service = continuar");
    lcd.setCursor(0,1);
    lcd.print("Go? Serv ");
    lcd.print((uint32_t)((SERVICE_TIMEOUT-comp1)/3600000));
    lcd.print("h ");
    lcd.print((uint32_t)(((SERVICE_TIMEOUT-comp1)%3600000)/60000));
    lcd.print("m ");
    lcd.print((uint32_t)(((SERVICE_TIMEOUT-comp1)%60000)/1000));
    lcd.print("s");
    lcd.print(" - Reset = reiniciar");
    while(1) {
      if(SERVICE_PRESIONADO) {
        motorServiceCount = comp1;
        break;
      }
      else if (RESET_PRESIONADO) {
        motorServiceCount = 0;
        break;
      }
      lcd.scrollDisplayLeft();
      delay(500);
    }
  }
  else if(!arch_status1 && arch_status2) {   // Autotest: Si existe el 2 pero no el 1, error que paso con el 1?
    lcd.setCursor(0,0);
    lcd.print("Falta archivo 1 - Service = continuar");
    lcd.setCursor(0,1);
    lcd.print("Go? Serv ");
    lcd.print((uint32_t)((SERVICE_TIMEOUT-comp2)/3600000));
    lcd.print("h ");
    lcd.print((uint32_t)(((SERVICE_TIMEOUT-comp2)%3600000)/60000));
    lcd.print("m ");
    lcd.print((uint32_t)(((SERVICE_TIMEOUT-comp2)%60000)/1000));
    lcd.print("s");
    lcd.print(" - Reset = reiniciar");
    while(1) {
      if(SERVICE_PRESIONADO) {
        motorServiceCount = comp2;
        break;
      }
      else if (RESET_PRESIONADO) {
        motorServiceCount = 0;
        break;
      }
      lcd.scrollDisplayLeft();
      delay(500);
    }
  }

  // Si no existe el archivo 1, crearlo
  if (!arch_status1) {
    myFile = SD.open("comp1.txt", FILE_WRITE);
    if(myFile) {
      myFile.write((byte *) &motorServiceCount, 4);
      myFile.close();
    }
    else {
      lcd.setCursor(0,0);
      lcd.print("Error al CREAR 1");
      lcd.setCursor(0,1);
      lcd.print("el archivo en SD");
      return false;
    }
  }

  // Si no existe el archivo 2, crearlo
  if (!arch_status2) {
    myFile = SD.open("comp2.txt", FILE_WRITE);
    if(myFile) {
      myFile.write((byte *) &motorServiceCount, 4);
      myFile.close();
    }
    else {
      lcd.setCursor(0,0);
      lcd.print("Error al CREAR 2");
      lcd.setCursor(0,1);
      lcd.print("el archivo en SD");
      return false;
    }    
  }
  lcd.clear();
  digitalWrite(BACKLIGHT, HIGH);
  delay(500);
  return true;
}


void pin_init() {
  pinMode(BUTTON_SERVICE, INPUT);
  pinMode(LLAVE, INPUT);
  pinMode(BUTTON_RESET, INPUT);
  pinMode(S_MOTOR, INPUT);
  pinMode(LED_OFF, OUTPUT);
  pinMode(LED_ON, OUTPUT);
  pinMode(CONTACTOR, OUTPUT);
  pinMode(BACKLIGHT, OUTPUT);
  digitalWrite(BACKLIGHT, HIGH);
  digitalWrite(LED_OFF, HIGH);
  digitalWrite(LED_ON, LOW);
  APAGAR_MOTOR
}
