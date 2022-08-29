#include "Test.hpp"
#include "rlm3-wifi.h"
#include "rlm3-task.h"
#include <cstring>


struct LinkRecvInfo
{
	volatile size_t count;
	char buffer[32];
};
static LinkRecvInfo g_link_recv_info[RLM3_WIFI_LINK_COUNT];
static volatile RLM3_Task g_task;
static bool g_network_connect_called = false;
static bool g_network_disconnect_called = false;
static size_t g_network_connect_link_id = 0;
static size_t g_network_disconnect_link_id = 0;


extern void RLM3_WIFI_Receive_Callback(size_t link_id, uint8_t data)
{
	ASSERT(link_id < RLM3_WIFI_LINK_COUNT);
	auto& link_info = g_link_recv_info[link_id];

	if (link_info.count < sizeof(link_info.buffer))
		link_info.buffer[link_info.count] = data;
	link_info.count++;
	RLM3_GiveFromISR(g_task);
}

extern void RLM3_WIFI_NetworkConnect_Callback(size_t link_id, bool local_connection)
{
	g_network_connect_called = true;
	g_network_connect_link_id = link_id;
	RLM3_GiveFromISR(g_task);
}

extern void RLM3_WIFI_NetworkDisconnect_Callback(size_t link_id, bool local_connection)
{
	g_network_disconnect_called = true;
	g_network_disconnect_link_id = link_id;
	RLM3_GiveFromISR(g_task);
}


TEST_CASE(RLM3_WIFI_Init_HappyCase)
{
	ASSERT(!RLM3_WIFI_IsInit());
	ASSERT(RLM3_WIFI_Init());
	ASSERT(RLM3_WIFI_IsInit());
}

TEST_CASE(RLM3_WIFI_Init_Failure)
{
	SIM_WIFI_InitFailure();

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
	SIM_WIFI_SetVersion(0x12345678, 0x13579BDF);
	RLM3_WIFI_Init();

	uint32_t at_version = 0;
	uint32_t sdk_version = 0;
	ASSERT(RLM3_WIFI_GetVersion(&at_version, &sdk_version));

	ASSERT(at_version == 0x12345678);
	ASSERT(sdk_version == 0x13579BDF);
}

TEST_CASE(RLM3_WIFI_GetVersion_NotInitialized)
{
	SIM_WIFI_SetVersion(0x12345678, 0x13579BDF);

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
	SIM_WIFI_SetNetwork("test-ssid", "test-password");
	RLM3_WIFI_Init();

	ASSERT(!RLM3_WIFI_IsNetworkConnected());
	ASSERT(RLM3_WIFI_NetworkConnect("test-ssid", "test-password"));
	ASSERT(RLM3_WIFI_IsNetworkConnected());
}

TEST_CASE(RLM3_WIFI_NetworkConnect_NotInitialized)
{
	SIM_WIFI_SetNetwork("test-ssid", "test-password");

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
	SIM_WIFI_SetNetwork("test-ssid", "test-password");
	RLM3_WIFI_Init();

	ASSERT_ASSERTS(RLM3_WIFI_NetworkConnect("test-ssid-wrong", "test-password"));
}

TEST_CASE(RLM3_WIFI_NetworkConnect_WrongPassword)
{
	SIM_WIFI_SetNetwork("test-ssid", "test-password");
	RLM3_WIFI_Init();

	ASSERT_ASSERTS(RLM3_WIFI_NetworkConnect("test-ssid", "test-password-wrong"));
}

TEST_CASE(RLM3_WIFI_NetworkConnect_AndDeinit)
{
	SIM_WIFI_SetNetwork("test-ssid", "test-password");
	RLM3_WIFI_Init();

	ASSERT(RLM3_WIFI_NetworkConnect("test-ssid", "test-password"));
	RLM3_WIFI_Deinit();
	ASSERT(!RLM3_WIFI_IsNetworkConnected());
}

TEST_CASE(RLM3_WIFI_NetworkDisconnect_HappyCase)
{
	SIM_WIFI_SetNetwork("test-ssid", "test-password");
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
	SIM_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_WIFI_SetServer(0, "test-server", "test-service");
	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");

	ASSERT(!RLM3_WIFI_IsServerConnected(0));
	ASSERT(RLM3_WIFI_ServerConnect(0, "test-server", "test-service"));
	ASSERT(RLM3_WIFI_IsServerConnected(0));
}

TEST_CASE(RLM3_WIFI_ServerConnect_NoNetwork)
{
	SIM_WIFI_SetServer(0, "test-server", "test-service");
	RLM3_WIFI_Init();

	ASSERT_ASSERTS(RLM3_WIFI_ServerConnect(0, "test-server", "test-service"));
}

TEST_CASE(RLM3_WIFI_ServerConnect_NotSet)
{
	SIM_WIFI_SetNetwork("test-ssid", "test-password");
	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");

	ASSERT(!RLM3_WIFI_ServerConnect(0, "test-server", "test-service"));
	ASSERT(!RLM3_WIFI_IsServerConnected(0));
}

TEST_CASE(RLM3_WIFI_ServerConnect_WrongServer)
{
	SIM_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_WIFI_SetServer(0, "test-server", "test-service");
	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");

	ASSERT_ASSERTS(RLM3_WIFI_ServerConnect(0, "test-server-wrong", "test-service"));
}

TEST_CASE(RLM3_WIFI_ServerConnect_WrongService)
{
	SIM_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_WIFI_SetServer(0, "test-server", "test-service");
	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");

	ASSERT_ASSERTS(RLM3_WIFI_ServerConnect(0, "test-server", "test-service-wrong"));
}

TEST_CASE(RLM3_WIFI_ServerDisconnect_HappyCase)
{
	SIM_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_WIFI_SetServer(0, "test-server", "test-service");
	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_ServerConnect(0, "test-server", "test-service");

	RLM3_WIFI_ServerDisconnect(0);
	ASSERT(!RLM3_WIFI_IsServerConnected(0));
}

TEST_CASE(RLM3_WIFI_ServerDisconnect_NotConnected)
{
	SIM_WIFI_SetNetwork("test-ssid", "test-password");
	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");

	ASSERT_ASSERTS(RLM3_WIFI_ServerDisconnect(0));
}

TEST_CASE(RLM3_WIFI_ServerDisconnect_NetworkDisconnect)
{
	SIM_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_WIFI_SetServer(0, "test-server", "test-service");
	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_ServerConnect(0, "test-server", "test-service");

	RLM3_WIFI_NetworkDisconnect();
	ASSERT(!RLM3_WIFI_IsServerConnected(0));
}

TEST_CASE(RLM3_WIFI_ServerDisconnect_DeInit)
{
	SIM_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_WIFI_SetServer(0, "test-server", "test-service");
	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_ServerConnect(0, "test-server", "test-service");

	RLM3_WIFI_Deinit();
	ASSERT(!RLM3_WIFI_IsServerConnected(0));
}

TEST_CASE(RLM3_WIFI_Transmit_HappyCase)
{
	SIM_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_WIFI_SetServer(0, "test-server", "test-service");
	SIM_WIFI_Transmit(0, "abcd");

	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_ServerConnect(0, "test-server", "test-service");

	ASSERT(RLM3_WIFI_Transmit(0, (const uint8_t*)"abcd", 4));
}

TEST_CASE(RLM3_WIFI_Transmit2_HappyCase)
{
	SIM_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_WIFI_SetServer(0, "test-server", "test-service");
	SIM_WIFI_Transmit(0, "abcdefg");

	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_ServerConnect(0, "test-server", "test-service");

	ASSERT(RLM3_WIFI_Transmit2(0, (const uint8_t*)"abcd", 4, (const uint8_t*)"efg", 3));
}

TEST_CASE(RLM3_WIFI_Transmit_NotSet)
{
	SIM_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_WIFI_SetServer(0, "test-server", "test-service");

	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_ServerConnect(0, "test-server", "test-service");

	ASSERT(!RLM3_WIFI_Transmit(0, (const uint8_t*)"abcd", 4));
}

TEST_CASE(RLM3_WIFI_Transmit_NotActive)
{
	SIM_WIFI_Transmit(0, "abcd");

	ASSERT_ASSERTS(RLM3_WIFI_Transmit(0, (const uint8_t*)"abcd", 4));
}

TEST_CASE(RLM3_WIFI_Transmit_DataMismatch)
{
	SIM_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_WIFI_SetServer(0, "test-server", "test-service");
	SIM_WIFI_Transmit(0, "abcd");

	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_ServerConnect(0, "test-server", "test-service");

	ASSERT_ASSERTS(RLM3_WIFI_Transmit(0, (const uint8_t*)"abce", 4));
}

TEST_CASE(RLM3_WIFI_Transmit_SizeMismatch)
{
	SIM_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_WIFI_SetServer(0, "test-server", "test-service");
	SIM_WIFI_Transmit(0, "abcd");

	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_ServerConnect(0, "test-server", "test-service");

	ASSERT_ASSERTS(RLM3_WIFI_Transmit(0, (const uint8_t*)"abcde", 5));
}

TEST_CASE(RLM3_WIFI_Transmit_InvalidSizeZero)
{
	SIM_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_WIFI_SetServer(0, "test-server", "test-service");

	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_ServerConnect(0, "test-server", "test-service");

	ASSERT_ASSERTS(RLM3_WIFI_Transmit(0, (const uint8_t*)"", 0));
}

TEST_CASE(RLM3_WIFI_Transmit_InvalidSizeTooBig)
{
	uint8_t buffer[1026] = {};
	for (size_t i = 0; i < 1025; i++)
		buffer[i] = 'a';

	SIM_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_WIFI_SetServer(0, "test-server", "test-service");
	SIM_WIFI_Transmit(0, (const char*)buffer);

	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_ServerConnect(0, "test-server", "test-service");

	ASSERT_ASSERTS(RLM3_WIFI_Transmit(0, buffer, 1025));
}

TEST_CASE(RLM3_WIFI_Receive_HappyCase)
{
	SIM_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_WIFI_SetServer(0, "test-server", "test-service");
	SIM_WIFI_Receive(0, "abc");
	SIM_WIFI_Receive(0, "def");

	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_ServerConnect(0, "test-server", "test-service");

	auto& link_recv_info = g_link_recv_info[0];

	while (link_recv_info.count == 0)
		RLM3_Take();
	ASSERT(link_recv_info.count == 3);
	ASSERT(std::strncmp(link_recv_info.buffer, "abc", 3) == 0);
	while (link_recv_info.count == 3)
		RLM3_Take();
	ASSERT(link_recv_info.count == 6);
	ASSERT(std::strncmp(link_recv_info.buffer, "abcdef", 6) == 0);
}

TEST_CASE(RLM3_WIFI_ReceiveMultiple_HappyCase)
{
	SIM_WIFI_SetNetwork("test-ssid", "test-password");
	SIM_WIFI_SetServer(0, "test-server", "test-service");
	SIM_WIFI_Receive(0, "abc");
	SIM_WIFI_Receive(0, "def");
	SIM_WIFI_SetServer(1, "test-server-b", "test-service-b");
	SIM_WIFI_Receive(1, "uvw");
	SIM_WIFI_Receive(1, "xyz");

	RLM3_WIFI_Init();
	RLM3_WIFI_NetworkConnect("test-ssid", "test-password");
	RLM3_WIFI_ServerConnect(0, "test-server", "test-service");
	RLM3_WIFI_ServerConnect(1, "test-server-b", "test-service-b");

	while (g_link_recv_info[1].count == 0)
		RLM3_Take();
	ASSERT(g_link_recv_info[1].count == 3);
	ASSERT(std::strncmp(g_link_recv_info[1].buffer, "uvw", 3) == 0);
	while (g_link_recv_info[1].count == 3)
		RLM3_Take();
	ASSERT(g_link_recv_info[1].count == 6);
	ASSERT(std::strncmp(g_link_recv_info[1].buffer, "uvwxyz", 6) == 0);

	ASSERT(g_link_recv_info[0].count == 6);
	ASSERT(std::strncmp(g_link_recv_info[0].buffer, "abcdef", 6) == 0);
}

TEST_CASE(RLM3_WIFI_LocalNetwork_HappyCase)
{
	SIM_WIFI_SetLocalNetwork("test-ssid", "test-password", 2, "test-ip-address", "test-service");
	SIM_WIFI_Connect(0);
	SIM_WIFI_Receive(0, "abc");
	SIM_WIFI_Connect(1);
	SIM_WIFI_Transmit(1, "hello world");
	SIM_WIFI_Disconnect(1);
	SIM_WIFI_Disconnect(0);

	RLM3_WIFI_Init();
	RLM3_WIFI_LocalNetworkEnable("test-ssid", "test-password", 2, "test-ip-address", "test-service");
	ASSERT(!g_network_connect_called);
	RLM3_Take();
	ASSERT(g_network_connect_called);
	ASSERT(g_network_connect_link_id == 0);
	g_network_connect_called = false;
	ASSERT(g_link_recv_info[0].count == 0);
	RLM3_Take();
	ASSERT(g_link_recv_info[0].count == 3);
	ASSERT(!g_network_connect_called);
	RLM3_Take();
	ASSERT(g_network_connect_called);
	ASSERT(g_network_connect_link_id == 1);
	RLM3_WIFI_Transmit(1, (const uint8_t*)"hello world", 11);
	ASSERT(!g_network_disconnect_called);
	RLM3_Take();
	ASSERT(g_network_disconnect_called);
	ASSERT(g_network_disconnect_link_id == 1);
	g_network_disconnect_called = false;
	RLM3_Take();
	ASSERT(g_network_disconnect_called);
	ASSERT(g_network_disconnect_link_id == 0);
}

TEST_SETUP(WIFI_TEST_SETUP)
{
	for (auto& i : g_link_recv_info)
	{
		i.count = 0;
	}
	g_network_connect_called = false;
	g_network_disconnect_called = false;
	g_task = RLM3_GetCurrentTask();
}
