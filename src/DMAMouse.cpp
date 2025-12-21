// DMAMouse.cpp - DMA Mouse Control Library Implementation
#include "DMAMouse.h"
#include "../external/memprocfs/vmmdll.h"
#include <iostream>
#include <cstring>
#include <vector>

// Mouse control structure (must match MouseHandler.cpp)
struct MouseControl {
    int deltaX;
    int deltaY;
    int active;
    int signature;  // 0xDEADBEEF
};

DMAMouse::DMAMouse() 
    : hVMM(nullptr), targetPID(0), targetAddress(0), initialized(false) {
    lastError[0] = '\0';
}

DMAMouse::~DMAMouse() {
    Shutdown();
}

bool DMAMouse::Initialize(const char* processName) {
    if (initialized) {
        Shutdown();
    }

    // Step 1: Connect to DMA device
    if (!ConnectDMA()) {
        return false;
    }

    // Step 2: Find target process
    if (!FindTargetProcess(processName)) {
        Shutdown();
        return false;
    }

    // Step 3: Get module base and locate mouse structure
    QWORD moduleBase = VMMDLL_ProcessGetModuleBaseU(hVMM, targetPID, (LPSTR)processName);
    if (moduleBase == 0) {
        SetError("Failed to get module base address");
        Shutdown();
        return false;
    }

    if (!LocateMouseStructure(moduleBase)) {
        Shutdown();
        return false;
    }

    initialized = true;
    return true;
}

bool DMAMouse::MoveMouse(int deltaX, int deltaY) {
    if (!initialized) {
        SetError("Not initialized - call Initialize() first");
        return false;
    }

    return WriteMouse(deltaX, deltaY);
}

bool DMAMouse::IsConnected() const {
    return initialized;
}

const char* DMAMouse::GetLastError() const {
    return lastError;
}

void DMAMouse::Shutdown() {
    if (hVMM) {
        VMMDLL_Close(hVMM);
        hVMM = nullptr;
    }
    targetPID = 0;
    targetAddress = 0;
    initialized = false;
}

// === Internal Implementation ===

bool DMAMouse::ConnectDMA() {
    const char* args[] = { "", "-device", "fpga" };
    hVMM = VMMDLL_Initialize(3, args);
    if (!hVMM) {
        SetError("Failed to initialize MemProcFS - check DMA device connection");
        return false;
    }
    return true;
}

bool DMAMouse::FindTargetProcess(const char* processName) {
    // Try exact name
    if (!VMMDLL_PidGetFromName(hVMM, processName, &targetPID)) {
        targetPID = 0;
    }
    
    // If not found and name is long, try truncated version (MemProcFS limitation)
    if (targetPID == 0 && strlen(processName) > 14) {
        char shortName[16] = {0};
        strncpy_s(shortName, processName, 14);
        if (!VMMDLL_PidGetFromName(hVMM, shortName, &targetPID)) {
            targetPID = 0;
        }
    }

    if (targetPID == 0) {
        SetError("Target process not found - is it running?");
        return false;
    }

    return true;
}

bool DMAMouse::LocateMouseStructure(QWORD moduleBase) {
    // Known offset from testing (0x44000)
    QWORD offsetAddr = moduleBase + 0x44000;
    
    // Verify signature at known offset
    int signature = 0;
    if (VMMDLL_MemRead(hVMM, targetPID, offsetAddr + 0xC, (PBYTE)&signature, sizeof(signature))) {
        if (signature == (int)0xDEADBEEF) {
            targetAddress = offsetAddr;
            return true;
        }
    }

    // Fallback: Pattern scan for signature
    QWORD scanStart = moduleBase + 0x40000;
    QWORD scanSize = 0x10000;  // 64KB range
    
    targetAddress = FindSignatureInMemory(scanStart, scanSize);
    if (targetAddress == 0) {
        SetError("Could not locate mouse control structure");
        return false;
    }

    return true;
}

QWORD DMAMouse::FindSignatureInMemory(QWORD scanStart, QWORD scanSize) {
    std::vector<BYTE> buffer(scanSize);
    
    // Read entire region into local memory
    if (!VMMDLL_MemRead(hVMM, targetPID, scanStart, buffer.data(), (DWORD)scanSize)) {
        return 0;
    }

    // Scan for signature (0xDEADBEEF) in local buffer
    const DWORD signature = 0xDEADBEEF;
    for (size_t i = 0; i <= scanSize - sizeof(DWORD); i += 4) {
        if (*(DWORD*)(buffer.data() + i) == signature) {
            // Signature found - structure starts 0xC bytes before
            return scanStart + i - 0xC;
        }
    }

    return 0;
}

bool DMAMouse::WriteMouse(int deltaX, int deltaY) {
    MouseControl cmd;
    cmd.deltaX = deltaX;
    cmd.deltaY = deltaY;
    cmd.active = 1;
    cmd.signature = (int)0xDEADBEEF;

    if (!VMMDLL_MemWrite(hVMM, targetPID, targetAddress, (PBYTE)&cmd, sizeof(cmd))) {
        SetError("Failed to write mouse command via DMA");
        return false;
    }

    return true;
}

void DMAMouse::SetError(const char* error) {
    strncpy_s(lastError, error, sizeof(lastError) - 1);
}
