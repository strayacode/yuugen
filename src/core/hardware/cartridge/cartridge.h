#pragma once

#include <string>
#include <array>
#include <memory>
#include "common/memory_mapped_file.h"
#include "core/hardware/cartridge/backup/backup.h"

namespace core {

class System;

class Cartridge {
public:
    Cartridge(System& system);

    void reset();
    void load(const std::string& path);
    void direct_boot();

    u16 read_auxspicnt() { return auxspicnt.data; }
    void write_auxspicnt(u16 value, u32 mask);

    u16 read_auxspidata() { return auxspidata; }
    void write_auxspidata(u16 value, u32 mask);

    u32 read_romctrl() { return romctrl.data; }
    void write_romctrl(u32 value, u32 mask);
    void write_command_buffer(u64 value, u64 mask);

    u32 get_arm7_entrypoint() { return header.arm7_entrypoint; }
    u32 get_arm9_entrypoint() { return header.arm9_entrypoint; }

    u32 read_data();

private:
    void load_header();
    void load_backup(std::string path);
    void start_transfer();
    void process_decrypted_command();

    struct Header {
        char game_title[12];

        u32 arm9_offset; // specifies from which offset in the rom data will be transferred to the arm9/arm7 bus
        u32 arm9_entrypoint; // specifies where r15 (program counter) will be set to in memory
        u32 arm9_ram_address; // specifies where in memory data from the cartridge will be transferred to
        u32 arm9_size; // specifies the amount of bytes to be transferred from the cartridge to memory

        u32 arm7_offset; // specifies from which offset in the rom data will be transferred to the arm9/arm7 bus
        u32 arm7_entrypoint; // specifies where r15 (program counter) will be set to in memory
        u32 arm7_ram_address; // specifies where in memory data from the cartridge will be transferred to
        u32 arm7_size; // specifies the amount of bytes to be transferred from the cartridge to memory

        u32 icon_title_offset; // specifies the offset in the rom image to where the icon and title is
        
        // used to identify the backup type
        u32 gamecode;
    };

    union AUXSPICNT {
        struct {
            u8 baudrate : 2;
            u16 : 4;
            bool chipselect_hold : 1;
            bool busy : 1;
            u16 : 5;
            bool slot_mode : 1;
            bool transfer_ready_irq : 1;
            bool slot_enable : 1;
        };

        u16 data;
    };

    union ROMCTRL {
        struct {
            u32 key1_gap1_length : 13;
            bool key2_encrypt_data : 1;
            u32 : 1;
            bool key2_apply_seed : 1;
            u32 key1_gap2_length : 6;
            bool key2_encrypt_command : 1;
            bool word_ready : 1;
            u32 block_size : 3;
            bool transfer_rate : 1;
            bool key1_gap_rate : 1;
            bool resb_release_reset : 1;
            bool data_direction : 1;
            bool block_start : 1;
        };

        u32 data;
    };

    enum class CommandType {
        Dummy,
        ReadData,
        GetFirstId,
        GetSecondId,
        GetThirdId,
        ReadHeader,
        ReadSecureArea,
        None,
    };

    AUXSPICNT auxspicnt;
    u16 auxspidata;
    ROMCTRL romctrl;
    u64 command_buffer;
    u64 command;
    u32 transfer_count;
    u32 transfer_size;
    u32 rom_position;
    u64 seed0;
    u64 seed1;
    bool key1_encryption;
    CommandType command_type;
    std::array<u32, 0x412> key1_buffer;
    std::array<u32, 3> key1_code;
    std::array<u8, 0x4000> secure_area;
    bool cartridge_inserted;

    std::unique_ptr<Backup> backup;

    Header header;
    common::MemoryMappedFile memory_mapped_file;
    System& system;
};

} // namespace core