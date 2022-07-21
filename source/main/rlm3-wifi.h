#pragma once

#include "rlm3-base.h"
#include "rlm3-task.h"

#ifdef __cplusplus
extern "C" {
#endif


extern bool RLM3_WIFI_Init();
extern void RLM3_WIFI_Deinit();
extern bool RLM3_WIFI_IsInit();

extern bool RLM3_WIFI_GetVersion(uint32_t* at_version, uint32_t* sdk_version);

extern bool RLM3_WIFI_NetworkConnect(const char* ssid, const char* password);
extern void RLM3_WIFI_NetworkDisconnect();
extern bool RLM3_WIFI_IsNetworkConnected();

extern bool RLM3_WIFI_ServerConnect(const char* server, const char* service);
extern void RLM3_WIFI_ServerDisconnect();
extern bool RLM3_WIFI_IsServerConnected();

extern bool RLM3_WIFI_Transmit(const uint8_t* data, size_t size);
extern void RLM3_WIFI_Receive_Callback(uint8_t data);


extern void SIM_RLM3_WIFI_InitFailure();
extern void SIM_RLM3_WIFI_SetVersion(uint32_t at_version, uint32_t sdk_version);
extern void SIM_RLM3_WIFI_SetNetwork(const char* ssid, const char* password);
extern void SIM_RLM3_WIFI_SetServer(const char* server, const char* service);
extern void SIM_RLM3_WIFI_Transmit(const char* expected);
extern void SIM_RLM3_WIFI_Receive(const char* data);


#ifdef __cplusplus
}
#endif
