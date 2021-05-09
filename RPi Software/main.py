import time
from src.ppk2_api import PPK2_API
import paho.mqtt.client as mqtt

url = "192.168.X.X"            # RPI MQTT Mosquitto
topic = "/station/1/hr"         # HR Topic

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(url, 1883, 60)

ppk2_test = PPK2_API("/dev/ttyACM0")
ppk2_test.get_modifiers()
ppk2_test.use_ampere_meter()
ppk2_test.toggle_DUT_power("ON")

ppk2_test.start_measuring()

rc = 0

while rc == 0:
    rc = client.loop()
    read_data = ppk2_test.get_data()
    if read_data != b'':
        samples = ppk2_test.get_samples(read_data)
        print(f"Average of {len(samples)} samples is: {sum(samples)/len(samples)}uA")
        listToStr = ' '.join([str(elem) for elem in samples])
        client.publish(topic,listToStr)
        time.sleep(0.01)