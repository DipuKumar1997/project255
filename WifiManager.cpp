
#include"WifiManager.hpp"
#include<iostream>
#include<NetworkManager.h>
#include <nm-access-point.h>


WifiManager::WifiManager(){
    /*
    struct _GError {
        GQuark       domain;
        gint         code;
        gchar       *message;
    };
    */
    GError* error=nullptr;
    /*
    struct _GCancellable{
        GObject parent_instance;

        GCancellablePrivate *priv;
    };
    */

    // It creates and returns a fully initialized NMClient object.
    client= nm_client_new(nullptr,&error);

    if(!client){
        if (error){
            std::cerr<<"Networkmanager error: "<<error->message<<'\n';
            g_error_free(error);
        }
        throw std::runtime_error("could not connect to Networkmanager");
    }
} 
WifiManager::~WifiManager(){
    if(client){
        g_object_unref(client);
    }
}

void WifiManager::scan(NMDeviceWifi* wifi){
    GError* error = nullptr;
    nm_device_wifi_request_scan(wifi,nullptr,&error);

    if(error){
        std::cerr<<"scan requested failed: "<<error->message<<'\n';
        g_error_free(error);
        return;
    }
    std::cout << "Scan requested successfully.\n";
}
void WifiManager::listOfDevices(){
    /*
    struct _GPtrArray{
        gpointer *pdata;
        guint	    len;
    };

    GPtrArray
    │
    ├── pdata ───────────────┐
    │                        │
    └── len = 4              │
                            ▼
                    ┌─────────────┐
    pdata[0] ─────────►│ NMDevice*   │
    pdata[1] ─────────►│ NMDevice*   │
    pdata[2] ─────────►│ NMDevice*   │
    pdata[3] ─────────►│ NMDevice*   │
                    └─────────────┘
    */


    const GPtrArray* devices=nm_client_get_devices(client);

    for(guint i=0;i<devices->len;++i){
        // Give me the pointer stored at position i.
        // Treat that pointer as an NMDevice*.

        /*
        
                           NMDevice
                 │
                    ┌────────┼─────────┐
                    │        │         │
                Ethernet   Wi-Fi    Bridge
                    │        │
                NMDevice  NMDeviceWifi
                            │
                            └── Wi-Fi specific functions

                            

    NMDevice* device       is the generic object.

                        If it represents Wi-Fi, we can treat it as:

                        NMDeviceWifi*
        */
        NMDevice* device = static_cast<NMDevice*>(g_ptr_array_index(devices,i));

        if(!NM_IS_DEVICE_WIFI(device)) continue;
        // NMDevice*
        // │
        // │ cast
        // ▼
        // NMDeviceWifi*

        /*
        wlan0
        │
        ▼
        NMDeviceWifi
        │
        ▼
        Access Point list
        │
        ├── AP 1 (College air)
        ├── AP 2 
        ├── AP 3
        └── AP 4

             NetworkManager
                    │
             saved profiles
                    │
       ┌────────────┼────────────┐
       ▼            ▼            ▼
   Profile A    Profile B    Profile C
       │            │            │
     SSID         SSID         SSID
     UUID         UUID         UUID
   credentials  credentials  credentials
        */
        NMDeviceWifi* wifi = NM_DEVICE_WIFI(device);

        


        // iface in the function nm_device_get_iface stands for interface.
        // When you call nm_device_get_iface, it returns a string containing the kernel network interface name.


        // eth0 or enp3s0 for a wired Ethernet connection.
        //wlan0 or wlp2s0 for a wireless Wi-Fi connection.
        // lo for the local loopback interface.
        
        /*
        if (NM_IS_DEVICE_WIFI(device))
            std::cout << " : Wi-Fi";
        */
        

        const char* iface=nm_device_get_iface(device);

        //Return Value: It returns a literal string owned by the library 
        
        // (e.g., "ethernet", "wifi", "infiniBand", "bridge").
        const char* type=nm_device_get_type_description(device);


        // The full form of hw address in nm_device_get_hw_address stands for hardware address.
        //In networking and NetworkManager, this refers to your device's physical MAC address (Media Access Control address) which uniquely identifies a piece of network hardware on a local network.

        const char* mac = nm_device_get_hw_address(device);

        std::cout<<iface<<" : "<<type<<" mac "<<(mac?mac:"Unknown")<<'\n';
        std::cout << "----------------------\n";


        const GPtrArray* aps=nm_device_wifi_get_access_points(wifi);
        std::cout<<"Access points found: "<<aps->len<<'\n';
        for(guint j=0;j<aps->len ;++j){
            // NMAccessPoint* ap=static_cast<NMAccessPoint*>g_ptr_array_index(aps,j);
            // NMAccessPoint* ap=reinterpret_cast<NMAccessPoint*>g_ptr_array_index(aps,j);
            NMAccessPoint* ap = NM_ACCESS_POINT(g_ptr_array_index(aps, j));

            std::cout<<" AP #"<<j<<'\n';

            //  char* ssid_str = nm_utils_ssid_to_utf8(ssid);
             GBytes* ssid= nm_access_point_get_ssid(ap);

            if (ssid){
                gsize size=0;
                const guint8* data = reinterpret_cast<const guint8*>(g_bytes_get_data(ssid , &size));

                std::string ssidString(reinterpret_cast<const char* >(data ),size);
                 std::cout << "       SSID : " << ssidString << '\n';

            }
            const char* bssid =  nm_access_point_get_bssid(ap);

            std::cout << "      BSSID: "  << (bssid ? bssid : "unknown")  << '\n';

            std::cout << "      Signal: " << nm_access_point_get_strength(ap) << "%\n";

            std::cout << "      Frequency: " << nm_access_point_get_frequency(ap) << " MHz\n";
        }

    }
}
int main(int argc, char const *argv[])
{   
    WifiManager wifi;
    wifi.listOfDevices();
    
    return 0;
}

/*

     C++ PROGRAM
       │
       │ libnm API
       ▼
   NMClient
       │
       ▼
            NetworkManager daemon
            NMClient isn't  Wi-Fi card.

            It is  C++ handle to NetworkManager.
*/