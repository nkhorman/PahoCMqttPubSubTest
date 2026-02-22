#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <getopt.h>

#include "MqttSubPub.hpp"

void OnChangeSub(std::string const &topic, std::string const &val)
{
	std::cout << "Subscription update - '" << topic << "' = '" << val << "'" << std::endl;
}

void mqtt(
	std::string const &host
	, std::string const &value
	, int qos
	, int retain
	, int tock
	, std::string const &strCaPath
	, std::string const &strServerCertPem
	, std::string const &strClientKeyPem
	, int LogLevel
	, int debug
	)
{
	MqttSubPub mq;
	std::cout << "mq clientid: " << mq.ClientId() << std::endl;

	mq.LogLevel(LogLevel);
	mq.Debug(debug);

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
		if(value.empty())
		{
			if(mq.Subscribe(OnChangeSub, qos))
				std::cout << "Subscribe - success" << std::endl;
			else
				std::cout << "Subscribe - fail - " << mq.LastResult() << std::endl;
		}
		else if(!mq.Publish(value, qos, retain))
			std::cout << "Publish - fail - " << mq.LastResult() << std::endl;
	}).Connect(host);

	int tick = 0;
	while(value.empty() && !mq.needShutdown() && !mq.isError())
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

void help()
{
	std::cout <<
		"Parameters;"
		"\n\t-u | --url\t- mqtt,mqtts://host/topic"
		"\n\t-v | --value\t- MQTT Write Value"
		"\n\t-q | --qos\t- MQTT Write QualityOfService 1,2,3"
		"\n\t--retain\t- MQTT Write Retain flag"
		"\n\t-c | --capath\t- CA Path"
		"\n\t-s | --serverchain\t- SSL PEM file with server Certificate chain"
		"\n\t-k | --clientkey\t- SSL PEM file with private key"
		"\n\t-l | --loglevel\t- MQTT / SSL protcol debug loging level 1-5"
		<< std::endl;
}

int main(int argc, char **argv)
{
	char const *url = "localhost";
	char const *value = "";
	int qos = 0;
	int retain = 0;
	int tick = 0;
	char const *strCaPath = "";
	char const *strServerChainPem = "";
	char const *strClientKeyPem = "";
	int LogLevel = 0;
	int debug = 0;

	struct option options[] =
	{
			{ "url", required_argument, NULL, 'u' },
			{ "value", required_argument, NULL, 'v' },
			{ "qos", required_argument, NULL, 'q' },
			{ "retain", no_argument, &retain, 1 },
			{ "tick", no_argument, &tick, 1 },
			{ "capath", required_argument, NULL, 'c' },
			{ "serverchain", required_argument, NULL, 's' },
			{ "clientkey", required_argument, NULL, 'k' },
			{ "loglevel", required_argument, NULL, 'l' },
			{ NULL, 0, 0, 0 }
	};


	int n = 0;
	while (n >= 0)  
	{
			n = getopt_long(argc, argv, "u:v:q:s:k:l:c:", options, NULL);
			if (n < 0)
					continue;
			switch (n)
			{
				case 'u': url = optarg; break;
				case 'v': value = optarg; break;
				case 'q': qos = atoi(optarg); break;
				case 's': strServerChainPem = optarg; break;
				case 'k': strClientKeyPem = optarg; break;
				case 'l': LogLevel = atoi(optarg); debug = (LogLevel > 0); break;
				case 'c': strCaPath = optarg; break;
			}
	}
	if(argc == 1)
		help();
	else
	{
		argc -= optind;
		argv += optind;

		mqtt(url
			, value
			, qos
			, retain
			, tick
			, strCaPath
			, strServerChainPem
			, strClientKeyPem
			, LogLevel
			, debug
			);
	}

	return 0;
}
