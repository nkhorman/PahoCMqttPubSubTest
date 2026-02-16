
#include <map>
#include <sstream>
#include <iostream>

#include "MqttSubPub.hpp"

MqttSubPub &MqttSubPub::Host(std::string const &url)
{
	if(hostUrl_ != url)
	{
		hostUrl_ = url;
		if(initialized_)
			Shutdown();
		Startup();
	}

	initialized_= (lastResult_ == MQTTCLIENT_SUCCESS);

	return *this;
}

void trace_callback(enum MQTTCLIENT_TRACE_LEVELS level, char* message)
{
	fprintf(stderr, "Trace : %d, %s\n", level, message);
}

MqttSubPub & MqttSubPub::S1()
{
	if(logLevel_)
	{
		MQTTClient_setTraceCallback(trace_callback);
		MQTTClient_setTraceLevel((enum MQTTCLIENT_TRACE_LEVELS)logLevel_);
	}

	if(lastResult_ == MQTTCLIENT_SUCCESS)
	{
		stage("Startup(s1)::create");
		lastResult_ = MQTTClient_create(
			&client
			, hostUrl_.c_str()
			, clientId_.c_str()
			, MQTTCLIENT_PERSISTENCE_NONE
			, NULL
			);
		stageLastError();
	}
	return *this;
}

MqttSubPub & MqttSubPub::S2()
{
	if(lastResult_ == MQTTCLIENT_SUCCESS)
	{
		stage("Startup(s2)::setCallbacks");
		lastResult_ = MQTTClient_setCallbacks(client, this
			, MsgConnectionLost, MsgArrived, MsgDelivered
		);
		stageLastError();
	}
	return *this;
}

int SSL_err_handler(const char *str, size_t len, void *u)
{
	printf("SSL ERROR : '%s'\n", str);
	return 0;
}

MqttSubPub & MqttSubPub::S3()
{
	if(lastResult_ == MQTTCLIENT_SUCCESS)
	{
		stage("Startup(s3)::connect");
		conn_opts = MQTTClient_connectOptions_initializer;
		conn_opts.keepAliveInterval = 20;
		conn_opts.cleansession = 1;
		conn_opts.username = userName_.c_str();
		conn_opts.password = userPass_.c_str();

		if(isSsl_)
		{
			conn_opts.ssl = & ssl_opts;
			ssl_opts = MQTTClient_SSLOptions_initializer;
			// ssl_opts.trustStore = sslServerChainPem.c_str(); //"/home/neal/src/nest/src/mqtt/neal/pubsub/chain.pem";
			// ssl_opts.privateKey = strClientKeyPem.c_str(); //"/home/neal/src/nest/src/mqtt/neal/pubsub/client.pem";
			ssl_opts.enableServerCertAuth = 0;
			ssl_opts.ssl_error_cb = SSL_err_handler;
			ssl_opts.sslVersion = 3;
		}

		lastResult_ = MQTTClient_connect(client, &conn_opts);
		stageLastError();
	}

	initialized_= (lastResult_ == MQTTCLIENT_SUCCESS);
	return *this;
}

MqttSubPub & MqttSubPub::Startup()
{
	if(!initialized_)
		S1().S2().S3();
	return *this;
}

void MqttSubPub::Shutdown()
{
	if(initialized_)
	{
		if(subscribed_)
			Unsubscribe();
		initialized_ = false;
		stage("Shutdown()::disconnect");
		lastResult_ = MQTTClient_disconnect(client, timeout_);
		stageLastError();
		MQTTClient_destroy(&client);
	}
}

std::string MqttSubPub::LastResult()
{
	std::map<int, std::string> clientError = {
		{ MQTTCLIENT_SUCCESS, "Success" },
		{ MQTTCLIENT_FAILURE, "Failure" },
		{ MQTTCLIENT_DISCONNECTED, "Disconnected" },
		{ MQTTCLIENT_MAX_MESSAGES_INFLIGHT, "Max messages inflight" },
		{ MQTTCLIENT_BAD_UTF8_STRING, "Bad utf8 string" },
		{ MQTTCLIENT_NULL_PARAMETER, "Null parameter" },
		{ MQTTCLIENT_TOPICNAME_TRUNCATED, "Topic name truncated" },
		{ MQTTCLIENT_BAD_STRUCTURE, "Bad Structure" },
		{ MQTTCLIENT_BAD_QOS, "Bad QOS" },
		{ MQTTCLIENT_SSL_NOT_SUPPORTED, "SSL not suported" },
		{ MQTTCLIENT_BAD_MQTT_VERSION, "Bad MQTT version" },
		{ MQTTCLIENT_BAD_PROTOCOL, "Bad protocol" },
		{ MQTTCLIENT_BAD_MQTT_OPTION, "Bad MQTT option" },
		{ MQTTCLIENT_WRONG_MQTT_VERSION, "Wrong MQTT version" },
		{ MQTTCLIENT_0_LEN_WILL_TOPIC, "0 len will topic" },
	};
	auto it = clientError.find(lastResult_);

	return
		stage_ + " - "
		+ (
			it != clientError.end()
			? it->second
			: "Unknown = " + std::to_string(lastResult_)
		);
}

void MqttSubPub::MsgDelivered(void *ctx, MQTTClient_deliveryToken dt)
{
	MqttSubPub *ptr = static_cast<MqttSubPub *>(ctx);
	ptr->deliveredtoken_ = dt;
	std::cout << "Message with token " << dt << " delivered" << std::endl;
}

int MqttSubPub::MsgArrived(void *ctx, char *topic, int topicLen, MQTTClient_message *message)
{
	MqttSubPub *ptr = static_cast<MqttSubPub *>(ctx);

	if(ptr && ptr->fnSubCallback_)
		ptr->fnSubCallback_(topic, std::string((char const *)message->payload, message->payloadlen));
	else
		std::cout << "Message arrived" << std::endl
			<< "     topic: " << topic << std::endl
			<< "   message: " << std::string((char const *)message->payload, message->payloadlen) << std::endl
			;
	MQTTClient_freeMessage(&message);
	MQTTClient_free(topic);
	return 1;
}

void MqttSubPub::MsgConnectionLost(void *ctx, char *cause)
{
	MqttSubPub *ptr = static_cast<MqttSubPub *>(ctx);

	std::cout << std::endl << "Connection lost" << std::endl;
	if(cause)
		std::cout << "     cause: " << cause << std::endl;
	if(ptr)
		ptr->connLost_ = true;
}

bool MqttSubPub::Subscribe(std::function<void(std::string const &topic, std::string const &msg)> fn, int qos)
{
	if(subscribed_)
		Unsubscribe();

	if(initialized_ && !subscribed_)
	{
		stage("Subscribe()::subscribe");
		lastResult_ = MQTTClient_subscribe(client, topic_.c_str(), qos);
		stageLastError();
		subscribed_ = (lastResult_ == MQTTCLIENT_SUCCESS);
		if(subscribed_)
			fnSubCallback_ = fn;
	}

	return subscribed_;
}

bool MqttSubPub::Unsubscribe()
{
	if(initialized_ && subscribed_)
	{
		stage("Unsubscribe()::unsubscribe");
		lastResult_ = MQTTClient_unsubscribe(client, topic_.c_str());
		stageLastError();
		subscribed_ = !(lastResult_ == MQTTCLIENT_SUCCESS);
	}

	return !subscribed_;
}

bool MqttSubPub::Publish(std::string const &value, int qos, int retain)
{
	lastResult_ = MQTTCLIENT_FAILURE;
	if(initialized_)
	{
		pubmsg.payload = (char *)value.c_str();
		pubmsg.payloadlen = value.length();
		pubmsg.qos = (qos >=0 && qos < 3 ? qos : 0);
		pubmsg.retained = retain;
		stage_ = "Publish()::publish";

		lastResult_ = MQTTClient_publishMessage(client, topic_.c_str(), &pubmsg, &token);
		stageLastError();
		if (lastResult_ == MQTTCLIENT_SUCCESS)
		{
			std::cout << "Message publish token " << token << std::endl;
			stage_ = "Publish()::waitForCompletion";
			lastResult_ = MQTTClient_waitForCompletion(client, token, timeout_);
			stageLastError();
		}
	}

	return lastResult_ == MQTTCLIENT_SUCCESS;
}
