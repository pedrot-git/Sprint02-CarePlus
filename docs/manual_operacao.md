# Manual de Operacao

## 1. Preparar FIWARE

Use uma VM Linux, preferencialmente Ubuntu Server LTS, com Docker e Docker Compose instalados. Libere no firewall/security group as portas:

- `1026/TCP` para Orion Context Broker
- `4041/TCP` para IoT Agent MQTT
- `8666/TCP` para STH-Comet
- `1883/TCP` para MQTT Broker Mosquitto

Instale os requisitos:

```bash
sudo apt update
sudo apt install docker.io docker-compose -y
sudo systemctl enable docker
sudo systemctl start docker
```

Suba a stack FIWARE usada como base da disciplina:

```bash
git clone https://github.com/fabiocabrini/fiware
cd fiware
sudo docker-compose up -d
sudo docker ps
```

A collection Postman possui health checks para:

- Orion: `http://00.000.0.000:1026/version`
- IoT Agent: `http://00.000.0.000:4041/version`
- STH-Comet: `http://00.000.0.000:8666/version`

Substitua `00.000.0.000` pelo IP publico da VM FIWARE antes de rodar a collection, o Wokwi ou o dashboard Colab.

## 2. Provisionar IoT Agent

No Postman, importe a collection e execute:

1. `0. Diagnostico da VM`
2. `2. Setup IoT Agent + Device`

Se algum POST retornar `409`, significa que o recurso provavelmente ja existe. Nesse caso, consulte os itens de listagem antes de apagar algo.

## 3. Rodar Wokwi

Abra o projeto Wokwi com os arquivos da pasta `wokwi/`.

Confira no `sketch.ino`:

```cpp
const char* mqttServer = "00.000.0.000";
const int mqttPort = 1883;
const char* mqttTopic = "/TEF/token001/attrs";
```

Ao iniciar, o Serial Monitor deve mostrar conexao Wi-Fi, conexao MQTT e publicacao de payload UltraLight.

## 4. Consultar estado atual

No Postman, execute:

- `4. Consultas Orion - Estado Atual/Get Entity - keyValues`

O retorno esperado e a entidade `CarePlusToken:token001` com os atributos de passos, pontos, bateria, RSSI e acelerometro.

## 5. Persistir historico

Execute:

- `5. STH-Comet - Subscription/Create Subscription to STH-Comet`

Depois rode o Wokwi por mais tempo e consulte a pasta:

- `6. STH-Comet - Historico por atributo`

## 6. Dashboard Colab

Abra o Google Colab, cole o conteudo de `dashboard_colab/careplus_sprint02_colab.py` e execute. O notebook consulta o estado atual no Orion e o historico no STH-Comet.

## 7. Solucao de problemas

Se o Orion retornar `404` para a entidade:

- Confirme se o device `token001` existe no IoT Agent.
- Confirme se o topico MQTT e exatamente `/TEF/token001/attrs`.
- Confirme se o payload usa os object IDs `s`, `p`, `st`, `ps`, `v`, `tp`, `b`, `r`, `al`, `ax`, `ay`, `az`.
- Confirme se os headers `fiware-service=openiot` e `fiware-servicepath=/` estao no Postman.

Se a VM nao responder:

- Verifique se a VM esta ligada.
- Verifique se os containers Docker estao rodando.
- Verifique firewall/regras de rede para as portas `1026`, `4041`, `8666` e `1883`.

Para encerrar a stack:

```bash
cd fiware
sudo docker-compose down
```

Para reset completo, remova os volumes somente se quiser apagar entidades, subscriptions e historico:

```bash
sudo docker volume rm fiware_mongo-historical-data
sudo docker volume rm fiware_mongo-internal-data
```


