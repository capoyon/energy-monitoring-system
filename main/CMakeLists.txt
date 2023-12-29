idf_component_register(
    SRCS 
        "energy-monitoring-system.c"
        "../data/data.c"
        "../time/time.c"
        "../web/web.c"
        "../wifi/wifi.c"
    
    INCLUDE_DIRS 
        "../data"
        "../time"
        "../web"
        "../web/html"
        "../wifi"
)

