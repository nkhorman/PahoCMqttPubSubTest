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
	, std::string const &value
	, std::string const &topic
	, int qos
	, int retain
	, int tock
	, std::string const &userName
	, std::string const &userPass
	, std::string const &strServerCertPem
	, std::string const &strClientKeyPem
	)
{
	MqttSubPub mq;
	std::cout << "mq clientid: " << mq.ClientId() << std::endl;
	std::cout << "Subscribe - '" << host << "' : '" << topic << "'" << std::endl;

	if(!userName.empty())
	{
		std::cout << "user: " << userName << ", pass: " << userPass << std::endl;
		mq.UserName(userName)
		.UserPass(userPass);
	}

	if(!strServerCertPem.empty())
	{
		std::cout << "chain: " << strServerCertPem << ", client: " << strClientKeyPem << std::endl;
		mq.SslServerChain(strServerCertPem)
			.SslClientKey(strClientKeyPem);
	}

	if(mq
		.Host(host)
		.Topic(topic)
		.Subscribe(callback, qos)
	)
		std::cout << "Subscribe - success - " << mq.LastResult() << std::endl;
	else
		std::cout << "Subscribe - fail - " << mq.LastResult() << std::endl;

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
	char const *strServerChainPem = "";
	char const *strClientKeyPem = "";
	char const *userName = "";
	char const *userPass = "";

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
			{ "serverchain", required_argument, NULL, 's' },
			{ "clientkey", required_argument, NULL, 'k' },
			{ NULL, 0, 0, 0 }
	};


	int n = 0;
	while (n >= 0)  
	{
			n = getopt_long(argc, argv, "u:t:v:q:n:p:s:k:", options, NULL);
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
		, strServerChainPem
		, strClientKeyPem
		);

	return 0;
}
