#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

// Numero di campioni fra cui determinare il picco della tensione alternata
#define campioni 30
// Potenza minima misurabile (in Watt)
#define watt_minimi 25
// Tensione di riferimento
#define tensione_rif 220 // V

// AHServer
// Server IP address = 93.48.49.218
IPAddress server_ip(93, 48, 49, 218);
// Server port = 16000
int port = 16000;

EthernetUDP Udp;

// Arduino's MAC address
byte mac[] = {
        0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

// Arduino's IP address
IPAddress ip(192, 168, 0, 177);

// variabili della parte di CSD
int current_code;
double volt;
double volt_max;
double volt_rms;
double volt_cal_avg = 0.0;
double power;
int watt_min = watt_minimi;

#define sensor_id 0;

// struct che verrà inviata nel pacchetto
typedef struct {
    uint16_t id;
    uint16_t power;
} Pacchetto;


void setup() {
    Ethernet.init(5);

    // Open serial communications and wait for port to open:
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    Serial.println("CSD Arduino porting with Ethernet");

    // start the Ethernet connection:
    Ethernet.begin(mac, ip);

    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        Serial.println("Ethernet shield not found \n");
        while (true) {
            delay(1); // do nothing, no point running without Ethernet hardware
        }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
        Serial.println("Ethernet cable not connected \n");
    }

    Udp.begin(16000);

    Serial.println("\nAVVIO CALIBRAZIONE");

    // 10 fasi di calibrazione del sensore
    for(int i=1; i<=10; i++) {
        // Azzero il picco misurato
        volt_max = 0.0;

        for(int j=0; j<campioni; j++) {
            current_code = analogRead(A0);
            volt = ((double)current_code) * 5.0 / 1023.0;

            // Determino il picco (a riposo)
            if(volt > volt_max)
                volt_max = volt;
        }

        // Dal picco (a riposo) determino il valore RMS (a riposo)
        volt_rms = 0.707 * volt_max;

        // Media pesata fra i risultati delle differenti fasi di calibrazione
        volt_cal_avg = volt_rms + volt_cal_avg;

        Serial.print((String) "CALIBRAZIONE " + i + ": ");
        Serial.print(volt_rms, 8);
        Serial.print(" Volt \n");

        // valuto la moving average solo a partire dalla seconda calibrazione
        if(i != 1)
            volt_cal_avg = volt_cal_avg / 2;

        Serial.print("Media pesata  : ");
        Serial.print(volt_cal_avg, 8);
        Serial.print(" Volt\n\n");

        delay(1000);
    }

    Serial.print("----------------------------------------\n\n");
}

void loop() {

    // Effettuo le vere e proprie misurazioni

    // Azzero il picco misurato
    volt_max = 0.0;

    for(int i=0; i<campioni; i++) {

        current_code = analogRead(A0);
        volt = ((double)current_code) * 5.0 / 1023.0;

        // Determino il picco di tensione
        if(volt>volt_max)
            volt_max = volt;
    }

    // Dal picco determino il valore RMS e sottraggo il valore rms a riposo
    volt_rms = (volt_max * 0.707) - volt_cal_avg;

    // Calcolo la potenza (30 costante relativa al sensore SCT013-030)
    power = 30*volt_rms*tensione_rif;

    // Valore assoluto della potenza
    if(power < 0)
        power = -power;

    Serial.print(volt_rms, 8);
    Serial.print(" deltaV | ");

    if (power>watt_min) {
        // la potenza misurata è superiore alla soglia minima di rumore
        Serial.print(power, 8);
        Serial.print(" WATT\n");
    } else {
        // è inferiore
        Serial.print("minore di ");
        Serial.print(watt_min, DEC);
        Serial.print(" WATT (");
        Serial.print(power, 8);
        Serial.print(" WATT)\n");
    }

    Pacchetto p;
    p.id = sensor_id;
    p.power = round(power);

    Udp.beginPacket(server_ip, port);
    Udp.write((byte *)&p, sizeof(p)); // da risolvere
    Udp.endPacket();
    delay(10000);

}
