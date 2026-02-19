#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <getopt.h>

#include "MqttSubPub.hpp"

void callback(std::string const &topic, std::string const &val)
{
	std::cout << "Subscription update - '" << topic << "' = '" << val << "'" << std::endl;
}

void mqtt(
	std::string const &host
	, std::string const &topic
	, std::string const &value
	, int qos
	, int retain
	, int tock
	, std::string const &userName
	, std::string const &userPass
	, std::string const &strCaPath
	, std::string const &strServerCertPem
	, std::string const &strClientKeyPem
	, int LogLevel
	)
{
	MqttSubPub mq;
	std::cout << "mq clientid: " << mq.ClientId() << std::endl;

	mq.LogLevel(LogLevel);

	if(!userName.empty())
	{
		std::cout << "user: " << userName << ", pass: " << userPass << std::endl;
		mq.UserName(userName)
		.UserPass(userPass);
	}

#ifdef BUILD_MQTT_W_SSL
	if(!strServerCertPem.empty() || !strClientKeyPem.empty() || !strCaPath.empty())
	{
		std::cout << "CaPath: " << strCaPath << ", chain: " << strServerCertPem << ", client: " << strClientKeyPem << std::endl;
		mq.SslCaPath(strCaPath)
			.SslServerChain(strServerCertPem)
			.SslClientKey(strClientKeyPem);
	}
#endif

	std::cout << "Connect - '" << host << "'" << std::endl;
	mq.OnConnect([&]()
	{
		std::cout << "Subscribe - '" << topic << "'" << std::endl;
		if(mq.Topic(topic).Subscribe(callback, qos))
			std::cout << "Subscribe - success - " << mq.LastResult() << std::endl;
		else
			std::cout << "Subscribe - fail - " << mq.LastResult() << std::endl;
	}).Connect(host);

	int tick = 0;
	while(!mq.needShutdown() && !mq.isError())
	{
		usleep(1000 * 1000);
		tick +=1;
		if(tock && tick % 10 == 0)
		{
			std::string s = std::to_string(tick);
			mq.Publish(s, qos, retain);
		}
	}
}

int main(int argc, char **argv)
{
	char const *url = "localhost";
	char const *topic = "test topic";
	char const *value = "test value";
	int qos = 0;
	int retain = 0;
	int tick = 0;
	char const *strCaPath = "";
	char const *strServerChainPem = "";
	char const *strClientKeyPem = "";
	char const *userName = "";
	char const *userPass = "";
	int LogLevel = 0;

	struct option options[] =
	{
			{ "url", required_argument, NULL, 'u' },
			{ "topic", required_argument, NULL, 't' },
			{ "value", required_argument, NULL, 'v' },
			{ "qos", required_argument, NULL, 'q' },
			{ "retain", no_argument, &retain, 1 },
			{ "tick", no_argument, &tick, 1 },
			{ "username", required_argument, NULL, 'n' },
			{ "userpass", required_argument, NULL, 'p' },
			{ "capath", required_argument, NULL, 'c' },
			{ "serverchain", required_argument, NULL, 's' },
			{ "clientkey", required_argument, NULL, 'k' },
			{ "loglevel", required_argument, NULL, 'l' },
			{ NULL, 0, 0, 0 }
	};


	int n = 0;
	while (n >= 0)  
	{
			n = getopt_long(argc, argv, "u:t:v:q:n:p:s:k:l:c:", options, NULL);
			if (n < 0)
					continue;
			switch (n)
			{
				case 'u': url = optarg; break;
				case 't': topic = optarg; break;
				case 'v': value = optarg; break;
				case 'q': qos = atoi(optarg); break;
				case 'n': userName = optarg; break;
				case 'p': userPass = optarg; break;
				case 's': strServerChainPem = optarg; break;
				case 'k': strClientKeyPem = optarg; break;
				case 'l': LogLevel = atoi(optarg); break;
				case 'c': strCaPath = optarg; break;
			}
	}
	argc -= optind;
	argv += optind;

	mqtt(url
		, topic
		, value
		, qos
		, retain
		, tick
		, userName
		, userPass
		, strCaPath
		, strServerChainPem
		, strClientKeyPem
		, LogLevel
		);

	return 0;
}
