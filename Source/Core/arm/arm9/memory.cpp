#include <cassert>
#include "Common/Log.h"
#include "Common/Memory.h"
#include "Core/arm/arm9/memory.h"
#include "Core/Core.h"

ARM9Memory::ARM9Memory(System& system) : system(system) {
    bios = LoadBios<0x8000>("../bios/bios9.bin");
}

void ARM9Memory::Reset() {
    UpdateMemoryMap(0, 0xFFFFFFFF);
}

void ARM9Memory::UpdateMemoryMap(u32 low_addr, u32 high_addr) {
    // for mapping itcm and dtcm:
    // itcm has higher priority than dtcm
    // do UpdateARM9MemoryMap for old itcm and dtcm range, this will make the pages now not in the range
    // as nullptr. then do another UpdateARM9MemoryMap to make sure the itcm and range is fully covered
    for (u64 addr = low_addr; addr < high_addr; addr += 0x1000) {
        // get the pagetable index
        int index = addr >> 12;

        if (system.cp15.GetITCMReadEnabled() && (addr < system.cp15.GetITCMSize())) {
            read_page_table[index] = &system.cp15.itcm[addr & 0x7FFF];
        } else if (system.cp15.GetDTCMReadEnabled() && Common::in_range(system.cp15.GetDTCMBase(), system.cp15.GetDTCMBase() + system.cp15.GetDTCMSize(), addr)) {
            read_page_table[index] = &system.cp15.dtcm[(addr - system.cp15.GetDTCMBase()) & 0x3FFF];
        } else {
            switch (addr >> 24) {
            case 0x02:
                read_page_table[index] = &system.main_memory[addr & 0x3FFFFF];
                break;
            case 0x03:
                switch (system.wramcnt) {
                case 0:
                    read_page_table[index] = &system.shared_wram[addr & 0x7FFF];
                    break;
                case 1:
                    read_page_table[index] = &system.shared_wram[(addr & 0x3FFF) + 0x4000];
                    break;
                case 2:
                    read_page_table[index] = &system.shared_wram[addr & 0x3FFF];
                    break;
                case 3:
                    // just set to a nullptr to indicate we should return 0
                    read_page_table[index] = nullptr;
                    break;
                }
                break;
            case 0xFF:
                if ((addr & 0xFFFF0000) == 0xFFFF0000) {
                    read_page_table[index] = &bios[addr & 0x7FFF];
                } else {
                    read_page_table[index] = nullptr;
                }

                break;
            default:
                // set as a nullptr, which indicates that we should do a regular read / write
                read_page_table[index] = nullptr;
                break;
            }
        }
    }

    for (u64 addr = low_addr; addr < high_addr; addr += 0x1000) {
        // get the pagetable index
        int index = addr >> 12;

        if (system.cp15.GetITCMWriteEnabled() && (addr < system.cp15.GetITCMSize())) {
            write_page_table[index] = &system.cp15.itcm[addr & 0x7FFF];
        } else if (system.cp15.GetDTCMWriteEnabled() && Common::in_range(system.cp15.GetDTCMBase(), system.cp15.GetDTCMBase() + system.cp15.GetDTCMSize(), addr)) {
            write_page_table[index] = &system.cp15.dtcm[(addr - system.cp15.GetDTCMBase()) & 0x3FFF];
        } else {
            switch (addr >> 24) {
            case 0x02:
                write_page_table[index] = &system.main_memory[addr & 0x3FFFFF];
                break;
            case 0x03:
                switch (system.wramcnt) {
                case 0:
                    write_page_table[index] = &system.shared_wram[addr & 0x7FFF];
                    break;
                case 1:
                    write_page_table[index] = &system.shared_wram[(addr & 0x3FFF) + 0x4000];
                    break;
                case 2:
                    write_page_table[index] = &system.shared_wram[addr & 0x3FFF];
                    break;
                case 3:
                    // just set to a nullptr to indicate we should return 0
                    write_page_table[index] = nullptr;
                    break;
                }
                break;
            default:
                // set as a nullptr, which indicates that we should do a regular read / write
                write_page_table[index] = nullptr;
                break;
            }
        }
    }
}

u8 ARM9Memory::ReadByte(u32 addr) {
    switch (addr >> 24) {
    case 0x04:
        switch (addr) {
        case 0x040001A8:
        case 0x040001A9:
        case 0x040001AA:
        case 0x040001AB:
        case 0x040001AC:
        case 0x040001AD:
        case 0x040001AE:
        case 0x040001AF:
            // recieve a cartridge command and store in the buffer
            return system.cartridge.ReadCommand(addr - 0x040001A8);
        case 0x04000208:
            return system.cpu_core[1].ime & 0x1;
        case 0x04000240:
            return system.video_unit.vram.vramcnt[0];
        case 0x04000241:
            return system.video_unit.vram.vramcnt[1];
        case 0x04000242:
            return system.video_unit.vram.vramcnt[2];
        case 0x04000243:
            return system.video_unit.vram.vramcnt[3];
        case 0x04000244:
            return system.video_unit.vram.vramcnt[4];
        case 0x04000245:
            return system.video_unit.vram.vramcnt[5];
        case 0x04000246:
            return system.video_unit.vram.vramcnt[6];
        case 0x04000248:
            return system.video_unit.vram.vramcnt[7];
        case 0x04000249:
            return system.video_unit.vram.vramcnt[8];
        case 0x04000300:
            return system.POSTFLG9;
        case 0x04004000:
            return 0;
        default:
            log_fatal("[ARM9] Undefined 8-bit io read %08x", addr);
        }
    case 0x05:
        return Common::read<u8>(system.video_unit.get_palette_ram(), addr & 0x7FF);
    case 0x06:
        return system.video_unit.vram.read_vram<u8>(addr);
    case 0x07:
        return Common::read<u8>(system.video_unit.get_oam(), addr & 0x7FF);
    case 0x08: case 0x09:
        // check if the arm9 has access rights to the gba slot
        // if not return 0
        if (system.EXMEMCNT & (1 << 7)) {
            return 0;
        }
        // otherwise return openbus (0xFFFFFFFF)
        return 0xFF;
    default:
        log_fatal("handle byte read from %08x", addr);
    }
}

u16 ARM9Memory::ReadHalf(u32 addr) {
    switch (addr >> 24) {
    case 0x03:
        switch (system.wramcnt) {
        case 0:
            return Common::read<u16>(&system.shared_wram[addr & 0x7FFF], 0);
        case 1:
            return Common::read<u16>(&system.shared_wram[(addr & 0x3FFF) + 0x4000], 0);
        case 2:
            return Common::read<u16>(&system.shared_wram[addr & 0x3FFF], 0);
        case 3:
            return 0;
        }
        break;
    case 0x04:
        switch (addr) {
        case 0x04000004:
            return system.video_unit.dispstat[1];
        case 0x04000006:
            return system.video_unit.vcount;
        case 0x04000060:
            return system.video_unit.renderer_3d->disp3dcnt;
        case 0x040000BA:
            return system.dma[1].ReadDMACNT_H(0);
        case 0x040000C6:
            return system.dma[1].ReadDMACNT_H(1);
        case 0x040000D2:
            return system.dma[1].ReadDMACNT_H(2);
        case 0x040000DE:
            return system.dma[1].ReadDMACNT_H(3);
        case 0x040000EC:
            return system.dma[1].DMAFILL[3] & 0xFFFF;
        case 0x04000100:
            return system.timers[1].read_counter(0);
        case 0x04000104:
            return system.timers[1].read_counter(1);
        case 0x04000108:
            return system.timers[1].read_counter(2);
        case 0x0400010C:
            return system.timers[1].read_counter(3);
        case 0x04000130:
            return system.input.KEYINPUT;
        case 0x04000180:
            return system.ipc.ipcsync[1].data;
        case 0x04000184:
            return system.ipc.ipcfifocnt[1].data;
        case 0x040001A0:
            return system.cartridge.AUXSPICNT;
        case 0x040001A2:
            return system.cartridge.AUXSPIDATA;
        case 0x04000204:
            return system.EXMEMCNT;
        case 0x04000208:
            return system.cpu_core[1].ime & 0x1;
        case 0x04000280:
            return system.maths_unit.DIVCNT;
        case 0x040002B0:
            return system.maths_unit.SQRTCNT;
        case 0x04000300:
            return system.POSTFLG9;
        case 0x04000304:
            return system.video_unit.powcnt1;
        case 0x04000320:
            return 0;
        case 0x04004004:
            return 0;
        case 0x04004010:
            return 0;
        }

        break;
    case 0x05:
        return Common::read<u16>(system.video_unit.get_palette_ram(), addr & 0x7FF);
    case 0x06:
        return system.video_unit.vram.read_vram<u16>(addr);
    case 0x07:
        return Common::read<u16>(system.video_unit.get_oam(), addr & 0x7FF);
    case 0x08: case 0x09:
        // check if the arm9 has access rights to the gba slot
        // if not return 0
        if (system.EXMEMCNT & (1 << 7)) {
            return 0;
        }
        // otherwise return openbus (0xFFFFFFFF)
        return 0xFFFF;
    default:
        log_fatal("handle half read from %08x", addr);
        break;
    }

    if (Common::in_range(0x04000000, 0x04000060, addr)) {
        return system.video_unit.renderer_2d[0]->read_half(addr);
    }

    if (Common::in_range(0x04001000, 0x04001060, addr)) {
        return system.video_unit.renderer_2d[1]->read_half(addr);
    }

    log_warn("ARM9: handle half read %08x", addr);

    return 0;
}

u32 ARM9Memory::ReadWord(u32 addr) {
    switch (addr >> 24) {
    case 0x03:
        switch (system.wramcnt) {
        case 0:
            return Common::read<u32>(&system.shared_wram[addr & 0x7FFF], 0);
        case 1:
            return Common::read<u32>(&system.shared_wram[(addr & 0x3FFF) + 0x4000], 0);
        case 2:
            return Common::read<u32>(&system.shared_wram[addr & 0x3FFF], 0);
        case 3:
            return 0;
        }
        break;
    case 0x04:
        switch (addr) {
        case 0x040000B0:
            return system.dma[1].channel[0].source;
        case 0x040000B4:
            return system.dma[1].channel[0].destination;
        case 0x040000B8:
            return system.dma[1].ReadDMACNT(0);
        case 0x040000BC:
            return system.dma[1].channel[1].source;
        case 0x040000C0:
            return system.dma[1].channel[1].destination;
        case 0x040000C4:
            return system.dma[1].ReadDMACNT(1);
        case 0x040000C8:
            return system.dma[1].channel[2].source;
        case 0x040000CC:
            return system.dma[1].channel[2].destination;
        case 0x040000D0:
            return system.dma[1].ReadDMACNT(2);
        case 0x040000D4:
            return system.dma[1].channel[3].source;
        case 0x040000D8:
            return system.dma[1].channel[3].destination;
        case 0x040000DC:
            return system.dma[1].ReadDMACNT(3);
        case 0x040000E0:
            return system.dma[1].DMAFILL[0];
        case 0x040000E4:
            return system.dma[1].DMAFILL[1];
        case 0x040000E8:
            return system.dma[1].DMAFILL[2];
        case 0x040000EC:
            return system.dma[1].DMAFILL[3];
        case 0x04000100:
            return (system.timers[1].read_control(0) << 16) | (system.timers[1].read_counter(0));
        case 0x04000180:
            return system.ipc.ipcsync[1].data;
        case 0x040001A4:
            return system.cartridge.ROMCTRL;
        case 0x04000208:
            return system.cpu_core[1].ime & 0x1;
        case 0x04000210:
            return system.cpu_core[1].ie;
        case 0x04000214:
            return system.cpu_core[1].irf;
        case 0x04000240:
            return ((system.video_unit.vram.vramcnt[3] << 24) | (system.video_unit.vram.vramcnt[2] << 16) | (system.video_unit.vram.vramcnt[1] << 8) | (system.video_unit.vram.vramcnt[0]));
        case 0x04000280:
            return system.maths_unit.DIVCNT;
        case 0x04000290:
            return system.maths_unit.DIV_NUMER & 0xFFFFFFFF;
        case 0x04000294:
            return system.maths_unit.DIV_NUMER >> 32;
        case 0x04000298:
            return system.maths_unit.DIV_DENOM & 0xFFFFFFFF;
        case 0x0400029C:
            return system.maths_unit.DIV_DENOM >> 32;
        case 0x040002A0:
            return system.maths_unit.DIV_RESULT & 0xFFFFFFFF;
        case 0x040002A4:
            return system.maths_unit.DIV_RESULT >> 32;
        case 0x040002A8:
            return system.maths_unit.DIVREM_RESULT & 0xFFFFFFFF;
        case 0x040002AC:
            return system.maths_unit.DIVREM_RESULT >> 32;
        case 0x040002B4:
            return system.maths_unit.SQRT_RESULT;
        case 0x040002B8:
            return system.maths_unit.SQRT_PARAM & 0xFFFFFFFF;
        case 0x040002BC:
            return system.maths_unit.SQRT_PARAM >> 32;
        case 0x04004000:
            return 0;
        case 0x04004008:
            return 0;
        case 0x04100000:
            return system.ipc.read_ipcfiforecv(1);
        case 0x04100010:
            return system.cartridge.ReadData();
        }

        break;
    case 0x05:
        return Common::read<u32>(system.video_unit.get_palette_ram(), addr & 0x7FF);
    case 0x06:
        return system.video_unit.vram.read_vram<u32>(addr);
    case 0x07:
        return Common::read<u16>(system.video_unit.get_oam(), addr & 0x7FF);
    case 0x08: case 0x09:
        // check if the arm9 has access rights to the gba slot
        // if not return 0
        if (system.EXMEMCNT & (1 << 7)) {
            return 0;
        }
        // otherwise return openbus (0xFFFFFFFF)
        return 0xFFFFFFFF;
    case 0x0A:
        return 0;
    }

    if (Common::in_range(0x04000000, 0x04000060, addr)) {
        return system.video_unit.renderer_2d[0]->read_word(addr);
    }

    if (Common::in_range(0x04000320, 0x040006A3, addr)) {
        return system.video_unit.renderer_3d->read_word(addr);
    }

    if (Common::in_range(0x04001000, 0x04001060, addr)) {
        return system.video_unit.renderer_2d[1]->read_word(addr);
    }

    log_warn("ARM9: handle word read %08x", addr);

    return 0;
}

void ARM9Memory::WriteByte(u32 addr, u8 data) {
    switch (addr >> 24) {
    case 0x04:
        switch (addr) {
        case 0x040001A1:
            // write to the high byte of AUXSPICNT
            system.cartridge.AUXSPICNT = (system.cartridge.AUXSPICNT & 0xFF) | (data << 8);
            return;
        case 0x040001A8:
        case 0x040001A9:
        case 0x040001AA:
        case 0x040001AB:
        case 0x040001AC:
        case 0x040001AD:
        case 0x040001AE:
        case 0x040001AF:
            // recieve a cartridge command and store in the buffer
            system.cartridge.ReceiveCommand(data, addr - 0x040001A8);
            return;
        case 0x04000208:
            system.cpu_core[1].ime = data & 0x1;
            return;
        case 0x04000240:
            system.video_unit.vram.update_vram_mapping(VRAM::Bank::A, data);
            return;
        case 0x04000241:
            system.video_unit.vram.update_vram_mapping(VRAM::Bank::B, data);
            return;
        case 0x04000242:
            system.video_unit.vram.update_vram_mapping(VRAM::Bank::C, data);
            return;
        case 0x04000243:
            system.video_unit.vram.update_vram_mapping(VRAM::Bank::D, data);
            return;
        case 0x04000244:
            system.video_unit.vram.update_vram_mapping(VRAM::Bank::E, data);
            return;
        case 0x04000245:
            system.video_unit.vram.update_vram_mapping(VRAM::Bank::F, data);
            return;
        case 0x04000246:
            system.video_unit.vram.update_vram_mapping(VRAM::Bank::G, data);
            return;
        case 0x04000247:
            system.write_wramcnt(data);
            return;
        case 0x04000248:
            system.video_unit.vram.update_vram_mapping(VRAM::Bank::H, data);
            return;
        case 0x04000249:
            system.video_unit.vram.update_vram_mapping(VRAM::Bank::I, data);
            return;
        case 0x04000300:
            system.POSTFLG9 = data;
            return;
        }
        
        break;
    case 0x05:
        Common::write<u8>(system.video_unit.get_palette_ram(), addr & 0x7FF, data);
        return;
    case 0x06:
        system.video_unit.vram.write_vram<u8>(addr, data);
        return;
    }

    if (Common::in_range(0x04000000, 0x04000060, addr)) {
        system.video_unit.renderer_2d[0]->write_byte(addr, data);
        return;
    }

    if (Common::in_range(0x04001000, 0x04001060, addr)) {
        system.video_unit.renderer_2d[1]->write_byte(addr, data);
        return;
    }

    if (Common::in_range(0x04000320, 0x040006A3, addr)) {
        system.video_unit.renderer_3d->write_byte(addr, data);
        return;
    }

    log_warn("ARM9: handle byte write %08x = %02x", addr, data);
}

void ARM9Memory::WriteHalf(u32 addr, u16 data) {
    switch (addr >> 24) {
    case 0x04:
        switch (addr) {
        case 0x04000004:
            system.video_unit.dispstat[1] = data;
            return;
        case 0x04000060:
            system.video_unit.renderer_3d->disp3dcnt = data;
            return;
        case 0x0400006C:
            system.video_unit.renderer_2d[0]->master_bright = data;
            return;
        case 0x040000B8:
            system.dma[1].WriteDMACNT_L(0, data);
            return;
        case 0x040000BA:
            system.dma[1].WriteDMACNT_H(0, data);
            return;
        case 0x040000C4:
            system.dma[1].WriteDMACNT_L(1, data);
            return;
        case 0x040000C6:
            system.dma[1].WriteDMACNT_H(1, data);
            return;
        case 0x040000D0:
            system.dma[1].WriteDMACNT_L(2, data);
            return;
        case 0x040000D2:
            system.dma[1].WriteDMACNT_H(2, data);
            return;
        case 0x040000DC:
            system.dma[1].WriteDMACNT_L(3, data);
            return;
        case 0x040000DE:
            system.dma[1].WriteDMACNT_H(3, data);
            return;
        case 0x04000100:
            system.timers[1].write_counter(0, data);
            return;
        case 0x04000102:
            system.timers[1].write_control(0, data);
            return;
        case 0x04000104:
            system.timers[1].write_counter(1, data);
            return;
        case 0x04000106:
            system.timers[1].write_control(1, data);
            return;
        case 0x04000108:
            system.timers[1].write_counter(2, data);
            return;
        case 0x0400010A:
            system.timers[1].write_control(2, data);
            return;
        case 0x0400010C:
            system.timers[1].write_counter(3, data);
            return;
        case 0x0400010E:
            system.timers[1].write_control(3, data);
            return;
        case 0x04000130:
            system.input.KEYINPUT = data;
            return;
        case 0x04000180:
            system.ipc.write_ipcsync(1, data);
            return;
        case 0x04000184:
            system.ipc.write_ipcfifocnt(1, data);
            return;
        case 0x040001A0:
            system.cartridge.WriteAUXSPICNT(data);
            return;
        case 0x040001A2:
            system.cartridge.WriteAUXSPIDATA(data);
            return;
        case 0x04000204:
            system.EXMEMCNT = data;
            return;
        case 0x04000208:
            system.cpu_core[1].ime = data & 0x1;
            return;
        case 0x04000248:
            system.video_unit.vram.update_vram_mapping(VRAM::Bank::H, data & 0xFF);
            system.video_unit.vram.update_vram_mapping(VRAM::Bank::I, data >> 8);
            return;
        case 0x04000280:
            system.maths_unit.DIVCNT = data;
            system.maths_unit.StartDivision();
            return;
        case 0x040002B0:
            system.maths_unit.SQRTCNT = data;
            system.maths_unit.StartSquareRoot();
            return;
        case 0x04000300:
            system.POSTFLG9 = data;
            return;
        case 0x04000304:
            system.video_unit.powcnt1 = data;
            return;
        case 0x0400106C:
            system.video_unit.renderer_2d[1]->master_bright = data;
            return;
        }
        break;
    case 0x05:
        Common::write<u16>(system.video_unit.get_palette_ram(), addr & 0x7FF, data);
        return;
    case 0x06:
        system.video_unit.vram.write_vram<u16>(addr, data);
        return;
    case 0x07:
        Common::write<u16>(system.video_unit.get_oam(), addr & 0x7FF, data);
        return;
    case 0x08: case 0x09:
        // for now do nothing lol
        return;
    }

    if (Common::in_range(0x04000000, 0x04000060, addr)) {
        system.video_unit.renderer_2d[0]->write_half(addr, data);
        return;
    }

    if (Common::in_range(0x04000320, 0x040006A3, addr)) {
        system.video_unit.renderer_3d->write_half(addr, data);
        return;
    }

    if (Common::in_range(0x04001000, 0x04001060, addr)) {
        system.video_unit.renderer_2d[1]->write_half(addr, data);
        return;
    }

    log_warn("ARM9: handle half write %08x = %04x", addr, data);
}

void ARM9Memory::WriteWord(u32 addr, u32 data) {
    switch (addr >> 24) {
    case 0x04:
        switch (addr) {
        case 0x04000004:
            system.video_unit.dispstat[1] = data & 0xFFFF;
            system.video_unit.vcount = data >> 16;
            return;
        case 0x04000060:
            system.video_unit.renderer_3d->disp3dcnt = data;
            return;
        case 0x04000064:
            system.video_unit.dispcapcnt = data;
            return;
        case 0x040000B0:
            system.dma[1].channel[0].source = data;
            return;
        case 0x040000B4:
            system.dma[1].channel[0].destination = data;
            return;
        case 0x040000B8:
            system.dma[1].WriteDMACNT(0, data);
            return;
        case 0x040000BC:
            system.dma[1].channel[1].source = data;
            return;
        case 0x040000C0:
            system.dma[1].channel[1].destination = data;
            return;
        case 0x040000C4:
            system.dma[1].WriteDMACNT(1, data);
            return;
        case 0x040000C8:
            system.dma[1].channel[2].source = data;
            return;
        case 0x040000CC:
            system.dma[1].channel[2].destination = data;
            return;
        case 0x040000D0:
            system.dma[1].WriteDMACNT(2, data);
            return;
        case 0x040000D4:
            system.dma[1].channel[3].source = data;
            return;
        case 0x040000D8:
            system.dma[1].channel[3].destination = data;
            return;
        case 0x040000DC:
            system.dma[1].WriteDMACNT(3, data);
            return;
        case 0x040000E0:
            system.dma[1].DMAFILL[0] = data;
            return;
        case 0x040000E4:
            system.dma[1].DMAFILL[1] = data;
            return;
        case 0x040000E8:
            system.dma[1].DMAFILL[2] = data;
            return;
        case 0x040000EC:
            system.dma[1].DMAFILL[3] = data;
            return;
        case 0x040001A0:
            system.cartridge.WriteAUXSPICNT(data & 0xFFFF);
            system.cartridge.WriteAUXSPIDATA(data >> 16);
            return;
        case 0x040001A4:
            system.cartridge.WriteROMCTRL(data);
            return;
        case 0x040001A8:
            system.cartridge.ReceiveCommand(data & 0xFF, 0);
            system.cartridge.ReceiveCommand((data >> 8) & 0xFF, 1);
            system.cartridge.ReceiveCommand((data >> 16) & 0xFF, 2);
            system.cartridge.ReceiveCommand((data >> 24) & 0xFF, 3);
            return;
        case 0x040001AC:
            system.cartridge.ReceiveCommand(data & 0xFF, 4);
            system.cartridge.ReceiveCommand((data >> 8) & 0xFF, 5);
            system.cartridge.ReceiveCommand((data >> 16) & 0xFF, 6);
            system.cartridge.ReceiveCommand((data >> 24) & 0xFF, 7);
            return;
        case 0x04000180:
            system.ipc.write_ipcsync(1, data);
            return;
        case 0x04000188:
            system.ipc.write_send_fifo(1, data);
            return;
        case 0x04000208:
            system.cpu_core[1].ime = data & 0x1;
            return;
        case 0x04000210:
            system.cpu_core[1].ie = data;
            return;
        case 0x04000214:
            system.cpu_core[1].irf &= ~data;
            return;
        case 0x04000240:
            system.video_unit.vram.update_vram_mapping(VRAM::Bank::A, data & 0xFF);
            system.video_unit.vram.update_vram_mapping(VRAM::Bank::B, (data >> 8) & 0xFF);
            system.video_unit.vram.update_vram_mapping(VRAM::Bank::C, (data >> 16) & 0xFF);
            system.video_unit.vram.update_vram_mapping(VRAM::Bank::D, (data >> 24) & 0xFF);
            return;
        case 0x04000244:
            system.video_unit.vram.update_vram_mapping(VRAM::Bank::E, data & 0xFF);
            system.video_unit.vram.update_vram_mapping(VRAM::Bank::F, (data >> 8) & 0xFF);
            system.video_unit.vram.update_vram_mapping(VRAM::Bank::G, (data >> 16) & 0xFF);
            system.write_wramcnt((data >> 24) & 0xFF);
            return;
        case 0x04000280:
            system.maths_unit.DIVCNT = data;
            system.maths_unit.StartDivision();
            // assert here so we know which programs are broken when using this
            assert(true);
            return;
        case 0x04000290:
            // write to lower 32 bits of DIV_NUMER, starting a division
            system.maths_unit.DIV_NUMER = (system.maths_unit.DIV_NUMER & ~0xFFFFFFFF) | data;
            system.maths_unit.StartDivision();
            return;
        case 0x04000294:
            // write to upper 32 bits of DIV_NUMER, starting a division
            system.maths_unit.DIV_NUMER = (system.maths_unit.DIV_NUMER & 0xFFFFFFFF) | ((u64)data << 32);
            system.maths_unit.StartDivision();
            return;
        case 0x04000298:
            // write to lower 32 bits of DIV_DENOM, starting a division
            system.maths_unit.DIV_DENOM = (system.maths_unit.DIV_DENOM & ~0xFFFFFFFF) | data;
            system.maths_unit.StartDivision();
            return;
        case 0x0400029C:
            // write to upper 32 bits of DIV_DENOM, starting a division
            system.maths_unit.DIV_DENOM = (system.maths_unit.DIV_DENOM & 0xFFFFFFFF) | ((u64)data << 32);
            system.maths_unit.StartDivision();
            return;
        case 0x040002A0: case 0x040002A4: case 0x040002A8:
            return;
        case 0x040002B0:
            system.maths_unit.SQRTCNT = (system.maths_unit.SQRTCNT & ~0xFFFF) | data;
            system.maths_unit.StartSquareRoot();
            return;
        case 0x040002B8:
            // write to lower 32 bits of SQRT_PARAM
            system.maths_unit.SQRT_PARAM = (system.maths_unit.SQRT_PARAM & ~0xFFFFFFFF) | data;
            system.maths_unit.StartSquareRoot();
            return;
        case 0x040002BC:
            // write to upper 32 bits of SQRT_PARAM
            system.maths_unit.SQRT_PARAM = (system.maths_unit.SQRT_PARAM & 0xFFFFFFFF) | ((u64)data << 32);
            system.maths_unit.StartSquareRoot();
            return;
        case 0x04000304:
            system.video_unit.powcnt1 = data;
            return;
        case 0x04001004: case 0x04001060: case 0x04001064: case 0x04001068:
            return;
        case 0x0400106C:
            system.video_unit.renderer_2d[1]->master_bright = data;
            return;
        }
        
        break;
    case 0x05:
        Common::write<u32>(system.video_unit.get_palette_ram(), addr & 0x7FF, data);
        return;
    case 0x06:
        system.video_unit.vram.write_vram<u32>(addr, data);
        return;
    case 0x07:
        Common::write<u32>(system.video_unit.get_oam(), addr & 0x7FF, data);
        return;
    case 0x08: case 0x09:
        // for now do nothing lol
        return;
    }

    if (Common::in_range(0x04000000, 0x04000060, addr)) {
        system.video_unit.renderer_2d[0]->write_word(addr, data);
        return;
    }

    if (Common::in_range(0x04000320, 0x040006A3, addr)) {
        system.video_unit.renderer_3d->write_word(addr, data);
        return;
    }

    if (Common::in_range(0x04001000, 0x04001060, addr)) {
        system.video_unit.renderer_2d[1]->write_word(addr, data);
        return;
    }

    log_warn("ARM9: handle word write %08x = %08x", addr, data);
}