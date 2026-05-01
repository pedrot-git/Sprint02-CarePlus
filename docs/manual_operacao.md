# Manual de Operacao

## 1. Preparar FIWARE

Suba a VM e confirme que os containers estao ativos. A collection Postman possui health checks para:

- Orion: `http://35.198.7.130:1026/version`
- IoT Agent: `http://35.198.7.130:4041/version`
- STH-Comet: `http://35.198.7.130:8666/version`

## 2. Provisionar IoT Agent

No Postman, importe a collection e execute:

1. `0. Diagnostico da VM`
2. `2. Setup IoT Agent + Device`

Se algum POST retornar `409`, significa que o recurso provavelmente ja existe. Nesse caso, consulte os itens de listagem antes de apagar algo.

## 3. Rodar Wokwi

Abra o projeto Wokwi com os arquivos da pasta `wokwi/`.

Confira no `sketch.ino`:

```cpp
const char* mqttServer = "35.198.7.130";
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

