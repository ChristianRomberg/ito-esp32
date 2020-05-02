set(srcs "main.cpp"
        "bluetooth.cpp")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ".")
