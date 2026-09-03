
'
    const GPtrArray* aps =
        nm_device_wifi_get_access_points(wifi);

'

nl80211 is a netlink-based programming interface in the Linux kernel used to configure and control wireless 802.11 network hardware.

             C++ PROGRAM
                  │
                  ▼
       request Wi-Fi scan
                  │
                  ▼
          NetworkManager
                  │
                  ▼
              nl80211
                  │
                  ▼
          Linux Wi-Fi stack
                  │
                  ▼
              driver
                  │
                  ▼
               radio
                  │
        ┌─────────┼─────────┐
        ▼         ▼         ▼
       AP1       AP2       AP3
        │         │         │
        └─────────┼─────────┘
                  │
                  ▼
          scan results
                  │
                  ▼
          NetworkManager
                  │
                  ▼
       get_access_points()

       nm_device_wifi_request_scan()



t = 0 ms
C++
 │
 ├── request scan ──────────────► NetworkManager
 │
 │                               │
 │                               ▼
 │                           Wi-Fi driver
 │                               │
 │                               ▼
 │                          scan the radio
 │                               │
 │                               ▼
 │                          collect results
 │                               │
 │◄──────────── later ───────────┘
 │
 ▼
scan completed
 │
 ▼
get_access_points()



1. Linux Wi-Fi architecture
        ↓
2. 802.11 association
        ↓
3. One radio / one channel
        ↓
4. Virtual Wi-Fi interfaces
        ↓
5. mac80211 / cfg80211
        ↓
6. Concurrent interface modes
        ↓
7. Multiple BSS/STA possibilities
        ↓
8. Network namespaces / routing
        ↓
9. Multiple IP paths
        ↓
10. TUN
        ↓
11. Multipath transport
        ↓
12. Failover


              ONE PHYSICAL RADIO
                      │
                      ▼
             ┌─────────────────┐
             │ SOFTWARE LAYER  │
             │                 │
             │ virtual links   │
             │ path monitor    │
             │ scheduler       │
             │ multipath       │
             └────────┬────────┘
                      │
             ┌────────┼────────┐
             ▼        ▼        ▼
            AP-A     AP-B     AP-C
             │        │        │
            ISP-A    ISP-B    ISP-C


iw dev
iw phy
iw list

phy
interface
managed mode
AP mode
channel
interface combinations