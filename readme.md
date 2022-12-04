# Powerserver Arduino
## Applicativo embedded per sensore di consumi elettrici

# Obiettivi

Si vuole estendere il prototipo inziale, sviluppato su scheda STM32 per l'esame di Computer Systems Design: anzichè mostrare i dati sul consumo elettrico misurato sul display LCD a caratteri, si vogliono trasmettere le misurazioni di consumo elettrico mediante la rete a un server remoto.

# Prototipo iniziale su scheda STM32
Il prototipo iniziale è un misuratore di consumi elettrici non invasivo.
Il progetto è stato sviluppato su scheda STMicroelectronics STM32F303 ed è basato sull’utilizzo di un sensore YHDC SCT013, che misura la corrente attraverante un cavo elettrico sulla base della legge fisica di Faraday-Neumann-Lentz. Abbiamo inoltre utilizzato un display a cristalli liquidi 16x2 HD44780 per la stampa a video del valore corrente di potenza assorbita.

# Porting da STM32 ad Arduino Nano
Volendo modificare il prototipo esistente, rimuovendo il display LCD e aggiungendo le funzionalità di rete, si è optato per il passaggio a piattaforma Arduino, a causa della maggior disponibilità di librerie e componenti aggiuntivi per la comunicazione tramite rete (in particolare Ethernet, tecnologia da noi scelta per la comunicazione su rete).
Si è effettuato un refactoring del codice, già sviluppato per il microcontrollore STM, che è stato poi adattato per funzionare su una scheda Arduino Nano Every.
Sono state eliminate le sezioni superflue per i nostri nuovi scopi, ad esempio tutta la parte dedicata alla gestione del display.

# Librerie utilizzate

### Ethernet library for Arduino
Per fornire connettività di rete al sensore IoT si è utilizzata la libreria Ethernet.h ufficiale per Arduino.

Questa libreria è stata progettata per funzionare con Arduino Ethernet Shield, Arduino Ethernet Shield 2, Leonardo Ethernet e qualsiasi altro dispositivo basato su chip W5100/W5200/W5500. La libreria consente a una scheda Arduino di connettersi a Internet. La scheda può fungere da server che accetta le connessioni in entrata o da client che effettua quelle in uscita. La libreria supporta fino a otto (le schede W5100 e quelle con <= 2 kB di SRAM sono limitate a quattro) connessioni simultanee (in entrata, in uscita o una combinazione).

La scheda Arduino comunica con lo shield utilizzando il bus SPI, pertanto viene utilizzata anche la libreria SPI.h per tale comunicazione seriale.

Si utilizza EthernetUdp per la comunicazione basata su User Datagram Protocol.
