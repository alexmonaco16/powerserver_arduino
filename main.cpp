#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

// identificativo univoco del sensore
#define sensor_id 0;

// Numero di campioni fra cui determinare il picco della tensione alternata
#define campioni 30
// Soglia di potenza minima misurabile (in Watt)
#define watt_minimi 25
// Tensione di riferimento (220V in Italia)
#define tensione_rif 220

// AHServer
// indirizzo IP del server = 93.48.49.218
IPAddress server_ip(93, 48, 49, 218);
// porto del server = 16000
int port = 16000;

// definizione socket UDP
EthernetUDP Udp;

// MAC address del sensore IoT
byte mac[] = {
        0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

// indirizzo IP (locale) del sensore IoT
IPAddress ip(192, 168, 0, 177);


// variabili del progetto di CSD
int current_code;
double volt;
double volt_max;
double volt_rms;
double volt_cal_avg = 0.0;
double power;
int watt_min = watt_minimi;


// struct che verrà inviata nel pacchetto
typedef struct {
    uint16_t id;
    uint16_t power;
} Pacchetto;


void setup() {
    // Inizializzazione interfaccia Ethernet
    Ethernet.init(5);

    // DEBUG: apertura comunicazione seriale
    Serial.begin(9600);
    while (!Serial) {
        ;
    }

    // DEBUG
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

    // Inizio comunicazione UDP sul porto 16000
    Udp.begin(16000);

    // DEBUG
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

        // DEBUG
        Serial.print((String) "CALIBRAZIONE " + i + ": ");
        Serial.print(volt_rms, 8);
        Serial.print(" Volt \n");

        // valuto la moving average solo a partire dalla seconda calibrazione
        if(i != 1)
            volt_cal_avg = volt_cal_avg / 2;

        // DEBUG
        Serial.print("Media pesata  : ");
        Serial.print(volt_cal_avg, 8);
        Serial.print(" Volt\n\n");

        delay(1000);
    }

    // DEBUG
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

    // DEBUG
    Serial.print(volt_rms, 8);
    Serial.print(" deltaV | ");

    if (power>watt_min) {
        // la potenza misurata è superiore alla soglia minima di rumore
        // DEBUG
        Serial.print(power, 8);
        Serial.print(" WATT\n");
    } else {
        // la potenza misurata è inferiore alla soglia minima di rumore
        // pertanto, la azzero
        power = 0;
        // DEBUG
        Serial.print("minore di ");
        Serial.print(watt_min, DEC);
        Serial.print(" WATT (");
        Serial.print(power, 8);
        Serial.print(" WATT)\n");
    }

    // incapsulo l'id del sensore e la misurazione di consumo in un pacchetto
    Pacchetto p;
    p.id = sensor_id;
    p.power = round(power);

    // invio via rete il pacchetto al server remoto
    Udp.beginPacket(server_ip, port);
    Udp.write((byte *)&p, sizeof(p)); // da risolvere
    Udp.endPacket();
    delay(10000);

}
