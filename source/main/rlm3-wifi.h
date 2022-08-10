#pragma once

#include "rlm3-base.h"
#include "rlm3-task.h"

#ifdef __cplusplus
extern "C" {
#endif


#define RLM3_WIFI_LINK_COUNT (5)


extern bool RLM3_WIFI_Init();
extern void RLM3_WIFI_Deinit();
extern bool RLM3_WIFI_IsInit();

extern bool RLM3_WIFI_GetVersion(uint32_t* at_version, uint32_t* sdk_version);

extern bool RLM3_WIFI_NetworkConnect(const char* ssid, const char* password);
extern void RLM3_WIFI_NetworkDisconnect();
extern bool RLM3_WIFI_IsNetworkConnected();

extern bool RLM3_WIFI_ServerConnect(size_t link_id, const char* server, const char* service);
extern void RLM3_WIFI_ServerDisconnect(size_t link_id);
extern bool RLM3_WIFI_IsServerConnected(size_t link_id);

extern bool RLM3_WIFI_Transmit(size_t link_id, const uint8_t* data, size_t size);
extern void RLM3_WIFI_Receive_Callback(size_t link_id, uint8_t data);


extern void SIM_RLM3_WIFI_InitFailure();
extern void SIM_RLM3_WIFI_SetVersion(uint32_t at_version, uint32_t sdk_version);
extern void SIM_RLM3_WIFI_SetNetwork(const char* ssid, const char* password);
extern void SIM_RLM3_WIFI_SetServer(size_t link_id, const char* server, const char* service);
extern void SIM_RLM3_WIFI_Transmit(size_t link_id, const char* expected);
extern void SIM_RLM3_WIFI_Receive(size_t link_id, const char* data);


#ifdef __cplusplus
}
#endif
