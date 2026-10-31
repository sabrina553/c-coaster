# IDF v6 split "driver" into per-peripheral components; PRIV_REQUIRES keeps them private to main.
idf_component_register(SRCS "main.c" "mcp3008.c" "stats.c" "interpolation.c"
                            "wifi.c" "time_sync.c" "thingspeak.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES esp_driver_gpio esp_wifi esp_netif esp_event nvs_flash
                                     esp_http_client esp_timer mbedtls)
