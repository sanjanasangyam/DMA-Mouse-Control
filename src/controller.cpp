// Controller.cpp - DMA Mouse Controller with automatic address discovery
#include <iostream>
#include <windows.h>
#include "external/memprocfs/vmmdll.h"

// Update this offset if MouseHandler code changes (or set to 0 to force pattern scan)
#define MOUSECONTROL_OFFSET 0x44000

struct MouseControl {
    int deltaX, deltaY, active, signature;
};

bool WriteMouse(VMM_HANDLE h, DWORD pid, uint64_t addr, int dx, int dy) {
    MouseControl data = {dx, dy, 1, (int)0xDEADBEEF};
    return VMMDLL_MemWrite(h, pid, addr, (PBYTE)&data, sizeof(data));
}

// Scan memory for DEADBEEF signature (fallback if offset fails)
uint64_t FindSignatureInMemory(VMM_HANDLE h, DWORD pid, uint64_t base, size_t scanSize) {
    const uint32_t targetSig = 0xDEADBEEF;
    
    std::cout << "Reading " << (scanSize / 1024 / 1024) << "MB into local buffer for scanning..." << std::endl;
    
    // Allocate buffer for entire region
    uint8_t* buf = new uint8_t[scanSize];
    memset(buf, 0, scanSize);
    
    // Read entire region (like DMALib patternScan does)
    if (!VMMDLL_MemRead(h, pid, base, buf, (DWORD)scanSize)) {
        std::cout << "  [WARN] Could not read full region, trying chunked read..." << std::endl;
        
        // Fallback: read in chunks
        const size_t chunkSize = 0x100000; // 1MB chunks
        int successfulReads = 0;
        
        for (size_t offset = 0; offset < scanSize; offset += chunkSize) {
            size_t readSize = (offset + chunkSize > scanSize) ? (scanSize - offset) : chunkSize;
            if (VMMDLL_MemRead(h, pid, base + offset, buf + offset, (DWORD)readSize)) {
                successfulReads++;
            }
        }
        
        std::cout << "  Chunked read: " << successfulReads << " successful chunks" << std::endl;
    } else {
        std::cout << "  [OK] Read region successfully" << std::endl;
    }
    
    // Now scan the local buffer (fast!)
    std::cout << "  Scanning local buffer for DEADBEEF..." << std::endl;
    for (size_t i = 0; i <= scanSize - 4; i += 4) {
        uint32_t value = *(uint32_t*)(buf + i);
        if (value == targetSig) {
            uint64_t signatureAddr = base + i;
            uint64_t structStart = signatureAddr - 0xC; // Signature is at offset +0xC
            
            std::cout << "\n  [FOUND] DEADBEEF at 0x" << std::hex << signatureAddr << std::dec << std::endl;
            std::cout << "  Structure starts at 0x" << std::hex << structStart << std::dec << std::endl;
            
            delete[] buf;
            return structStart;
        }
        
        // Progress every 1MB
        if (i % 0x100000 == 0 && i > 0) {
            std::cout << "  Scanned " << (i / 1024 / 1024) << "MB..." << std::endl;
        }
    }
    
    delete[] buf;
    std::cout << "\n  [FAIL] DEADBEEF not found" << std::endl;
    return 0;
}

int main() {
    std::cout << "=== DMA Mouse Controller ===" << std::endl;
    std::cout << "Offset: 0x" << std::hex << MOUSECONTROL_OFFSET << std::dec << "\n" << std::endl;
    
    // Initialize MemProcFS
    LPCSTR args[] = {"", "-device", "fpga"};
    VMM_HANDLE h = VMMDLL_Initialize(3, args);
    if (!h) { std::cerr << "DMA init failed\n"; return 1; }
    std::cout << "[OK] MemProcFS initialized\n" << std::endl;
    
    // Find process
    std::cout << "Searching for MouseHandler..." << std::endl;
    DWORD pid = 0;
    const char* names[] = {"MouseHandler", "MouseHandler.", "MouseHandle", "MouseHandler.exe"};
    for (int i = 0; i < 4; i++) {
        if (VMMDLL_PidGetFromName(h, names[i], &pid) && pid != 0) {
            std::cout << "[OK] Found: " << names[i] << " (PID: " << pid << ")\n" << std::endl;
            break;
        }
    }
    if (pid == 0) { std::cerr << "Process not found!\n"; return 1; }
    
    // Get module base
    std::cout << "Getting module base..." << std::endl;
    uint64_t base = VMMDLL_ProcessGetModuleBaseU(h, pid, (LPSTR)"MouseHandler.exe");
    if (base == 0) base = VMMDLL_ProcessGetModuleBaseU(h, pid, (LPSTR)"MouseHandler");
    if (base == 0) { std::cerr << "Failed to get module base\n"; return 1; }
    std::cout << "[OK] Module base: 0x" << std::hex << base << std::dec << "\n" << std::endl;
    
    // Calculate address using offset (like cheat devs)
    uint64_t addr = base + MOUSECONTROL_OFFSET;
    std::cout << "Trying offset: 0x" << std::hex << addr << std::dec << std::endl;
    std::cout << "  (base + 0x" << std::hex << MOUSECONTROL_OFFSET << ")" << std::dec << "\n" << std::endl;
    
    // Verify offset by reading and checking signature
    MouseControl testData = {0};
    bool offsetWorks = false;
    
    if (VMMDLL_MemRead(h, pid, addr, (PBYTE)&testData, sizeof(testData))) {
        if (testData.signature == (int)0xDEADBEEF) {
            std::cout << "  [OK] Offset verified!\n" << std::endl;
            offsetWorks = true;
        }
    }
    
    // Fallback to pattern scan if offset failed
    if (!offsetWorks) {
        std::cout << "[WARN] Offset failed, scanning for signature...\n" << std::endl;
        
        uint64_t scanStart = base + 0x40000;
        size_t scanSize = 0x10000; // 64KB range
        
        addr = FindSignatureInMemory(h, pid, scanStart, scanSize);
        
        if (addr == 0) {
            std::cerr << "\n[FAIL] Could not find signature!" << std::endl;
            std::cerr << "Is MouseHandler.exe running on the target PC?" << std::endl;
            std::cout << "\nPress Enter to exit...";
            std::cin.get();
            return 1;
        }
        
        std::cout << "\n[OK] Signature found!\n" << std::endl;
    }
    
    std::cout << "Using address: 0x" << std::hex << addr << std::dec << "\n" << std::endl;
    
    // Start mouse tracking loop
    std::cout << "Mouse tracking active!" << std::endl;
    std::cout << "Move your cursor to control the remote PC.\n" << std::endl;
    
    POINT lastPos = {0};
    GetCursorPos(&lastPos);
    
    int count = 0;
    while (true) {
        POINT currentPos;
        GetCursorPos(&currentPos);
        
        // Calculate delta movement
        int dx = currentPos.x - lastPos.x;
        int dy = currentPos.y - lastPos.y;
        
        // Only send if there's actual movement
        if (dx != 0 || dy != 0) {
            if (WriteMouse(h, pid, addr, dx, dy)) {
                count++;
                if (count % 100 == 0) {
                    std::cout << "Sent " << count << " movements (last: dx=" << dx << " dy=" << dy << ")\n";
                }
            }
            lastPos = currentPos;
        }
        
        Sleep(1); // Poll at ~1000Hz for smooth tracking
    }
}