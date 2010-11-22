#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ogcsys.h>
#include <network.h>

#include "Network.h"
#include "Tools.h"
#include "System.h"

bool Network::Initialized = false;
char Network::IPAddress[16] = "0.0.0.0";

int Network::Startup()
{
	if (Initialized) return 0;
	int retryCount = 0;

	int ret;
	printf("Initializing Network...");
	fflush(stdout);
	
	Spinner::Start();	
	ret = if_config(IPAddress, NULL, NULL, true);
	if (ret < 0) 
	{
		Spinner::Stop();
		return ret;
	}

	while (System::State == SystemState::Running) 
	{
		ret = net_init();
		if (ret < 0) 
		{
			if (ret != -EAGAIN) 
			{
				retryCount++;
				printf("\b\n>> ERROR! net_init failed trying again: %d\n", ret);
				fflush(stdout);
			}
		}
		if (ret > -1 || retryCount > 4) break;
		printf("\b..");
		fflush(stdout);
	}
	Spinner::Stop();

	if (ret < 0 || retryCount != 0)
	{
		printf("\n>> ERROR! Failed to Initialize the network: ErrorCode (%d)\n", ret);
		return -1;
	}

	printf("\b.Done\n");
	Initialized = true;
	return 0;
}

void Network::ShutDown()
{
	if (!Initialized) return;
	net_deinit();
	Initialized = false;
}
