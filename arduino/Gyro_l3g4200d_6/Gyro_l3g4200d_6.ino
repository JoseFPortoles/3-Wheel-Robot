#include <Wire.h>

#define CTRL_REG1 0x20 //Reg de ctrl 1: Define rango de salida, ancho de banda, modo de consumo y habilitacin ejes
#define CTRL_REG2 0x21 //Reg de ctrl 2: Configura filtro pasa alta  
#define CTRL_REG3 0x22 //Reg de ctrl 3: 
#define CTRL_REG4 0x23 //
#define CTRL_REG5 0x24 //Reg de ctrl 5: (Des)habilitar FIFO, Filtro pasa alta, Selección INT1 y Out
#define FIFO_CTRL_REG 0x2E //Selección de modo FIFO y marca de agua
#define FIFO_SRC_REG 0x2F 
#define OUT_X_L 0x28
#define OUT_X_H 0x29
#define OUT_Y_L 0x2A
#define OUT_Y_H 0x2B
#define OUT_Z_L 0x2C
#define OUT_Z_H 0x2D
#define STATUS_REG 0x27
#define LOGICA_DR_INT2 0
#define LONG_VAR 256
#define READS_LOOP 29 //Número de lecturas enviadas por ejecución de loop()
#define RISTRA 64 //Ristras de datos solicitadas de una vez a la FIFO

int lec = 0; //lectura del puerto i2c
volatile int ic = 0; //índice de llenado. Apunta a la primera posición vacía (En los arrays _VAR)
volatile int dir = 105; //Direccion por defecto del l3g4200d_1
volatile byte X_VAR[LONG_VAR]; //Velocidad Angular de Rotación en X {x_h[0],x_l[0],x_h[1],x_l[1],...}
volatile byte Y_VAR[LONG_VAR];
volatile byte Z_VAR[LONG_VAR];
int n = 0;
double S0 = 8.75;
int x = 0;
int y = 0;
int z = 0;
double xg = 0;
double yg = 0;
double zg = 0;
int pataLED = 12; //LED conectado a pata 12
int pataLED2 = 11; //LED conectado a pata 12
int pataIntDR = 2; //Interrupción Data Ready (DR) se lee en pata 2
double mdx = 0;
double mdy = 0;
double mdz = 0;
String msj;

void setup() {
  Wire.setClock(400000);
  Wire.begin();                // join i2c bus (address optional for master) (Arduino <--> Gyro)
  Serial.begin(9600);          // start serial communication at 9600bps (Arduino <--> Computer)
  // Paso 2: Configuramos el bicho
  //Primero habilitar ejes y tasa de lecturas
  Wire.beginTransmission(dir); // Establece conexion Gyro
  Wire.write(byte(CTRL_REG1));      // Vamos a tocar el primer registro de control
  Wire.write(0b00001111); // Habilitamos XYZ Pasamos de modo bajo consumo (power down) a modo normal. ODR = 100Hz, cut-off=12.5
  Wire.endTransmission();      // Cortamos la conexion
  //Segundo se configura el filtro pasa alta
  Wire.beginTransmission(dir); 
  Wire.write(byte(CTRL_REG2)); // Segundo registro de control     
  Wire.write(0b00000000); // Modo normal de reseteo. Se hace reset si se manda HP_RESET_FILTER. cut-off del filtro en 8 Hz 
  Wire.endTransmission();      
  Wire.beginTransmission(dir); //
  Wire.write(byte(CTRL_REG3));      
  Wire.write(0b00000100); // Habilitar interrupción en DR cuando se alcance el nivel de agua
  Wire.endTransmission();      
  Wire.beginTransmission(dir);
  Wire.write(byte(CTRL_REG4));      // sets register pointer to echo #1 register (0x02)
  Wire.write(0b00000000);//rango de escala 250 gps, actualización continua de los registros de salida
  Wire.endTransmission();      // stop transmitting
  Wire.beginTransmission(dir);
  Wire.write(byte(CTRL_REG5));      // sets register pointer to echo #1 register (0x02)
  Wire.write(0b01000000); //Habilitar FIFO. No user filtros pasa alta/baja para generar las salidas ni las interrupciones
  Wire.endTransmission();      // stop transmitting
  Wire.beginTransmission(dir);
  Wire.write(byte(FIFO_CTRL_REG));      // sets register pointer to echo #1 register (0x02)
  Wire.write(0b00010000); //Habilitar modo Bypass. Linea de agua en 16 (Sin sentido en modo bypass) 
  Wire.endTransmission();      // stop transmitting
  pinMode(pataLED, OUTPUT);
  pinMode(pataLED2, OUTPUT);
  pinMode(pataIntDR, INPUT);
  //attachInterrupt(digitalPinToInterrupt(pataIntDR), ISR_FIFOLect, LOGICA_DR_INT2);
  for(int j=0;j<100;j++){
  for(int i=0;i<10;i++){
    FIFOLect();
  }
  for (int i = 0; i<ic; i=i+2) {//VAR_ contienen elementos 0 a ic-1 de 1 byte cada uno 
    mdx = mdx + float((X_VAR[i+1]<<8)|X_VAR[i]);//acumulado velocidad angular en x (cruda)
    mdy = mdy + float((Y_VAR[i+1]<<8)|Y_VAR[i]);
    mdz = mdz + float((Z_VAR[i+1]<<8)|Z_VAR[i]);
  }
  ic = 0; //Se ponen los arrays _VAR a cero una vez vaciados
  }
  mdx = mdx/1000;//Zero level
  mdy = mdy/1000;
  mdz = mdz/1000;
}



void loop() {
  FIFOLect();//Lee un valor de cada eje
  VAR_Proc();
  if (n==READS_LOOP){
    msj = "X:" + String(xg*S0*1.5/1000000) + "Y:" + String(yg*S0*1.5/1000000) + "Z:" + String(zg*S0*1.5/1000000);
    Serial.println(msj);
    //Serial.println(xg*S0*1.5/1000000);
    //Serial.println(yg*S0*1.5/1000000);
    //Serial.println(zg*S0*1.5/1000000);
    n=0;
  }
  n++;
}

void VAR_Proc() {
    for (int i = 0; i<ic; i=i+2) {//VAR_ contienen elementos 0 a ic-1 de 1 byte cada uno 
      x = (X_VAR[i+1]<<8)|X_VAR[i]; //Lectura de x en un entero con signo 
      y = (Y_VAR[i+1]<<8)|Y_VAR[i];
      z = (Z_VAR[i+1]<<8)|Z_VAR[i];
      //traducir a grados/s
      //x es veloc. angular en mdps. mdx es deriva angular en mdps. x-mdx sería la velocidad angular real.
      //Al sumar todos los x-mdx queda la integral Int0->t(x-mdx)dt dividida por el intervalo de tiempo entre medidas.
      //1/ODR debería ser el intervalo entre medidas en segundos. 
      xg = xg + double(x) - mdx;
      yg = yg + double(y) - mdy;
      zg = zg + double(z) - mdz;
    }
    ic = 0; //Se ponen los arrays _VAR a cero una vez vaciados
}
void FIFOLect() {
  int st; //STATUS_REG
  int i=0;
  Wire.beginTransmission(dir);
  Wire.write(byte(STATUS_REG));// Leer STATUS_REG
  Wire.endTransmission();
  Wire.requestFrom(dir, 1); //Pedir registro entero
  st = Wire.read();
  if (st&240) {
    digitalWrite(pataLED,HIGH); //Datos perdidos
  }else{
    digitalWrite(pataLED,LOW); //No hay datos perdidos
  }
  //Serial.println(st);
  if (ic > 64) {
    digitalWrite(pataLED2,HIGH);
  }else{
    digitalWrite(pataLED2,LOW);
  }
  //Lectura de todos los ejes a la vez
  //while(ic<LONG_VAR){
  Wire.beginTransmission(dir);
  Wire.write(byte(OUT_X_L)|(1<<7));
  Wire.endTransmission();
  Wire.requestFrom(dir, 6);  
  while(Wire.available()<6) {
  }
  
  X_VAR[ic] = Wire.read();
  X_VAR[ic+1] = Wire.read();
  Y_VAR[ic] = Wire.read();
  Y_VAR[ic+1] = Wire.read();
  Z_VAR[ic] = Wire.read();
  Z_VAR[ic+1] = Wire.read();
  ic=ic+2;
}

