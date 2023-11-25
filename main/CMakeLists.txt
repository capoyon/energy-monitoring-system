idf_component_register(
    SRCS 
        "energy-monitoring-system.c"
        "../storage/storage.c"
        "../time/time.c"
        "../web/web.c"
        "../wifi/wifi.c"
    
    INCLUDE_DIRS 
        "../storage"
        "../time"
        "../web"
        "../wifi"
)

