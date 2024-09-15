

#include <iostream>
#include <thread>
#include <chrono>

#include "mosquitto.h"


// mosquitto


void pub(mosquitto* mosq, const std::string& topic, const std::string& payload)
{
	mosquitto_publish(mosq, nullptr, topic.c_str(), payload.size(), payload.c_str(), 0, false);
}

void sub(mosquitto* mosq, const std::string& topic)
{
	auto s = mosquitto_subscribe(mosq, nullptr, topic.c_str(), 0);
}


int main()
{


	int major, minor, revision;

	mosquitto_lib_version(&major, &minor, &revision);
	std::cout << "libmosquitto version: " << major << ", " << minor << ", " << revision << "\n";


	mosquitto_lib_init();

	mosquitto* mosq = mosquitto_new(nullptr, true, nullptr);



	if (mosquitto_connect(mosq, "192.168.88.158", 1883, 60) != MOSQ_ERR_SUCCESS)
	{
		std::cerr << "Error: connecting to MQTT broker failed" << "\n";
	}


	mosquitto_message_callback_set(mosq, [](mosquitto* mosq, void* obj, const mosquitto_message* message) {

		std::string payload = std::string((char*)message->payload, message->payloadlen);

		std::cout << "-" << "\n";
		std::cout << "topic: " << message->topic << "\n";
		std::cout << "payload: " << payload << "\n";
		});


	sub(mosq, "shelly-pivot-1/status/switch:0");
	sub(mosq, "shelly-pivot-1/status/input:0");

	pub(mosq, "shelly-pivot-1/command", "status_update");


	std::thread mosq_thread(mosquitto_loop_start, mosq);


	while (true) {
		
		pub(mosq, "shelly-pivot-1/command/switch:0", "toggle");


		std::this_thread::sleep_for(std::chrono::seconds(4));

		//mosquitto_loop(mosq, 4000, 1);

	}


	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
}



// -----------------------------
// paho-mqttpp3
// 
//#include "mqtt/client.h"
//
//
//const int qos_at_most_once = 0;
//const int qos_at_least_once = 1;
//const int qos_exactly_once = 2;
//
//
//int main()
//{
//    std::cout << "Hello World!" << "\n";
//
//
//    const std::string server_address = "192.168.88.158";
//    const std::string client_id = "test-client-paho";
//
//    mqtt::client cli(server_address, client_id);
//
//    mqtt::connect_options connOpts;
//    connOpts.set_keep_alive_interval(20);
//    connOpts.set_clean_session(true);
//
//    try {
//        // Connect to the client
//
//        cli.connect(connOpts);
//
//        // Publish using a message pointer.
//
//        //auto msg = mqtt::make_message("shelly-pivot-1/command/switch:0", "off");
//        //msg->set_qos(qos_at_least_once);
//        //
//        //cli.publish(msg);
//
//        // Now try with itemized publish.
//
//        //auto msg = mqtt::make_message( , "off");
//
//        std::string topic = "shelly-pivot-1/command/switch:0";
//        std::string payload = "on";
//
//        cli.publish(topic, payload.data(), payload.size());
//        //cli.publish(TOPIC, PAYLOAD2, strlen(PAYLOAD2), 0, false);
//
//        // Disconnect
//
//        cli.disconnect();
//    }
//    catch (const mqtt::exception& exc) {
//        std::cerr << "Error: " << exc.what() << " ["
//            << exc.get_reason_code() << "]" << std::endl;
//        return 1;
//    }
//
//    return 0;
//}
