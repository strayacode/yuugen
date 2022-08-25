#include "Core/ARM/ARM9/Coprocessor.h"
#include "Core/ARM/CPU.h"

ARM9Coprocessor::ARM9Coprocessor(CPU& cpu) : m_cpu(cpu) {}

void ARM9Coprocessor::reset() {
    m_control = 0;
    m_itcm_control = 0;
    m_dtcm_control = 0;

    m_itcm.fill(0);
    m_dtcm.fill(0);
}

void ARM9Coprocessor::direct_boot() {
    // configure control registers
    write(1, 0, 0, 0x0005707D);
    write(9, 1, 0, 0x0300000A);
    write(9, 1, 1, 0x00000020);
}

u32 ARM9Coprocessor::get_exception_base() {
    return (m_control & (1 << 13)) ? 0xFFFF0000 : 0;
}

u32 ARM9Coprocessor::read(u32 cn, u32 cm, u32 cp) {
    switch ((cn << 16) | (cm << 8) | cp) {
    case 0x000001:
        return 0x0F0D2112; 
    case 0x020000:
    case 0x020001:
    case 0x030000:
    case 0x050000:
    case 0x050001:
    case 0x050002:
    case 0x050003:
    case 0x060000:
    case 0x060100:
    case 0x060200:
    case 0x060300:
    case 0x060400:
    case 0x060500:
    case 0x060600:
    case 0x060700:
    case 0x010000:
        return m_control;
    case 0x090100:
        return m_dtcm_control;
    case 0x090101:
        return m_itcm_control;
    default:
        log_fatal("[ARM9Coprocessor] undefined register read c%d, c%d, c%d", cn, cm, cp);
    }
}

void ARM9Coprocessor::write(u32 cn, u32 cm, u32 cp, u32 data) {
    switch ((cn << 16) | (cm << 8) | cp) {
    case 0x010000:
        m_control = data;

        // update the arm9 memory map as r/w permissions may have changed
        m_cpu.memory().update_memory_map(itcm_base(), itcm_size());
        m_cpu.memory().update_memory_map(dtcm_base(), dtcm_base() + dtcm_size());
        break;
    case 0x020000:
    case 0x020001:
    case 0x030000:
    case 0x050002:
    case 0x050003:
    case 0x060000:
    case 0x060100:
    case 0x060200:
    case 0x060300:
    case 0x060400:
    case 0x060500:
    case 0x060600:
    case 0x060700:
        break;
    case 0x070004:
        m_cpu.halt();
        break;
    case 0x070500:
    case 0x070501:
    case 0x070600:
    case 0x070601:
    case 0x070602:
    case 0x070A01:
    case 0x070A02:
    case 0x070A04:
    case 0x070E01:
    case 0x070E02:
        break;
    case 0x090100:
        // unmap old dtcm region
        m_cpu.memory().update_memory_map(dtcm_base(), dtcm_base() + dtcm_size());
        
        m_dtcm_control = data;

        // remap with new dtcm region
        m_cpu.memory().update_memory_map(dtcm_base(), dtcm_base() + dtcm_size());

        log_debug("[ARM9Coprocessor] dtcm base: %08x dtcm size: %08x", dtcm_base(), dtcm_size());
        break;
    case 0x090101: {
        // unmap old itcm region
        m_cpu.memory().update_memory_map(itcm_base(), itcm_base() + itcm_size());
        
        m_itcm_control = data;

        // remap with new itcm region
        m_cpu.memory().update_memory_map(itcm_base(), itcm_base() + itcm_size());

        log_debug("[ARM9Coprocessor] itcm base: %08x itcm size: %08x", itcm_base(), itcm_size());
        break;
    default:
        log_fatal("[ARM9Coprocessor] undefined register write c%d, c%d, c%d with data %08x", cn, cm, cp, data);
    }
}

u32 ARM9Coprocessor::itcm_base() {
    return 0;
}

u32 ARM9Coprocessor::itcm_size() {
   return 512 << ((m_itcm_control >> 1) & 0x1F); 
}

u32 ARM9Coprocessor::dtcm_base() {
    return ((m_dtcm_control >> 12) & 0xFFFFF) << 12;
}

u32 ARM9Coprocessor::dtcm_size() {
   return 512 << (m_dtcm_control >> 1); 
}

bool ARM9Coprocessor::itcm_is_readable() {
    return !(m_control & (1 << 19)) && (m_control & (1 << 18));
}

bool ARM9Coprocessor::itcm_is_writeable() {
    return (m_control & (1 << 18));
}

bool ARM9Coprocessor::dtcm_is_readable() {
    return !(m_control & (1 << 17)) && (m_control & (1 << 16));
}

bool ARM9Coprocessor::dtcm_is_writeable() {
    return (m_control & (1 << 16));
}
