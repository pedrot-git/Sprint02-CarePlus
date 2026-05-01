# Arquitetura da Solucao

## Camadas

### Edge

O ESP32 simulado no Wokwi representa o dispositivo de borda da solucao. Ele coleta sinais do MPU6050, contabiliza passos, exibe feedback no OLED e permite validacao por botao no totem.

### MQTT

O ESP32 se conecta ao broker Mosquitto na porta `1883` e publica mensagens UltraLight no topico `/TEF/token001/attrs`.

### Back-end IoT/FIWARE

O IoT Agent MQTT recebe a telemetria, interpreta os object IDs UltraLight e cria/atualiza a entidade `CarePlusToken:token001` no Orion Context Broker.

### Persistencia

O Orion envia notificacoes para o STH-Comet por meio de uma subscription. O STH-Comet armazena o historico por atributo.

### Aplicacao/Dashboard

O Postman e usado para provisionamento, diagnostico e consulta. O Google Colab consulta Orion/STH-Comet e gera tabelas e graficos de acompanhamento.

## Diagrama

![Arquitetura CarePlus Sprint 02](arquitetura-careplus-v2.svg)

```mermaid
flowchart TD
    U["Beneficiario CarePlus"] --> W["Wearable/Token ESP32"]
    W --> S["Sensores: MPU6050, botao, OLED e LEDs"]
    W --> M["MQTT Mosquitto"]
    M --> I["IoT Agent MQTT UltraLight"]
    I --> O["Orion Context Broker"]
    O --> H["STH-Comet"]
    O --> P["Postman"]
    H --> C["Dashboard Google Colab"]
```

## Dados da entidade

Entidade principal: `CarePlusToken:token001`

Tipo: `CarePlusToken`

Atributos principais: `steps`, `pendingSteps`, `tokenValue`, `totalPoints`, `batteryLevel`, `activityLevel`, `accelX`, `accelY`, `accelZ`.

