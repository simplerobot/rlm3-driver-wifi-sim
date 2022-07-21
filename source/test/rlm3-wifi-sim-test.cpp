#include "Test.hpp"
#include "rlm3-wifi.h"
#include "rlm3-task.h"
#include <cstring>


static volatile size_t g_recv_buffer_count = 0;
static char g_recv_buffer_data[32];
static RLM3_Task g_recv_thread = nullptr;


extern void RLM3_WIFI_Receive_Callback(uint8_t data)
{
	if (g_recv_buffer_count < sizeof(g_recv_buffer_data))
		g_recv_buffer_data[g_recv_buffer_count] = data;
	g_recv_buffer_count++;
	RLM3_GiveFromISR(g_recv_thread);
}


TEST_CASE(RLM3_WIFI_Init_HappyCase)
{
	ASSERT(!RLM3_WIFI_IsInit());
	ASSERT(RLM3_WIFI_Init());
	ASSERT(RLM3_WIFI_IsInit());
}

TEST_CASE(RLM3_WIFI_Init_Failure)
{
	SIM_RLM3_WIFI_InitFailure();

	ASSERT(!RLM3_WIFI_Init());
	ASSERT(!RLM3_WIFI_IsInit());
}

TEST_CASE(RLM3_WIFI_Init_DuplicateInit)
{
	RLM3_WIFI_Init();

	ASSERT_ASSERTS(RLM3_WIFI_Init());
}

TEST_CASE(RLM3_WIFI_Deinit_HappyCase)
{
	RLM3_WIFI_Init();

	RLM3_WIFI_Deinit();

	ASSERT(!RLM3_WIFI_IsInit());
}

TEST_CASE(RLM3_WIFI_Deinit_NotInitialized)
{
	ASSERT_ASSERTS(RLM3_WIFI_Deinit());
}

TEST_CASE(RLM3_WIFI_GetVersion_HappyCase)
{
	SIM_RLM3_WIFI_SetVersion(0x12345678, 0x13579BDF);
	RLM3_WIFI_Init();

	uint32_t at_version = 0;
	uint32_t sdk_version = 0;
	ASSERT(RLM3_WIFI_GetVersion(&at_version, &sdk_version));

	ASSERT(at_version == 0x12345678);
	ASSERT(sdk_version == 0x13579BDF);
}

TEST_CASE(RLM3_WIFI_GetVersion_NotInitialized)
{
	SIM_RLM3_WIFI_SetVersion(0x12345678, 0x13579BDF);

	uint32_t at_version = 0;
	uint32_t sdk_version = 0;
	ASSERT_ASSERTS(RLM3_WIFI_GetVersion(&at_version, &sdk_version));
}

TEST_CASE(RLM3_WIFI_GetVersion_NotSet)
{
	RLM3_WIFI_Init();

	uint32_t at_version = 0;
	uint32_t sdk_version = 0;
	ASSERT(!RLM3_WIFI_GetVersion(&at_version, &sdk_version));

	ASSERT(at_version == 0);
	ASSERT(sdk_version == 0);
}

TEST_CASE(RLM3_WIFI_NetworkConnect_HappyCase)
{
	SIM_RLM3_WIFI_SetNetwork("test-ssid", "test-password");
	RLM3_WIFI_Init();

	ASSERT(!RLM3_WIFI_IsNetworkConnected());
	ASSERT(RLM3_WIFI_NetworkConnect("test-ssid", "test-password"));
	ASSERT(RLM3_WIFI_IsNetworkConnected());
}

TEST_CASE(RLM3_WIFI_NetworkConnect_NotInitialized)
{
	SIM_RLM3_WIFI_SetNetwork("test-ssid", "test-password");

	ASSERT_ASSERTS(RLM3_WIFI_NetworkConnect("test-ssid", "test-password"));
}

TEST_CASE(RLM3_WIFI_NetworkConnect_NotSet)
{
	RLM3_WIFI_Init();

	ASSERT(!RLM3_WIFI_NetworkConnect("test-ssid", "test-password"));
	ASSERT(!RLM3_WIFI_IsNetworkConnected());
}

TEST_CASE(RLM3_WIFI_NetworkConnect_WrongSsid)
{
	SIM_RLM3_WIFI_SetNetwork("test-ssid", "test-password");
	RLM3_WIFI_Init();

	ASSERT_ASSERTS(RLM3_WIFI_NetworkConnect("test-ssid-wrong", "test-password"));
}

TEST_CASE(RLM3_WIFI_NetworkConnect_WrongPassword)
{
	SIM_RLM3_WIFI_SetNetwork("test-ssid", "test-password");
	RLM3_WIFI_Init();

	ASSERT_ASSERTS(RLM3_WIFI_NetworkConnect("test-ssid", "test-password-wrong"));
}

TEST_CASE(RLM3_WIFI_NetworkConnect_AndDeinit)
{
	SIM_RLM3_WIFI_SetNetwork("test-ssid", "test-password");
	RLM3_WIFI_Init();

	ASSERT(RLM3_WIFI_NetworkConnect("test-ssid", "test-password"));
	RLM3_WIFI_Deinit();
	ASSERT(!RLM3_WIFI_IsNetworkConnected());
}

TEST_CASE(RLM3_WIFI_NetworkDisconnect_HappyCase)
{
	SIM_RLM3_WIFI_SetNetwork("test-ssid", "test-password");
	RLM3_WIFI_Init();

	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_NetworkDisconnect();
	ASSERT(!RLM3_WIFI_IsNetworkConnected());
}

TEST_CASE(RLM3_WIFI_NetworkDisconnect_NotConnected)
{
	RLM3_WIFI_Init();

	ASSERT_ASSERTS(RLM3_WIFI_NetworkDisconnect());
}

TEST_CASE(RLM3_WIFI_NetworkDisconnect_NotInit)
{
	ASSERT_ASSERTS(RLM3_WIFI_NetworkDisconnect());
}

TEST_CASE(RLM3_WIFI_ServerConnect_HappyCase)
{
	SIM_RLM3_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_RLM3_WIFI_SetServer("test-server", "test-service");
	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");

	ASSERT(!RLM3_WIFI_IsServerConnected());
	ASSERT(RLM3_WIFI_ServerConnect("test-server", "test-service"));
	ASSERT(RLM3_WIFI_IsServerConnected());
}

TEST_CASE(RLM3_WIFI_ServerConnect_NoNetwork)
{
	SIM_RLM3_WIFI_SetServer("test-server", "test-service");
	RLM3_WIFI_Init();

	ASSERT_ASSERTS(RLM3_WIFI_ServerConnect("test-server", "test-service"));
}

TEST_CASE(RLM3_WIFI_ServerConnect_NotSet)
{
	SIM_RLM3_WIFI_SetNetwork("test-ssid", "test-password");
	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");

	ASSERT(!RLM3_WIFI_ServerConnect("test-server", "test-service"));
	ASSERT(!RLM3_WIFI_IsServerConnected());
}

TEST_CASE(RLM3_WIFI_ServerConnect_WrongServer)
{
	SIM_RLM3_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_RLM3_WIFI_SetServer("test-server", "test-service");
	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");

	ASSERT_ASSERTS(RLM3_WIFI_ServerConnect("test-server-wrong", "test-service"));
}

TEST_CASE(RLM3_WIFI_ServerConnect_WrongService)
{
	SIM_RLM3_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_RLM3_WIFI_SetServer("test-server", "test-service");
	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");

	ASSERT_ASSERTS(RLM3_WIFI_ServerConnect("test-server", "test-service-wrong"));
}

TEST_CASE(RLM3_WIFI_ServerDisconnect_HappyCase)
{
	SIM_RLM3_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_RLM3_WIFI_SetServer("test-server", "test-service");
	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_ServerConnect("test-server", "test-service");

	RLM3_WIFI_ServerDisconnect();
	ASSERT(!RLM3_WIFI_IsServerConnected());
}

TEST_CASE(RLM3_WIFI_ServerDisconnect_NotConnected)
{
	SIM_RLM3_WIFI_SetNetwork("test-ssid", "test-password");
	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");

	ASSERT_ASSERTS(RLM3_WIFI_ServerDisconnect());
}

TEST_CASE(RLM3_WIFI_ServerDisconnect_NetworkDisconnect)
{
	SIM_RLM3_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_RLM3_WIFI_SetServer("test-server", "test-service");
	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_ServerConnect("test-server", "test-service");

	RLM3_WIFI_NetworkDisconnect();
	ASSERT(!RLM3_WIFI_IsServerConnected());
}

TEST_CASE(RLM3_WIFI_ServerDisconnect_DeInit)
{
	SIM_RLM3_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_RLM3_WIFI_SetServer("test-server", "test-service");
	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_ServerConnect("test-server", "test-service");

	RLM3_WIFI_Deinit();
	ASSERT(!RLM3_WIFI_IsServerConnected());
}

TEST_CASE(RLM3_WIFI_Transmit_HappyCase)
{
	SIM_RLM3_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_RLM3_WIFI_SetServer("test-server", "test-service");
	SIM_RLM3_WIFI_Transmit("abcd");

	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_ServerConnect("test-server", "test-service");

	ASSERT(RLM3_WIFI_Transmit((const uint8_t*)"abcd", 4));
}

TEST_CASE(RLM3_WIFI_Transmit_NotSet)
{
	SIM_RLM3_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_RLM3_WIFI_SetServer("test-server", "test-service");

	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_ServerConnect("test-server", "test-service");

	ASSERT(!RLM3_WIFI_Transmit((const uint8_t*)"abcd", 4));
}

TEST_CASE(RLM3_WIFI_Transmit_NotActive)
{
	SIM_RLM3_WIFI_Transmit("abcd");

	ASSERT_ASSERTS(RLM3_WIFI_Transmit((const uint8_t*)"abcd", 4));
}

TEST_CASE(RLM3_WIFI_Transmit_DataMismatch)
{
	SIM_RLM3_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_RLM3_WIFI_SetServer("test-server", "test-service");
	SIM_RLM3_WIFI_Transmit("abcd");

	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_ServerConnect("test-server", "test-service");

	ASSERT_ASSERTS(RLM3_WIFI_Transmit((const uint8_t*)"abce", 4));
}

TEST_CASE(RLM3_WIFI_Transmit_SizeMismatch)
{
	SIM_RLM3_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_RLM3_WIFI_SetServer("test-server", "test-service");
	SIM_RLM3_WIFI_Transmit("abcd");

	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_ServerConnect("test-server", "test-service");

	ASSERT_ASSERTS(RLM3_WIFI_Transmit((const uint8_t*)"abcde", 5));
}

TEST_CASE(RLM3_WIFI_Transmit_InvalidSizeZero)
{
	SIM_RLM3_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_RLM3_WIFI_SetServer("test-server", "test-service");

	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_ServerConnect("test-server", "test-service");

	ASSERT_ASSERTS(RLM3_WIFI_Transmit((const uint8_t*)"", 0));
}

TEST_CASE(RLM3_WIFI_Transmit_InvalidSizeTooBig)
{
	uint8_t buffer[1026] = {};
	for (size_t i = 0; i < 1025; i++)
		buffer[i] = 'a';

	SIM_RLM3_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_RLM3_WIFI_SetServer("test-server", "test-service");
	SIM_RLM3_WIFI_Transmit((const char*)buffer);

	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_ServerConnect("test-server", "test-service");

	ASSERT_ASSERTS(RLM3_WIFI_Transmit(buffer, 1025));
}

TEST_CASE(RLM3_WIFI_Receive_HappyCase)
{
	SIM_RLM3_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_RLM3_WIFI_SetServer("test-server", "test-service");
	SIM_RLM3_WIFI_Receive("abc");
	SIM_RLM3_WIFI_Receive("def");

	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_ServerConnect("test-server", "test-service");

	g_recv_buffer_count = 0;
	g_recv_thread = RLM3_GetCurrentTask();

	while (g_recv_buffer_count == 0)
		RLM3_Take();
	ASSERT(g_recv_buffer_count == 3);
	ASSERT(std::strncmp(g_recv_buffer_data, "abc", 3) == 0);
	while (g_recv_buffer_count == 3)
		RLM3_Take();
	ASSERT(g_recv_buffer_count == 6);
	ASSERT(std::strncmp(g_recv_buffer_data, "abcdef", 6) == 0);
}

