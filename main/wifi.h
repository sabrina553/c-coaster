#pragma once
#include <stdbool.h>
#include <stdint.h>

// Station-mode Wi-Fi using the SSID/password from Kconfig. Reconnects on drop.
void wifi_init(void);
// Blocks until an IP is assigned or timeout_ms passes; true if connected.
bool wifi_wait_connected(uint32_t timeout_ms);
