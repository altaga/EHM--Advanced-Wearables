import time
from src.ppk2_api import PPK2_API
import paho.mqtt.client as mqtt

# URL and Topic for Subscription

url = "192.168.X.X"            # RPI MQTT Mosquitto
topic = "/station/1/hr"         # HR Topic

# Debug Functions

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))

# Define Mqtt Client

client = mqtt.Client()

# Callback Functions only for debug
client.on_connect = on_connect
client.on_message = on_message

# Setup URL, PORT and Keepalive 

client.connect(url, 1883, 60)

# Setup PPK2 API

ppk2_test = PPK2_API("/dev/ttyACM0")
ppk2_test.get_modifiers()
ppk2_test.use_ampere_meter()
ppk2_test.toggle_DUT_power("ON")

# Start PPK

ppk2_test.start_measuring()

rc = 0

while rc == 0:
    rc = client.loop()
    read_data = ppk2_test.get_data()
    if read_data != b'':
        samples = ppk2_test.get_samples(read_data)
        # Reading at least 1024 samples
        print(f"Average of {len(samples)} samples is: {sum(samples)/len(samples)}uA")
        listToStr = ' '.join([str(elem) for elem in samples])
        # Publish the results to NodeRED 
        client.publish(topic,listToStr)
        # Sleep time necessary for properly sampling.
        time.sleep(0.01)