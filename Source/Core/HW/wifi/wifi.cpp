#include "Core/HW/wifi/wifi.h"

void Wifi::Reset() {
    W_TXREQ_SET = 0;
    W_MODE_WEP = 0;
    W_MACADDR_0 = 0;
    W_MACADDR_1 = 0;
    W_MACADDR_2 = 0;
    W_BSSID_0 = 0;
    W_BSSID_1 = 0;
    W_BSSID_2 = 0;
    W_AID_FULL = 0;
    W_RXBUF_BEGIN = 0;
    W_RXBUF_END = 0;
    W_RXBUF_WR_ADDR = 0;
}