#include <mqtt/client.h>

#define MQTT_ADDRESS "tcp://localhost:1883"
#define CLIENTID        "marcoid"
#define MQTT_PUBLISH_TOPIC      "MQTTCClientPubTopic"
#define MQTT_SUBSCRIBE_TOPIC    "MQTTCClientSubTopic"


int main(int argc, char *argv[])
{
    mqtt::client cliente(MQTT_ADDRESS, CLIENTID);
    mqtt::client::connect_options conn_ops;
}
