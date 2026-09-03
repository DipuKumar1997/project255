
// #include<Network
#include <NetworkManager.h>
typedef struct _NMClient NMClient;
class WifiManager
{
private:
    NMClient* client;
public:
    WifiManager(/* args */);
    ~WifiManager();
    void listOfDevices();
    void scan(NMDeviceWifi* wifi);
};

// WifiManager::WifiManager(/* args */)
// {
// }

// WifiManager::~WifiManager()
// {
// }
