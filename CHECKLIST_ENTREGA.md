# Checklist da Entrega - Sprint 02

## Arquivos

- [x] Codigo-fonte do ESP32 em `wokwi/sketch.ino`
- [x] Diagrama Wokwi em `wokwi/diagram.json`
- [x] Bibliotecas Wokwi em `wokwi/libraries.txt`
- [x] Collection Postman FIWARE/MQTT em `postman/`
- [x] Dashboard Colab em `dashboard_colab/`
- [x] README com arquitetura, configuracao e execucao
- [x] Manual de operacao
- [x] `INTEGRANTES.txt` preenchido com nomes e RM
- [x] Link publico do Wokwi conferido
- [x] Link publico do video de 3 min conferido
- [x] Repositorio GitHub publico informado

## Validacao tecnica

- [ ] VM FIWARE ligada
- [ ] Orion responde em `http://00.000.0.000:1026/version`
- [ ] IoT Agent responde em `http://00.000.0.000:4041/version`
- [ ] STH-Comet responde em `http://00.000.0.000:8666/version`
- [ ] MQTT acessivel em `00.000.0.000:1883`
- [ ] IoT Service criado com `apikey=TEF`
- [ ] Device `token001` criado
- [ ] Wokwi publica em `/TEF/token001/attrs`
- [ ] Orion mostra `CarePlusToken:token001`
- [ ] Subscription para STH-Comet criada
- [ ] Historico aparece no STH-Comet
- [ ] Dashboard Colab carrega dados e gera graficos

## Observacao importante

Antes da apresentacao, substitua `00.000.0.000` pelo IP publico da VM, ligue a VM/suba os containers e rode os health checks da collection.

