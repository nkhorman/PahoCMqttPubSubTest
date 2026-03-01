#pragma once

#include <functional>
#include <string>

#include "MQTTClient.h"
#include "StringRand.hpp"

class MqttSubPub
{
public:
	MqttSubPub(std::string const &id = "")
		: clientId_(id)
		, lastResult_(MQTTCLIENT_SUCCESS)
		{
			willTopicRetain_ = !clientId_.empty();
			if(clientId_.empty())
				clientId_ = "MqttSubPub" + StringRand::Simple(8);
			willTopic_ = clientId_ + "/status";
		};
	virtual ~MqttSubPub() { Shutdown(); }

	MqttSubPub &Connect(std::string const &url);

	inline MqttSubPub &Topic(std::string const &topic) { topic_ = topic; return *this;}
	inline std::string const &Topic() const { return topic_; }
	std::string LastResult();

	bool Publish(std::string const &value, int qos, int retain, std::string topic = "");
	inline std::string ClientId() const { return clientId_; }

	bool Subscribe(std::function<void(std::string const &topic, std::string const &msg)> fn, int qos = 0);
	bool Unsubscribe();

	void stage(std::string stage);
	void stageLastError();

	inline std::string stage() const { return stage_; }

	inline bool needShutdown() const { return connLost_; }
	inline bool isError() const { return lastResult_ != MQTTCLIENT_SUCCESS; }

	inline MqttSubPub &UserName(std::string const &str) { userName_ = str; return *this; }
	inline MqttSubPub &UserPass(std::string const &str) { userPass_ = str; return *this; }
#ifdef BUILD_MQTT_W_SSL
	inline MqttSubPub & SslCaPath(std::string const &str) { sslCaPath = str; return * this;}
	inline MqttSubPub & SslServerChain(std::string const &str) { sslServerChainPem = str; return * this;}
	inline MqttSubPub & SslClientKey(std::string const &str) { strClientKeyPem = str; return *this; }
#endif
	inline void LogLevel(int l) { logLevel_ = l; }
	inline void Debug(int d) { debug_ = d; }

	inline MqttSubPub &  OnConnect(std::function<void()> fn) { fnOnConnect_ = fn; return *this; }
	inline MqttSubPub &  OnDisconnect(std::function<void()> fn) { fnOnDisconnect_ = fn; return *this; }

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
	std::string willTopic_;
	bool willTopicRetain_ = false;

#ifdef BUILD_MQTT_W_SSL
	bool isSsl_ = false;
	std::string sslCaPath;
	std::string sslServerChainPem;
	std::string strClientKeyPem;
#endif
	int logLevel_ = 5;
	int debug_ = 0;

	std::function<void(std::string const &, std::string const &)> fnSubCallback_;

	volatile MQTTClient_deliveryToken deliveredtoken_;

	std::string stage_;
	MQTTClient client;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
#ifdef BUILD_MQTT_W_SSL
	MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;;
#endif
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	MQTTClient_deliveryToken token;
	MQTTClient_willOptions clientWill = MQTTClient_willOptions_initializer;

	std::function<void()> fnOnConnect_;
	std::function<void()> fnOnDisconnect_;
};