                       APPLICATIONS
                  Chrome / Teams / etc.
                            │
                            ▼
                     ┌────────────┐
                     │   TUN0     │
                     └─────┬──────┘
                           │
                           ▼
                 ┌───────────────────┐
                 │   PACKET ENGINE    │
                 │                   │
                 │ sequence numbers  │
                 │ fragmentation     │
                 │ duplication       │
                 │ reordering        │
                 │ retransmission    │
                 └─────────┬─────────┘
                           │
                           ▼
                 ┌───────────────────┐
                 │     SCHEDULER     │
                 └─────────┬─────────┘
                           │
          ┌────────────────┼────────────────┐
          │                │                │
          ▼                ▼                ▼
       PATH #1          PATH #2          PATH #3
       wlan0            wlan1            wlan2
          │                │                │
       ISP #1           ISP #2           ISP #3
          │                │                │
          └────────────────┼────────────────┘
                           │
                           ▼
                      AGGREGATOR
                         VPS
                           │
                           ▼
                       INTERNET





                        MONITOR
                            │
             ┌──────────────┼──────────────┐
             ▼              ▼              ▼
           PATH1          PATH2          PATH3
             │              │              │
             ▼              ▼              ▼
          metrics        metrics        metrics
             │              │              │
             └──────────────┼──────────────┘
                            ▼
                        SCHEDULER




   Chrome
   │
   ▼
tun0
   │
   ▼
your C++ program
   │
   ├── path 1
   ├── path 2
   ├── path 3
   └── path 4



                       WifiManager
                         │
                         ▼
                    libnm
                         │
                         ▼
                 NetworkManager
                         │
                         ▼
                  wpa_supplicant
                         │
                         ▼
                      nl80211
                         │
                         ▼
                   Linux kernel
                         │
                         ▼
                    Wi-Fi driver
                         │
                         ▼
                      hardware




                       WifiManager
                              │
             ┌────────────────┼────────────────┐
             │                │                │
             ▼                ▼                ▼
        NMClient          NMDevice        NetworkManager
             │                │
             │                ▼
             │          NMDeviceWifi
             │                │
             │                ▼
             │         NMAccessPoint
             │                │
             │       ┌────────┼─────────┐
             │       ▼        ▼         ▼
             │      SSID     BSSID    Signal
             │
             ▼
        saved profiles