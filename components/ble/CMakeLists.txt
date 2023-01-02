set(srcs "gap.c" "gatt.c" "ble.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "include"
                    REQUIRES bt nimble_peripheral_utils ssd1306 crsf
                    PRIV_REQUIRES )