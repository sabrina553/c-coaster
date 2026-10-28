# IDF v6 split "driver" into per-peripheral components; PRIV_REQUIRES keeps them private to main.
idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES esp_driver_gpio)
