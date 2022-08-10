#include "rlm3-wifi.h"
#include "rlm3-sim.hpp"
#include "Test.hpp"
#include <string>
#include <queue>


static bool g_fail_init;
static bool g_is_active;
static bool g_has_version;
static uint32_t g_at_version;
static uint32_t g_sdk_version;
static bool g_has_network;
static std::string g_ssid;
static std::string g_password;
static bool g_is_network_connected;

struct ServerSettings
{
	bool has_server;
	std::string server;
	std::string service;
	bool is_connected;
	std::queue<uint8_t> transmit_queue;
};
static ServerSettings g_server_settings[RLM3_WIFI_LINK_COUNT];


TEST_SETUP(SIM_WIFI_Init)
{
	g_is_active = false;
	g_fail_init = false;
	g_has_version = false;
	g_has_network = false;
	g_is_network_connected = false;
	for (auto& s : g_server_settings)
	{
		s.has_server = false;
		s.is_connected = false;
		while (!s.transmit_queue.empty())
			s.transmit_queue.pop();
	}
}


extern bool RLM3_WIFI_Init()
{
	ASSERT(!g_is_active);
	if (g_fail_init)
		return false;
	g_is_active = true;
	return true;
}

extern void RLM3_WIFI_Deinit()
{
	ASSERT(g_is_active);
	g_is_active = false;
	g_is_network_connected = false;
	for (auto& s : g_server_settings)
		s.is_connected = false;
}

extern bool RLM3_WIFI_IsInit()
{
	return g_is_active;
}

extern bool RLM3_WIFI_GetVersion(uint32_t* at_version, uint32_t* sdk_version)
{
	ASSERT(g_is_active);
	if (!g_has_version)
		return false;
	*at_version = g_at_version;
	*sdk_version = g_sdk_version;
	return true;
}

extern bool RLM3_WIFI_NetworkConnect(const char* ssid, const char* password)
{
	ASSERT(g_is_active);
	ASSERT(!g_is_network_connected);
	if (!g_has_network)
		return false;
	ASSERT(ssid == g_ssid);
	ASSERT(password == g_password);
	g_is_network_connected = true;
	return true;
}

extern void RLM3_WIFI_NetworkDisconnect()
{
	ASSERT(g_is_network_connected);
	g_is_network_connected = false;
	for (auto& s : g_server_settings)
		s.is_connected = false;
}

extern bool RLM3_WIFI_IsNetworkConnected()
{
	return g_is_network_connected;
}

extern bool RLM3_WIFI_ServerConnect(size_t link_id, const char* server, const char* service)
{
	ASSERT(link_id < RLM3_WIFI_LINK_COUNT);
	ASSERT(g_is_active);
	ASSERT(g_is_network_connected);
	auto& s = g_server_settings[link_id];
	ASSERT(!s.is_connected);
	if (!s.has_server)
		return false;
	ASSERT(server == s.server);
	ASSERT(service == s.service);
	s.is_connected = true;
	return true;
}

extern void RLM3_WIFI_ServerDisconnect(size_t link_id)
{
	ASSERT(link_id < RLM3_WIFI_LINK_COUNT);
	ASSERT(g_is_active);
	ASSERT(g_is_network_connected);
	auto& s = g_server_settings[link_id];
	ASSERT(s.is_connected);
	s.is_connected = false;
}

extern bool RLM3_WIFI_IsServerConnected(size_t link_id)
{
	ASSERT(link_id < RLM3_WIFI_LINK_COUNT);
	auto& s = g_server_settings[link_id];
	return s.is_connected;
}

extern bool RLM3_WIFI_Transmit(size_t link_id, const uint8_t* data, size_t size)
{
	ASSERT(link_id < RLM3_WIFI_LINK_COUNT);
	ASSERT(g_is_active);
	ASSERT(g_is_network_connected);
	auto& s = g_server_settings[link_id];
	ASSERT(s.is_connected);
	ASSERT(size > 0 && size <= 1024);
	if (s.transmit_queue.empty())
		return false;
	for (size_t i = 0; i < size; i++)
	{
		ASSERT(!s.transmit_queue.empty());
		uint8_t expected_transmit_byte = s.transmit_queue.front();
		s.transmit_queue.pop();
		uint8_t actual_transmit_byte = data[i];
		ASSERT(actual_transmit_byte == expected_transmit_byte);
	}
	return true;
}

extern __attribute__((weak)) void RLM3_WIFI_Receive_Callback(uint8_t data)
{
	// DO NOT MODIFIY THIS FUNCTION.  Override it by declaring a non-weak version in your project files.
}


extern void SIM_RLM3_WIFI_InitFailure()
{
	g_fail_init = true;
}

extern void SIM_RLM3_WIFI_SetVersion(uint32_t at_version, uint32_t sdk_version)
{
	g_has_version = true;
	g_at_version = at_version;
	g_sdk_version = sdk_version;
}

extern void SIM_RLM3_WIFI_SetNetwork(const char* ssid, const char* password)
{
	g_has_network = true;
	g_ssid = ssid;
	g_password = password;
}

extern void SIM_RLM3_WIFI_SetServer(size_t link_id, const char* server, const char* service)
{
	ASSERT(link_id < RLM3_WIFI_LINK_COUNT);
	auto& s = g_server_settings[link_id];
	s.has_server = true;
	s.server = server;
	s.service = service;
}

extern void SIM_RLM3_WIFI_Transmit(size_t link_id, const char* expected)
{
	ASSERT(link_id < RLM3_WIFI_LINK_COUNT);
	auto& s = g_server_settings[link_id];
	for (const char* cursor = expected; *cursor != 0; cursor++)
		s.transmit_queue.push(*cursor);
}

extern void SIM_RLM3_WIFI_Receive(size_t link_id, const char* data)
{
	ASSERT(link_id < RLM3_WIFI_LINK_COUNT);
	std::string str(data);
	SIM_AddInterrupt([=]() {
		ASSERT(g_is_active);
		ASSERT(g_is_network_connected);
		ASSERT(link_id < RLM3_WIFI_LINK_COUNT);
		auto& s = g_server_settings[link_id];
		ASSERT(s.is_connected);
		for (char c : str)
			RLM3_WIFI_Receive_Callback(link_id, c);
	});
}
