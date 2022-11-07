set(srcs "main.cpp"
         "common.cpp"
         "gap.cpp"
         "gatt.cpp")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ".")

#hier muss kein required oder ähnliches rein, weil main alles required