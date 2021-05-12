INSTRUCTION(ARM_STM_DECREMENT_BEFORE_WRITEBACK) {
    u32 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            address -= 4;
        }
    }

    u32 writeback = address;

    // subtract offset from base
    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) { 
            // write register to address
            WriteWord(address, regs.r[i]);
            // pre decrement the address
            address += 4;
        }
    }

    // writeback to base register
    regs.r[rn] = writeback;

    regs.r[15] += 4;
}

INSTRUCTION(ARM_LDM_INCREMENT_BEFORE) {
    u32 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            address += 4;
            regs.r[i] = ReadWord(address);
        }
    }
    
    if (instruction & (1 << 15)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_LDM_INCREMENT_BEFORE_USER) {
    u32 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];

    u32 old_mode = regs.cpsr & 0x1F;
    SwitchMode(SYS);

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            address += 4;
            regs.r[i] = ReadWord(address);
        }
    }

    SwitchMode(old_mode);
    
    if (instruction & (1 << 15)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_LDM_INCREMENT_BEFORE_WRITEBACK) {
    u32 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            address += 4;
            regs.r[i] = ReadWord(address);
        }
    }


    // if rn is in rlist:
    // if arm9 writeback if rn is the only register or not the last register in rlist
    // if arm7 then no writeback if rn in rlist
    if (arch == ARMv5) {
        if (((instruction & 0xFFFF) == (unsigned int)(1 << rn) )|| !(((instruction & 0xFFFF) >> rn) == 1)) {
            regs.r[rn] = address;
        }
    } else {
        if (!(instruction & (unsigned int)(1 << rn))) {
            regs.r[rn] = address;
        }
    }
    
    if (instruction & (1 << 15)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_LDM_INCREMENT_AFTER_WRITEBACK) {
    u32 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];
    
    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            regs.r[i] = ReadWord(address);
            address += 4;
        }
    }




    // if rn is in rlist:
    // if arm9 writeback if rn is the only register or not the last register in rlist
    // if arm7 then no writeback if rn in rlist
    if (arch == ARMv5) {
        if (((instruction & 0xFFFF) == (unsigned int)(1 << rn) )|| !(((instruction & 0xFFFF) >> rn) == 1)) {
            regs.r[rn] = address;
        }
    } else {
        if (!(instruction & (unsigned int)(1 << rn))) {
            regs.r[rn] = address;
        }
    }
    
    if (instruction & (1 << 15)) {
        // log_fatal("handle");
        // handle arm9 behaviour
        if (arch == ARMv5) {
            if (regs.r[15] & 0x1) {
                // switch to thumb mode
                regs.cpsr |= (1 << 5);

                // halfword align the address
                regs.r[15] &= ~1;

                ThumbFlushPipeline();
            } else {
                // word align the address
                regs.r[15] &= ~3;

                ARMFlushPipeline();
            }
        } else {
            // word align the address
            regs.r[15] &= ~3;

            ARMFlushPipeline();
        }
    } else {
        regs.r[15] += 4;
    }  
}

INSTRUCTION(ARM_LDM_INCREMENT_AFTER) {
    u32 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];
    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            regs.r[i] = ReadWord(address);
            address += 4;
        }
    }
    
    if (instruction & (1 << 15)) {
        log_fatal("handle");
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_LDM_INCREMENT_AFTER_USER) {
    u32 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];
    
    u8 old_mode = regs.cpsr & 0x1F;
    
    // first we must switch to user mode so that we can change the values of usr mode registers
    SwitchMode(SYS);

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            regs.r[i] = ReadWord(address);
            address += 4;
        }
    }

    // switching back to to old mode is my guess
    SwitchMode(old_mode);
    
    if (instruction & (1 << 15)) {
        log_fatal("handle");
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_STM_INCREMENT_AFTER_WRITEBACK) {
    u32 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            WriteWord(address, regs.r[i]);
            address += 4;
        }
    }

    regs.r[rn] = address;

    regs.r[15] += 4;
}

INSTRUCTION(ARM_STM_INCREMENT_AFTER) {
    u32 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            WriteWord(address, regs.r[i]);
            address += 4;
        }
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_LDM_DECREMENT_BEFORE) {
    u32 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            address -= 4;
        }
    }

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            regs.r[i] = ReadWord(address);
            address += 4;
        }
    }
    
    if (instruction & (1 << 15)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_LDM_DECREMENT_BEFORE_WRITEBACK) {
    u32 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            address -= 4;
        }
    }

    u32 writeback = address;

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            regs.r[i] = ReadWord(address);
            address += 4;
        }
    }

    // if rn is in rlist:
    // if arm9 writeback if rn is the only register or not the last register in rlist
    // if arm7 then no writeback if rn in rlist
    if (arch == ARMv5) {
        if (((instruction & 0xFFFF) == (unsigned int)(1 << rn) )|| !(((instruction & 0xFFFF) >> rn) == 1)) {
            regs.r[rn] = writeback;
        }
    } else {
        if (!(instruction & (unsigned int)(1 << rn))) {
            regs.r[rn] = writeback;
        }
    }
    
    if (instruction & (1 << 15)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_LDM_DECREMENT_AFTER_WRITEBACK) {
    u32 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            address -= 4;
        }
    }

    u32 writeback = address;

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            address += 4;
            regs.r[i] = ReadWord(address);
            
        }
    }

    // if rn is in rlist:
    // if arm9 writeback if rn is the only register or not the last register in rlist
    // if arm7 then no writeback if rn in rlist

    if (arch == ARMv5) {
        if (((instruction & 0xFFFF) == (unsigned int)(1 << rn) )|| !(((instruction & 0xFFFF) >> rn) == 1)) {
            regs.r[rn] = writeback;
        }
    } else {
        if (!(instruction & (unsigned int)(1 << rn))) {
            regs.r[rn] = writeback;
        }
    }
    
    if (instruction & (1 << 15)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_LDM_INCREMENT_BEFORE_USER_WRITEBACK) {
    u32 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];

    u8 old_mode = regs.cpsr & 0x1F;

    SwitchMode(SYS);

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            address += 4;
            regs.r[i] = ReadWord(address);
        }
    }

    SwitchMode(old_mode);


    // if rn is in rlist:
    // if arm9 writeback if rn is the only register or not the last register in rlist
    // if arm7 then no writeback if rn in rlist
    if (arch == ARMv5) {
        if (((instruction & 0xFFFF) == (unsigned int)(1 << rn) )|| !(((instruction & 0xFFFF) >> rn) == 1)) {
            regs.r[rn] = address;
        }
    } else {
        if (!(instruction & (unsigned int)(1 << rn))) {
            regs.r[rn] = address;
        }
    }
    
    if (instruction & (1 << 15)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_LDM_INCREMENT_AFTER_USER_WRITEBACK) {
    u32 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];
    
    u8 old_mode = regs.cpsr & 0x1F;
    
    // first we must switch to user mode so that we can change the values of usr mode registers
    SwitchMode(SYS);

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            regs.r[i] = ReadWord(address);
            address += 4;
        }
    }

    // switching back to to old mode is my guess
    SwitchMode(old_mode);

    // if rn is in rlist:
    // if arm9 writeback if rn is the only register or not the last register in rlist
    // if arm7 then no writeback if rn in rlist
    if (arch == ARMv5) {
        if (((instruction & 0xFFFF) == (unsigned int)(1 << rn) )|| !(((instruction & 0xFFFF) >> rn) == 1)) {
            regs.r[rn] = address;
        }
    } else {
        if (!(instruction & (unsigned int)(1 << rn))) {
            regs.r[rn] = address;
        }
    }

    if (instruction & (1 << 15)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_STM_DECREMENT_BEFORE) {
    u32 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];

    

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            address -= 4;
        }
    }

    // subtract offset from base
    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) { 
            // write register to address
            WriteWord(address, regs.r[i]);
            // pre decrement the address
            address += 4;
        }
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_LDM_DECREMENT_BEFORE_USER_WRITEBACK) {
    u32 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];

    u8 old_mode = regs.cpsr & 0x1F;

    SwitchMode(SYS);

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            address -= 4;
        }
    }

    u32 writeback = address;

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            regs.r[i] = ReadWord(address);
            address += 4;
        }
    }

    SwitchMode(old_mode);

    // if rn is in rlist:
    // if arm9 writeback if rn is the only register or not the last register in rlist
    // if arm7 then no writeback if rn in rlist
    if (arch == ARMv5) {
        if (((instruction & 0xFFFF) == (unsigned int)(1 << rn) )|| !(((instruction & 0xFFFF) >> rn) == 1)) {
            regs.r[rn] = writeback;
        }
    } else {
        if (!(instruction & (unsigned int)(1 << rn))) {
            regs.r[rn] = writeback;
        }
    }
    
    if (instruction & (1 << 15)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_LDM_DECREMENT_AFTER_USER_WRITEBACK) {
    u32 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];

    u8 old_mode = regs.cpsr & 0x1F;

    SwitchMode(SYS);

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            address -= 4;
        }
    }

    u32 writeback = address;

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            address += 4;
            regs.r[i] = ReadWord(address);
            
        }
    }

    SwitchMode(old_mode);

    // if rn is in rlist:
    // if arm9 writeback if rn is the only register or not the last register in rlist
    // if arm7 then no writeback if rn in rlist
    if (arch == ARMv5) {
        if (((instruction & 0xFFFF) == (unsigned int)(1 << rn) )|| !(((instruction & 0xFFFF) >> rn) == 1)) {
            regs.r[rn] = writeback;
        }
    } else {
        if (!(instruction & (unsigned int)(1 << rn))) {
            regs.r[rn] = writeback;
        }
    }
    
    if (instruction & (1 << 15)) {
        log_fatal("handle lol");
    }

    regs.r[15] += 4;
}

INSTRUCTION(ARM_STM_INCREMENT_BEFORE_WRITEBACK) {
    u32 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            address += 4;
            WriteWord(address, regs.r[i]);
        }
    }

    regs.r[rn] = address;

    regs.r[15] += 4;
}

INSTRUCTION(ARM_STM_INCREMENT_BEFORE_USER_WRITEBACK) {
    u32 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];

    u32 old_mode = regs.cpsr & 0x1F;
    SwitchMode(SYS);

    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            address += 4;
            WriteWord(address, regs.r[i]);
        }
    }

    SwitchMode(old_mode);

    regs.r[rn] = address;

    regs.r[15] += 4;
}

INSTRUCTION(ARM_STM_DECREMENT_AFTER_WRITEBACK) {
    u32 rn = (instruction >> 16) & 0xF;
    u32 address = regs.r[rn];

    
    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) {
            address -= 4;
        }
    }

    u32 writeback = address;
    
    // subtract offset from base
    for (int i = 0; i < 16; i++) {
        if (instruction & (1 << i)) { 
            // post decrement the address
            address += 4;
            // write register to address
            WriteWord(address, regs.r[i]);
            
        }
    }

    // writeback to base register
    regs.r[rn] = writeback;

    regs.r[15] += 4;
}