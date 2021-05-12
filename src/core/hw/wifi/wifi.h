#pragma once

#include <common/types.h>
#include <common/log.h>

struct Wifi {
    void Reset();

    u16 W_TXREQ_SET;
    u16 W_MODE_WEP;
    u16 W_MACADDR_0;
    u16 W_MACADDR_1;
    u16 W_MACADDR_2;
    u16 W_BSSID_0;
    u16 W_BSSID_1;
    u16 W_BSSID_2;
    u16 W_AID_FULL;
    u16 W_RXBUF_BEGIN;
    u16 W_RXBUF_END;
    u16 W_RXBUF_WR_ADDR;
};