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
- [ ] Link publico do video de 3 min conferido
- [x] Repositorio GitHub publico informado

## Validacao tecnica

- [ ] VM FIWARE ligada
- [ ] Orion responde em `http://35.198.7.130:1026/version`
- [ ] IoT Agent responde em `http://35.198.7.130:4041/version`
- [ ] STH-Comet responde em `http://35.198.7.130:8666/version`
- [ ] MQTT acessivel em `35.198.7.130:1883`
- [ ] IoT Service criado com `apikey=TEF`
- [ ] Device `token001` criado
- [ ] Wokwi publica em `/TEF/token001/attrs`
- [ ] Orion mostra `CarePlusToken:token001`
- [ ] Subscription para STH-Comet criada
- [ ] Historico aparece no STH-Comet
- [ ] Dashboard Colab carrega dados e gera graficos

## Observacao importante

Na ultima verificacao local, as portas da VM `35.198.7.130` nao responderam. Antes da apresentacao, ligue a VM/suba os containers e rode os health checks da collection.
