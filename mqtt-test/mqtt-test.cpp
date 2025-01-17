


/*

mqtt topics for shelly device

test device is Shelly Plus 1PM

access by http (web browser), if using the inbuilt access point, the address is http://192.168.33.1


settings -> access point -> enable access point = false (don't do this until successfully adding wifi)
settings -> bluetooth -> enable bluetooth = false
settings -> mqtt -> mqtt prefix = shelly-pivot-1 (for example)
settings -> mqtt -> server = 192.168.88.158 (for example)
settings -> mqtt -> client id = shelly-pivot-1 (for example)
settings -> device name -> device name = shelly-pivot-1 (for example)

mqtt server is actually mqtt broker



subscribe to these mqtt topics
shelly-pivot-1/status/switch:0 (output relay)
shelly-pivot-1/status/input:0 (switch)

send the following mqtt message
shelly-pivot-1/command = status_update

this also works
shelly-pivot-1/command/switch:0 = status_update
shelly-pivot-1/command/switch:0 = on
shelly-pivot-1/command/switch:0 = off

this doesn't work (but is covered by doing "status_update" for the whole device)
shelly-pivot-1/command/input:0 = status_update


*/


#include <iostream>
#include <thread>
#include <chrono>

#include "mosquitto.h"

#include "simdjson.h"

namespace sj = simdjson;


// mosquitto


const std::string mqtt_broker_address = "192.168.88.158";
const int mqtt_broker_port = 1883;


void pub(mosquitto* mosq, const std::string& topic, const std::string& payload)
{
	mosquitto_publish(mosq, nullptr, topic.c_str(), payload.size(), payload.c_str(), 0, false);
}

void sub(mosquitto* mosq, const std::string& topic)
{
	auto s = mosquitto_subscribe(mosq, nullptr, topic.c_str(), 0);
}


void do_subscriptions(mosquitto* mosq)
{
	// subscribe to topics we're interested in
	sub(mosq, "shelly-pivot-1/status/switch:0");
	sub(mosq, "shelly-pivot-1/status/input:0");

	//
	pub(mosq, "shelly-pivot-1/command", "status_update");
}


int main()
{


	int major, minor, revision;

	mosquitto_lib_version(&major, &minor, &revision);
	std::cout << "libmosquitto version: " << major << ", " << minor << ", " << revision << "\n";


	mosquitto_lib_init();

	mosquitto* mosq = mosquitto_new(nullptr, true, nullptr);


	while (true) {
		if (mosquitto_connect(mosq, mqtt_broker_address.c_str(), mqtt_broker_port, 60) == MOSQ_ERR_SUCCESS)
		{
			break;
		}
		std::cerr << "Error: connecting to MQTT broker failed" << "\n";
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}


	mosquitto_connect_callback_set(mosq, [](mosquitto* mosq, void* obj, int rc) {
		std::cout << std::endl;
		std::cout << "mosquitto connected" << std::endl;
		std::cout << std::endl;

		do_subscriptions(mosq);
		}
	);

	mosquitto_disconnect_callback_set(mosq, [](struct mosquitto*, void* obj, int rc) {
		std::cout << std::endl;
		std::cout << "mosquitto disconnected" << std::endl;
		std::cout << std::endl;
		}
	);

	mosquitto_message_callback_set(mosq, [](mosquitto* mosq, void* obj, const mosquitto_message* message) {

		std::string topic = message->topic;
		//std::string payload = std::string(

		const sj::padded_string payload = sj::padded_string((char*)message->payload, message->payloadlen);

		std::cout << "-" << "\n";
		std::cout << "topic: " << message->topic << "\n";
		std::cout << "payload: " << payload << "\n";


		if (topic == "shelly-pivot-1/status/switch:0") {
			sj::ondemand::parser parser;
			sj::ondemand::document p = parser.iterate(payload);

			auto result1 = p.find_field("output");
			if (result1.error() == sj::SUCCESS) {
				std::cout << "output is " << p["output"] << std::endl;
			}
			else {
				std::cout << "output doesn't exist" << std::endl;
			}

			auto result2 = p.find_field("thing");
			if (result2.error() == sj::SUCCESS) {
				std::cout << "thing is " << p["thing"] << std::endl;
			}
			else {
				std::cout << "thing doesn't exist" << std::endl;
			}



			std::cout << std::endl;
		}
		});



	std::thread mosq_thread(mosquitto_loop_start, mosq);


	while (true) {

		pub(mosq, "shelly-pivot-1/command/switch:0", "toggle");


		std::this_thread::sleep_for(std::chrono::seconds(4));

		//mosquitto_loop(mosq, 4000, 1);

	}


	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
}



