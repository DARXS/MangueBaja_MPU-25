# MangueBaja_MPU-25

Este repositório contém o código-fonte para o sistema de telemetria do veículo da equipe MangueBaja, compreendendo um Módulo de Processamento (MPU) transmissor e um módulo receptor.

## Visão Geral

O projeto utiliza um ESP32 como Módulo de Processamento principal (MPU) no veículo. Ele é responsável por:
* Coletar dados de diversos sensores através de uma rede CAN.
* Obter informações de geolocalização de um módulo GPS.
* Transmitir um pacote de dados consolidado via rádio LoRa.
* Fornecer uma interface de depuração em tempo real via Bluetooth Low Energy (BLE).

O código do receptor, que também utiliza um ESP32, é responsável por receber os dados de LoRa, exibi-los e registrá-los em um cartão SD para análise posterior.

## Estrutura do Projeto

O projeto está organizado da seguinte forma:
```
MangueBaja_MPU-25/
├── include/                # Arquivos de cabeçalho globais
│   ├── can_defs.h          # Definições de IDs da rede CAN
│   ├── hard_defs.h         # Definições de pinos para o transmissor (MPU)
│   ├── hard_defs_rec.h     # Definições de pinos para o receptor
│   ├── MPU_defs.h          # Definições da máquina de estados e do MPU
│   └── packets.h           # Estruturas dos pacotes de dados (LoRa e BLE)
├── lib/                    # Bibliotecas customizadas do projeto
│   ├── BLE/                # Lógica para comunicação Bluetooth Low Energy
│   ├── CAN/                # Lógica para comunicação via CAN
│   ├── LORA/               # Lógica para comunicação via LoRa
│   └── StateMachine/       # Implementação da máquina de estados
├── src/
│   └── main.cpp            # Ponto de entrada do programa do MPU transmissor
├── .gitignore              # Arquivos e pastas a serem ignorados pelo Git
├── platformio.ini          # Arquivo de configuração do PlatformIO com dependências
├── README.md               # Este arquivo de documentação
└── receiver.cpp            # Código-fonte para o módulo receptor
```
## Funcionalidades

### Transmissor (MPU)

* **Gerenciamento por Máquina de Estados**: Controla as operações de leitura de GPS e transmissão de rádio usando um buffer circular e tickers para agendamento de tarefas.
* **Comunicação CAN**: Ouve a rede CAN para receber dados de múltiplos sensores, incluindo:
    * Acelerômetro e Giroscópio (IMU)
    * RPM do motor
    * Velocidade
    * Temperaturas (Motor e CVT)
    * Tensão da bateria (SOC)
    * Dados de status de outros módulos (SCU, TCU, MMI)
* **Comunicação LoRa**: Envia a estrutura de dados `radio_packet_t` para a estação base.
* **Coleta de Dados GPS**: Utiliza a biblioteca TinyGPSPlus para ler e processar dados de um módulo GPS conectado via serial.
* **Depuração via BLE**: Inicia um servidor BLE que, ao receber o comando "MB", envia um pacote JSON com o status atual dos sistemas do veículo.

### Receptor

* **Recepção LoRa**: Configurado para receber pacotes de dados da estrutura `radio_packet_t` do veículo.
* **Armazenamento de Dados**: Salva os dados recebidos em um arquivo CSV em um cartão SD. Um novo arquivo é criado a cada inicialização.
* **Saída Serial**: Exibe os dados recebidos no monitor serial, tanto em formato de texto simples quanto em JSON.

## Estrutura dos Pacotes de Dados

Dois `structs` principais são usados para organizar os dados, definidos em `include/packets.h`:

* `radio_packet_t`: Contém todos os dados principais de telemetria enviados via LoRa, como dados do IMU, RPM, velocidade, temperaturas, coordenadas GPS e timestamp.
* `bluetooth`: Contém dados de status dos subsistemas para depuração via BLE, como o status de inicialização do LoRa, conexão com a internet, status do cartão SD e estados de atuadores.

## Software e Dependências

O projeto é construído usando PlatformIO e a framework Arduino. As dependências são gerenciadas pelo `platformio.ini` e incluem:

* `https://github.com/KrisKasprzak/EBYTE.git`
* `https://github.com/joaogabrielsantosD/CANmsg.git`
* `rlogiacco/CircularBuffer@^1.3.3`
* `tinyu-zhao/TinyGPSPlus-ESP32@^0.0.2`
* `bblanchon/ArduinoJson@^7.2.0`
