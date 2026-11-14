# Estrutura do Projeto MeshCore

Este guia apresenta a organização do repositório MeshCore e o fluxo usado para
transformar o código C++ em um firmware para uma placa LoRa.

## Visão geral

O MeshCore combina quatro partes principais:

1. Uma biblioteca C++ com o protocolo e o roteamento da rede mesh.
2. Aplicações de firmware para diferentes funções do dispositivo.
3. Adaptações para placas, rádios e microcontroladores diferentes.
4. Ferramentas de compilação, testes e documentação.

A combinação básica é:

```text
núcleo mesh + tipo de firmware + variante da placa = firmware final
```

## Árvore principal

```text
MeshCore/
├── src/
├── examples/
├── variants/
├── boards/
├── platformio.ini
├── arch/
├── lib/
├── test/
├── docs/
├── .github/
├── bin/
└── .pio/
```

## `src/`: núcleo reutilizável

O diretório `src/` contém a infraestrutura comum usada pelos diferentes
firmwares.

| Arquivo ou diretório | Responsabilidade |
| --- | --- |
| `MeshCore.h` | Interfaces básicas de placa, relógio e constantes do protocolo |
| `Packet.h/.cpp` | Estrutura, leitura e serialização dos pacotes |
| `Dispatcher.h/.cpp` | Filas, transmissão, recepção, airtime e interação com o rádio |
| `Mesh.h/.cpp` | Roteamento mesh, flood, rotas diretas e hops |
| `Identity.h/.cpp` | Identidades, chaves Ed25519, assinaturas e segredo compartilhado |
| `Utils.h/.cpp` | Hashes, criptografia, conversões e utilidades |
| `helpers/BaseChatMesh.*` | Contatos, mensagens, canais e funções de chat |
| `helpers/radiolib/` | Integração do núcleo com a RadioLib |
| `helpers/ui/` | Displays, botões, buzzer e vibração |
| `helpers/sensors/` | GPS, bateria e sensores ambientais |
| `helpers/wifi/`, `esp32/`, `nrf52/` | Integrações específicas de plataforma |

A hierarquia conceitual é semelhante a esta:

```text
Radio
  └── Dispatcher
        └── Mesh
              └── BaseChatMesh
                    └── MyMesh do firmware
```

`Dispatcher` sabe enviar e receber bytes pelo rádio. `Mesh` entende o
roteamento. `BaseChatMesh` acrescenta contatos e mensagens. O `MyMesh` de cada
firmware implementa o comportamento específico daquele tipo de dispositivo.

## `examples/`: aplicações e firmwares

Apesar do nome, estes diretórios representam aplicações completas que podem ser
compiladas para uma placa.

| Diretório | Função |
| --- | --- |
| `companion_radio/` | Nó conectado a celular ou computador via BLE, USB, Wi-Fi ou Ethernet |
| `simple_repeater/` | Repetidor de pacotes |
| `simple_room_server/` | Servidor de mensagens/BBS |
| `simple_sensor/` | Nó de sensores e telemetria |
| `simple_secure_chat/` | Chat pelo terminal serial |
| `kiss_modem/` | Modem LoRa usando o protocolo KISS |

Um exemplo normalmente contém:

```text
main.cpp       inicialização e loop principal
MyMesh.h/.cpp  comportamento específico do firmware
UITask.*       interface gráfica, quando existe
```

O `main.cpp` não contém toda a aplicação. Ele cria os objetos e conecta o
núcleo às implementações de hardware e às interfaces externas.

## `variants/`: adaptação para cada placa

Cada subdiretório de `variants/` representa uma placa ou uma família de placas.
Por exemplo:

```text
variants/heltec_v3/
├── platformio.ini
├── target.h
├── target.cpp
└── HeltecV3Board.h
```

Esses arquivos definem:

- Pinos do rádio, display, bateria e botões.
- Modelo do chip LoRa.
- Inicialização da placa.
- Relógio, GPS e sensores disponíveis.
- Implementação de funções específicas da placa.

O restante do MeshCore utiliza interfaces genéricas, como `mesh::Radio` e
`mesh::MainBoard`, sem precisar conhecer todos os detalhes da Heltec, RAK,
LilyGo ou outra placa.

## `platformio.ini`: configuração de compilação

O arquivo `platformio.ini` é o centro do sistema de build. Ele define:

- Plataforma: ESP32, nRF52, RP2040 ou STM32.
- Framework Arduino.
- Dependências externas.
- Flags e macros de compilação.
- Arquivos que entram em cada firmware.
- Ambientes nomeados, como `Heltec_v3_repeater`.

As configurações usam herança:

```text
arduino_base
  └── esp32_base
        └── Heltec_lora32_v3
              └── Heltec_v3_companion_radio_ble
```

Por exemplo, um ambiente pode acrescentar:

```ini
-D ENABLE_WIFI_INTERFACE
-D DISPLAY_CLASS=SSD1306Display
-D USE_SX1262
```

O `build_src_filter` seleciona os arquivos que serão compilados. Portanto, um
comando para um firmware específico não compila automaticamente todos os
exemplos do repositório.

## `boards/`: definições do PlatformIO

Os arquivos JSON descrevem placas para o PlatformIO, incluindo:

- Microcontrolador.
- Memória flash.
- Frequência.
- Protocolo de upload.
- Layout de memória.
- Bootloader.

Os arquivos `.ld` são scripts do linker, usados principalmente em placas nRF52
com diferentes bootloaders e layouts de memória.

## `arch/`: suporte específico de arquitetura

Este diretório contém código e bibliotecas necessários para plataformas
específicas.

```text
arch/esp32/   suporte local a OTA
arch/stm32/   LittleFS e scripts para gerar HEX
```

Esse código não representa a lógica central da rede mesh. Ele adapta o projeto
às particularidades de uma arquitetura.

## `lib/`: bibliotecas incluídas no repositório

Exemplos:

- `lib/ed25519/`: implementação criptográfica usada pelas identidades.
- `lib/nrf52/`: headers da API do SoftDevice nRF52.

Outras dependências, como RadioLib, Crypto e bibliotecas de sensores, são
baixadas pelo PlatformIO conforme definido em `platformio.ini` e `library.json`.

## `test/`: testes unitários

Os testes são executados no computador, sem uma placa física. O ambiente
`native` usa GoogleTest e mocks para testar partes como serialização, tabelas,
roteamento e manipulação de texto.

```bash
pio test --environment native --verbose
```

## `docs/`: documentação

Os documentos mais importantes para entender os protocolos são:

| Documento | Conteúdo |
| --- | --- |
| `companion_protocol.md` | Protocolo entre firmware Companion e aplicativos |
| `packet_format.md` | Estrutura binária dos pacotes LoRa |
| `payloads.md` | Conteúdo dos diferentes tipos de payload |
| `cli_commands.md` | Comandos de configuração e administração |
| `kiss_modem_protocol.md` | Protocolo do modem KISS |
| `nrf52_power_management.md` | Gerenciamento de energia em nRF52 |
| `heltec_v3_companion_diagnostics.md` | Diagnósticos do Companion Heltec V3 |
| `estrutura-do-projeto.md` | Este guia |

Para implementar um cliente em Java, C# ou outra linguagem, comece por
`companion_protocol.md` e `packet_format.md`.

## `bin/` e scripts de build

O projeto possui scripts para gerar os formatos de firmware usados por cada
família de placas:

- `build.sh`: lista e compila ambientes do PlatformIO.
- `merge-bin.py`: combina imagens ESP32.
- `create-uf2.py`: gera imagens UF2 quando necessário.
- `bin/uf2conv/`: conversão de HEX para UF2.

Exemplo de compilação de um ambiente:

```bash
pio run -e Heltec_v3_companion_radio_ble
```

Exemplo usando o script do projeto:

```bash
export FIRMWARE_VERSION=v1.0.0
sh build.sh build-firmware Heltec_v3_companion_radio_ble
```

## `.github/`: automação

Os workflows do GitHub Actions compilam firmwares, executam testes, geram
releases e publicam a documentação.

O workflow de validação de PR compila combinações representativas de ESP32,
nRF52, RP2040, STM32 e outros rádios. Isso reduz o risco de uma alteração no
núcleo quebrar uma plataforma diferente.

## O código Arduino

Arduino não é uma linguagem. Neste projeto:

- A linguagem é C++.
- Arduino é o framework de hardware.
- PlatformIO é o sistema de build.

Por isso aparecem arquivos `.cpp` e não apenas arquivos `.ino`.

O framework fornece APIs como:

```cpp
Serial.begin(115200);
millis();
pinMode(...);
digitalRead(...);
SPI;
Wire;
```

O framework também fornece o modelo tradicional:

```cpp
void setup() {
  // executado uma vez
}

void loop() {
  // executado repetidamente
}
```

O `main()` real é fornecido pelo Arduino. Ele chama `setup()` uma vez e depois
chama `loop()` continuamente.

No MeshCore, o `loop()` funciona como um event loop cooperativo:

```cpp
void loop() {
  the_mesh.loop();
  interface_manager.loop();
  sensors.loop();
  ui_task.loop();
  rtc_clock.tick();
  board.loop();
}
```

Cada componente executa rapidamente e devolve o controle ao próximo. Não há
necessariamente uma thread separada para rádio, sensores e interface.

## Macros de compilação

O projeto usa bastante o pré-processador C++:

```cpp
#ifdef ENABLE_WIFI_INTERFACE
  // este trecho só existe quando a macro foi definida
#endif
```

Uma macro pode ser definida no `platformio.ini`:

```ini
-D ENABLE_WIFI_INTERFACE
-D DISPLAY_CLASS=SSD1306Display
```

Assim, o mesmo código-fonte produz firmwares diferentes. Partes que não são
necessárias sequer são compiladas para determinado ambiente.

## Fluxo de inicialização do Companion Heltec V3

O fluxo simplificado de `examples/companion_radio/main.cpp` é:

```text
setup()
 ├── inicializa Serial
 ├── inicializa a placa
 ├── inicializa display
 ├── inicializa rádio LoRa
 ├── inicializa filesystem
 ├── carrega ou cria identidade criptográfica
 ├── carrega preferências e contatos
 ├── configura BLE, Wi-Fi, USB ou Ethernet
 ├── inicializa sensores
 └── inicializa a interface gráfica
```

Depois, o `loop()` executa continuamente:

```text
processa mesh
processa BLE, Wi-Fi, USB ou Ethernet
processa sensores
processa UI
atualiza relógio
executa gerenciamento de energia
```

A identidade do nó é persistida no filesystem. Se o dispositivo ainda não
possui uma identidade, uma nova chave é gerada e armazenada.

## Papéis dos firmwares

### Companion

É a ponte entre a rede LoRa e um aplicativo. Normalmente oferece BLE, USB,
Wi-Fi ou Ethernet e não repete mensagens como um repeater.

### Repeater

Recebe e retransmite pacotes para ampliar a cobertura da rede. Também oferece
comandos administrativos e estatísticas.

### Room Server

Funciona como um servidor de mensagens compartilhadas na rede mesh.

### Sensor

Lê sensores, produz telemetria e pode emitir alertas.

### KISS Modem

Expõe o rádio como um modem serial para outro software controlar o envio e o
recebimento de pacotes.

## O último PR analisado

Neste checkout, o commit mais recente adicionou telas de diagnóstico para o
Companion Heltec V3.

### `examples/companion_radio/diagnostics/`

Foram criados `DiagnosticData.h` e `DiagnosticData.cpp`. Essa camada coleta:

- Tensão e percentual estimado da bateria.
- Uptime.
- RSSI e SNR.
- Pacotes recebidos e enviados.
- Número de contatos.
- Frequência e potência de transmissão.
- Estado da conexão BLE.

### `ui-new/UITask.cpp`

Foram adicionadas três páginas:

```text
STATUS
RADIO
NETWORK
```

A UI solicita os dados ao `DiagnosticDataSource` e apenas desenha os valores no
display.

### `AbstractUITask.h` e `MultiSerialInterface.h`

Foi adicionado suporte para consultar se o Bluetooth está conectado, separando
os estados:

```text
BLE desligado
BLE ligado, aguardando conexão
BLE conectado a um cliente
```

### `variants/heltec_v3/platformio.ini`

Foi criado o ambiente:

```text
Heltec_v3_companion_radio_ble_custom
```

Ele define a macro `COMPANION_DIAGNOSTICS` e inclui os fontes de diagnóstico.
O ambiente original continua disponível sem as telas novas:

```text
Heltec_v3_companion_radio_ble
```

Portanto, essa alteração não modifica o comportamento do firmware original.

## Conceitos importantes para programadores Java e C#

- Não existe garbage collector.
- A memória é limitada e frequentemente alocada de forma estática.
- O projeto evita alocação dinâmica durante a operação normal.
- `#ifdef` decide funcionalidades em tempo de compilação.
- Muitos objetos são globais porque representam hardware único.
- `uint8_t`, `uint16_t` e `uint32_t` controlam exatamente o tamanho dos dados.
- `memcpy`, buffers fixos e arrays são comuns por causa do protocolo binário.
- O projeto normalmente não usa threads; o `loop()` coordena os componentes.
- `Stream`, `Serial`, `File` e `Print` são abstrações do ecossistema Arduino.

## Ordem recomendada de estudo

1. `variants/heltec_v3/platformio.ini`
2. `variants/heltec_v3/target.h`
3. `variants/heltec_v3/target.cpp`
4. `examples/companion_radio/main.cpp`
5. `examples/companion_radio/MyMesh.h`
6. `src/Dispatcher.h`
7. `src/Mesh.h`
8. `src/Packet.h`
9. `src/Identity.h`
10. `docs/companion_protocol.md`

## Arquivos gerados

O diretório `.pio/` contém resultados intermediários e artefatos gerados pelo
PlatformIO. Ele está no `.gitignore` e não deve ser editado ou versionado.

Da mesma forma, a pasta `out/` é usada pelo script de build para armazenar
firmwares finais e também não deve ser versionada.
