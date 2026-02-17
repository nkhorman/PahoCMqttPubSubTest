#pragma once

#include <functional>
#include <string>

#include "MQTTClient.h"
#include "StringRand.hpp"

class MqttSubPub
{
public:
	MqttSubPub()
		: clientId_("MqttSubPub")
		, lastResult_(MQTTCLIENT_SUCCESS)
		{
			clientId_ += StringRand::Simple(8);
		};
	virtual ~MqttSubPub() { Shutdown(); }

	MqttSubPub &Connect(std::string const &url);

	inline MqttSubPub &Topic(std::string const &topic) { topic_ = topic; return *this;}
	std::string LastResult();

	bool Publish(std::string const &value, int qos, int retain);
	std::string ClientId() const { return clientId_; }

	bool Subscribe(std::function<void(std::string const &topic, std::string const &msg)> fn, int qos = 0);
	bool Unsubscribe();

	inline void stage(std::string stage)
	{
#ifdef MQTT_STAGE
		stage_ = stage;
		std::cout << "stage: " << stage_ << std::endl;
#endif
	}
	inline void stageLastError()
	{
#ifdef MQTT_STAGE
		std::cout << "stage: " << LastResult() << std::endl;
#endif
	}
	std::string stage() const { return stage_; }

	bool needShutdown() const { return connLost_; }
	bool isError() const { return lastResult_ != MQTTCLIENT_SUCCESS; }

	MqttSubPub &UserName(std::string const &str) { userName_ = str; return *this; }
	MqttSubPub &UserPass(std::string const &str) { userPass_ = str; return *this; }
	MqttSubPub & SslServerChain(std::string const &str) { isSsl_ |= !str.empty(); sslServerChainPem = str; return * this;}
	MqttSubPub & SslClientKey(std::string const &str) { isSsl_ |= !str.empty(); strClientKeyPem = str; return *this; }
	void SslLogLevel(int l) { logLevel_ = l; }


	MqttSubPub &  OnConnect(std::function<void()> fn) { fnOnConnect_ = fn; return *this; }
	MqttSubPub &  OnDisconnect(std::function<void()> fn) { fnOnDisconnect_ = fn; return *this; }

protected:
	void Shutdown();
	MqttSubPub & Startup();
	MqttSubPub &S1();
	MqttSubPub &S2();
	MqttSubPub &S3();

	static void MsgDelivered(void *ctx, MQTTClient_deliveryToken dt);
	static int MsgArrived(void *ctx, char *topicName, int topicLen, MQTTClient_message *message);
	static void MsgConnectionLost(void *ctx, char *cause);

	std::string hostUrl_;
	std::string topic_;
	std::string clientId_;
	std::string userName_;
	std::string userPass_;
	bool initialized_ = false;
	long timeout_ = 10000;
	int lastResult_ = -1;
	bool subscribed_ = false;
	bool connLost_ = false;

	bool isSsl_ = false;
	std::string sslServerChainPem;
	std::string strClientKeyPem;
	int logLevel_ = 5;

	std::function<void(std::string const &, std::string const &)> fnSubCallback_;

	volatile MQTTClient_deliveryToken deliveredtoken_;

	std::string stage_;
	MQTTClient client;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;;
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	MQTTClient_deliveryToken token;

	std::function<void()> fnOnConnect_;
	std::function<void()> fnOnDisconnect_;
};