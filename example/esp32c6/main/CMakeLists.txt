set(srcs
    "esp32c6.c"
    "../../../src/ssd1680.c"
    "../../epaper-display/src/epd.c"
    )

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS
                        "."
                        "../../../src"
                        "../../epaper-display/src")
