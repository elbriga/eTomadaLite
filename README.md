# eTomada Lite

Firmware compacto do projeto **eTomada**, destinado a dispositivos simples de automação com Wi-Fi, normalmente compostos por um relé, LED de status e opcionalmente uma entrada de botão/interruptor.

O eTomada Lite foi projetado para executar o mesmo firmware-base em duas famílias diferentes de microcontroladores:

- **ESP8266 / ESP8285**
- **LN882H**, através do framework **LibreTiny**

A maior parte da aplicação é compartilhada entre as arquiteturas. Diferenças de Wi-Fi, HTTP, mDNS, OTA, armazenamento persistente e detalhes do SDK são isoladas pela camada de plataforma.

## Arquiteturas suportadas

### ESP8266 / ESP8285

Utiliza o core Arduino para ESP8266 através do PlatformIO:

```ini
platform = espressif8266
framework = arduino
```

Todos os targets ESP utilizam propositalmente um layout de flash de **1 MiB**:

```ini
board_build.ldscript = eagle.flash.1m.ld
```

Isso mantém o layout do firmware previsível mesmo em placas cuja flash física seja maior.

Targets atuais:

| Environment   | Board      | Hardware                 |
| ------------- | ---------- | ------------------------ |
| `dev`         | `esp12e`   | Protótipo ESP-12E        |
| `sonoff-mini` | `esp01_1m` | Sonoff Mini R1 / ESP8285 |
| `wemos`       | `d1_mini`  | Wemos D1 Mini            |

O Sonoff Mini R1 utiliza flash em modo `DOUT`.

---

### LN882H

O LN882H é suportado através do **LibreTiny**:

```ini
platform = libretiny
board = generic-ln882h
framework = arduino
```

O target atual é:

| Environment | Board            | Hardware                          |
| ----------- | ---------------- | --------------------------------- |
| `cozylife`  | `generic-ln882h` | Relé baseado em LN882H / CozyLife |

Para essa arquitetura é definido:

```ini
-D ETOMADA_LIBRETINY
-D HW_COZY
```

O firmware utiliza o layout padrão de **2 MiB** do `generic-ln882h`, preservando as regiões nativas utilizadas pelo LibreTiny para APP, OTA e dados persistentes.

## Uma aplicação, duas plataformas

O objetivo do projeto não é manter versões separadas do eTomada Lite para ESP e LN882H.

O código da aplicação permanece essencialmente o mesmo:

```text
main.cpp
   |
   +-- recovery
   |
   +-- app
       |
       +-- config
       +-- wifi
       +-- http
       +-- ota
       +-- mdns
       +-- ntp
       +-- rele
       +-- botao
       +-- mestre
       +-- log
```

As diferenças entre SDKs são concentradas principalmente em:

```text
include/platform.h
```

No ESP8266 são utilizadas as classes:

```cpp
ESP8266WiFi
ESP8266WebServer
ESP8266HTTPClient
ESP8266mDNS
Updater
```

No LibreTiny:

```cpp
WiFi
WebServer
HTTPClient
mDNS
Update
```

O servidor HTTP é exposto para o restante da aplicação através do mesmo tipo:

```cpp
ETomadaWebServer
```

Isso permite que módulos como `http.cpp`, `ota.cpp`, `api.cpp` e `wifi.cpp` sejam compartilhados pelas duas arquiteturas.

## Hardware Profiles

As diferenças entre placas são descritas por um `HardwareProfile`.

Cada profile informa:

```cpp
struct HardwareProfile
{
    const char *modelo;
    const char *board;

    int ledPin;
    bool ledInvertido;

    int relePin;
    int botaoPin;
};
```

Profiles atuais:

```text
hardware/
├── dev.h
├── miniR1.h
├── wemos.h
└── cozy.h
```

A seleção é feita em tempo de compilação:

```cpp
HW_DEV
HW_WEMOS
HW_SONOFF_MINIR1
HW_COZY
```

Isso mantém o código da aplicação independente dos GPIOs específicos de cada dispositivo.

## Compilação

O projeto utiliza PlatformIO.

### ESP-12E de desenvolvimento

```bash
pio run -e dev
```

Upload:

```bash
pio run -e dev -t upload
```

Monitor serial:

```bash
pio device monitor -e dev
```

### Sonoff Mini R1

```bash
pio run -e sonoff-mini
```

### Wemos D1 Mini

```bash
pio run -e wemos
```

### LN882H / CozyLife

```bash
pio run -e cozylife
```

O LN882H é compilado pelo LibreTiny utilizando o target `generic-ln882h`.

## Wi-Fi

Quando existe uma configuração Wi-Fi válida, o eTomada Lite inicia em modo STA e tenta conectar à rede configurada.

O hostname utilizado é o `deviceID` do dispositivo.

Se não houver configuração ou a conexão falhar, o firmware inicia um Access Point:

```text
eTomada-<chip-id>
```

A configuração pode ser enviada pela API:

```text
GET /api/configWifi?ssid=<SSID>&senha=<SENHA>
```

Após salvar a configuração, o firmware desconecta da rede atual e inicia uma nova tentativa de conexão.

## Configuração persistente

A configuração principal contém atualmente:

```cpp
struct Config
{
    uint32_t magic;

    char ssid[32];
    char senha[32];

    char deviceID[32];
    char mestre[32];
};
```

O armazenamento físico é diferente em cada arquitetura.

### ESP8266 / ESP8285

É utilizada EEPROM emulada com 256 bytes.

O espaço é dividido entre configuração da aplicação e controle de recovery:

```text
0        239  Configuração da aplicação
240      255  Recovery
```

### LN882H

No LN882H são reservados dois setores de 4 KiB no final da área de User Data da flash de 2 MiB:

```text
0x1FE000 - 0x1FEFFF   Configuração
0x1FF000 - 0x1FFFFF   Recovery
```

A configuração é armazenada diretamente através da API `Flash` do LibreTiny.

O firmware verifica se a flash física possui exatamente:

```text
2 MiB
```

antes de utilizar esses endereços.

## Recovery por ciclos de energia

O eTomada Lite possui um mecanismo de recovery incorporado diretamente ao firmware.

O objetivo é permitir recuperar dispositivos instalados onde não existe acesso fácil à serial.

O `main.cpp` permanece mínimo:

```cpp
setup()
    recoveryBoot()

    se recovery:
        recoveryInit()
    senão:
        appInit()

loop()
    se recovery:
        recoveryLoop()
    senão:
        recoveryBootTick()
        appLoop()
```

### Ativação

O recovery é ativado após **3 boots consecutivos** antes que o firmware tenha tempo de confirmar um boot normal.

O tempo para considerar um boot válido é:

```text
10 segundos
```

Assim, ligar e desligar rapidamente o dispositivo três vezes força a entrada no recovery.

Em um boot normal, após aproximadamente 10 segundos de funcionamento, o contador é confirmado e a sequência é encerrada.

### Storage do contador

No ESP8266 o contador utiliza a área reservada da EEPROM.

No LN882H é utilizado um pequeno journal no último setor da flash:

```text
0x1FF000 - 0x1FFFFF
```

São registrados eventos:

```text
BOOT
OKOK
```

Dessa forma não é necessário apagar e regravar o setor a cada boot.

## Modo Recovery

Ao entrar em recovery, o firmware:

1. carrega somente a configuração necessária;
2. mantém o relé desligado por segurança;
3. tenta conectar à rede Wi-Fi configurada;
4. inicia um servidor HTTP mínimo;
5. disponibiliza OTA e reboot.

O LED, quando disponível, pisca rapidamente durante o recovery.

### Wi-Fi do recovery

Primeiro é feita uma tentativa de conexão à rede configurada por até:

```text
15 segundos
```

Caso isso falhe, é criado um Access Point:

```text
eTomada-Recovery-<chip-id>
```

Senha:

```text
09876543
```

Isso permite recuperar o dispositivo mesmo quando a rede Wi-Fi configurada deixou de existir.

## OTA

A mesma API OTA é registrada tanto na aplicação normal quanto no modo recovery:

```text
POST /api/ota?tamanho=<bytes>
```

O upload é enviado como `multipart/form-data`.

Exemplo:

```bash
ARQ=firmware.bin

curl \
    -F "firmware=@$ARQ" \
    "http://etomada.local/api/ota?tamanho=$(stat -c%s "$ARQ")"
```

### ESP8266 / ESP8285

O OTA utiliza o formato de firmware nativo do ESP.

Antes de iniciar o update, o firmware verifica se o arquivo cabe no espaço disponível para o novo sketch.

### LN882H

No LibreTiny/LN882H o OTA recebe um arquivo **UF2**.

Exemplo:

```bash
ARQ=firmware.uf2

curl \
    -F "firmware=@$ARQ" \
    "http://etomada.local/api/ota?tamanho=$(stat -c%s "$ARQ")"
```

Como validação inicial, o firmware exige que o tamanho do arquivo seja múltiplo de 512 bytes, correspondente ao tamanho dos blocos UF2.

A escrita é feita pela API `Update` do LibreTiny.

Ao terminar corretamente, a API retorna sucesso e reinicia o dispositivo.

## API HTTP

A aplicação normal disponibiliza atualmente:

```text
GET  /api/getSnapshot
GET  /api/configWifi
GET  /api/configHostname
GET  /api/setRele
GET  /api/reset

POST /api/ota
POST /api/reboot
```

O endpoint:

```text
GET /api/getSnapshot
```

retorna informações como:

```text
versão do firmware
device ID
modelo
board
uptime
estado do relé
MAC
IP
SSID
RSSI
```

No modo recovery também existe:

```text
GET /api/status
```

que informa:

```text
mode
ssid
ip
rssi
uptime
```

## mDNS

Quando conectado à rede normal, o eTomada Lite anuncia:

```text
_etomada._tcp
```

na porta:

```text
80
```

São publicados registros TXT com informações do dispositivo:

```text
device
id
ssid
ip
mac
model
board
fw
api
```

Por exemplo:

```text
device=eTomada
api=Lite
```

Isso permite que controladores encontrem automaticamente dispositivos eTomada na rede.

### Limitação atual do LN882H

O anúncio mDNS funciona através da implementação disponível no LibreTiny.

Entretanto, a descoberta de outros serviços através de:

```cpp
MDNS.queryService()
```

está implementada atualmente apenas para ESP8266.

Por isso, a descoberta automática do **nodo Mestre** ainda não está disponível no LN882H.

Essa é atualmente uma das poucas diferenças funcionais entre as arquiteturas.

## NTP

O eTomada Lite sincroniza o relógio utilizando:

```text
a.ntp.br
pool.ntp.org
```

No ESP8266 é utilizada `configTime()`.

No LN882H é utilizada diretamente a implementação SNTP do lwIP.

## Eventos

O firmware possui uma arquitetura de eventos para comunicação com o controlador eTomada.

Tipos definidos atualmente incluem:

```text
LIGOU
DESLIGOU
TOGGLE
CLICK
DUPCLICK
LONG_PRESS
CHANGED
HORARIO
```

O botão/interruptor local pode gerar eventos que são enviados ao nodo Mestre através da API HTTP interna.

## Log remoto

Além do log serial, o firmware possui uma fila local para envio assíncrono de logs via HTTP.

Cada entrada contém:

```text
timestamp
uptime
level
module
message
```

A fila evita bloquear a aplicação durante a geração dos logs e descarta os registros mais antigos caso fique cheia.

## Estrutura geral

```text
.
├── platformio.ini
├── include/
│   ├── api.h
│   ├── app.h
│   ├── config.h
│   ├── eTomadaLite.h
│   ├── hardwareProfile.h
│   ├── ota.h
│   ├── platform.h
│   ├── recovery.h
│   └── ...
│
├── hardware/
│   ├── dev.h
│   ├── miniR1.h
│   ├── wemos.h
│   └── cozy.h
│
└── src/
    ├── main.cpp
    ├── app.cpp
    ├── config.cpp
    ├── wifi.cpp
    ├── http.cpp
    ├── ota.cpp
    ├── recovery.cpp
    ├── mdns-gs.cpp
    ├── mestre.cpp
    ├── rele.cpp
    ├── botao.cpp
    └── ...
```

## Filosofia do projeto

O eTomada Lite procura manter o firmware pequeno e independente do hardware específico.

A arquitetura segue três princípios principais:

1. **Uma única aplicação para múltiplos microcontroladores**

   ESP8266/ESP8285 e LN882H compartilham a mesma lógica de aplicação.

2. **Hardware isolado em profiles**

   GPIOs e particularidades elétricas ficam fora da lógica principal.

3. **Recovery sempre disponível**

   Mesmo dispositivos instalados sem acesso serial podem retornar a um firmware funcional através de três ciclos de energia e OTA pela rede.

Isso permite utilizar desde dispositivos ESP8266 tradicionais, como o Sonoff Mini R1, até módulos mais novos baseados em LN882H sem manter forks independentes do firmware.
