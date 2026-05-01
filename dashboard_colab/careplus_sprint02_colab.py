# CarePlus Sprint 02 - Dashboard FIWARE para Google Colab
# Cole este codigo em uma celula do Colab e execute.

import requests
import pandas as pd
import matplotlib.pyplot as plt
from datetime import datetime

FIWARE_URL = "35.198.7.130"  # troque se sua VM mudar
SERVICE = "openiot"
SERVICE_PATH = "/"
ENTITY_ID = "CarePlusToken:token001"
ENTITY_TYPE = "CarePlusToken"

HEADERS = {
    "fiware-service": SERVICE,
    "fiware-servicepath": SERVICE_PATH,
}


def get_json(url):
    response = requests.get(url, headers=HEADERS, timeout=20)
    print(response.status_code, url)
    response.raise_for_status()
    return response.json()


def get_orion_entity():
    url = f"http://{FIWARE_URL}:1026/v2/entities/{ENTITY_ID}?options=keyValues"
    return get_json(url)


def get_sth_attribute(attribute, last_n=100):
    url = (
        f"http://{FIWARE_URL}:8666/STH/v1/contextEntities/type/{ENTITY_TYPE}"
        f"/id/{ENTITY_ID}/attributes/{attribute}?lastN={last_n}"
    )
    data = get_json(url)
    values = data.get("contextResponses", [])[0].get("contextElement", {}).get("attributes", [])[0].get("values", [])
    rows = []
    for item in values:
        recv_time = item.get("recvTime") or item.get("recvTimeTs")
        value = item.get("attrValue")
        rows.append({"recvTime": recv_time, attribute: pd.to_numeric(value, errors="ignore")})
    return pd.DataFrame(rows)

print("Estado atual no Orion:")
entity = get_orion_entity()
display(pd.DataFrame([entity]).T.rename(columns={0: "valor"}))

attributes = ["steps", "pendingSteps", "tokenValue", "totalPoints", "batteryLevel", "rssi", "accelX", "accelY", "accelZ"]
frames = []
for attr in attributes:
    try:
        df_attr = get_sth_attribute(attr, last_n=100)
        if not df_attr.empty:
            frames.append(df_attr)
    except Exception as exc:
        print(f"Nao foi possivel ler {attr}: {exc}")

if frames:
    history = frames[0]
    for frame in frames[1:]:
        history = pd.merge(history, frame, on="recvTime", how="outer")

    history["recvTime"] = pd.to_datetime(history["recvTime"], errors="coerce")
    history = history.sort_values("recvTime").reset_index(drop=True)
    display(history.tail(20))

    chart_cols = [col for col in ["steps", "pendingSteps", "totalPoints", "batteryLevel"] if col in history.columns]
    history.plot(x="recvTime", y=chart_cols, figsize=(12, 5), marker="o")
    plt.title("CarePlus - passos, pontos e bateria")
    plt.xlabel("Horario")
    plt.ylabel("Valor")
    plt.grid(True)
    plt.show()

    accel_cols = [col for col in ["accelX", "accelY", "accelZ"] if col in history.columns]
    if accel_cols:
        history.plot(x="recvTime", y=accel_cols, figsize=(12, 5), marker=".")
        plt.title("CarePlus - acelerometro MPU6050")
        plt.xlabel("Horario")
        plt.ylabel("m/s²")
        plt.grid(True)
        plt.show()
else:
    print("Ainda nao ha historico no STH-Comet. Crie a subscription no Postman e rode a simulacao no Wokwi por alguns segundos.")
