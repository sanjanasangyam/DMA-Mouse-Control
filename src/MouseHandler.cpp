// MouseHandler.cpp - Main PC Program
// Runs on the computer where the mouse needs to be controlled
// Monitors two memory addresses for mouse movement delta values written via DMA

#include <Windows.h>
#include <iostream>
#include <atomic>

// Fixed base address for predictable memory location
// This creates a static memory location at a known offset from image base
#define MOUSE_CONTROL_BASE 0x10000

// Memory structure for DMA communication
struct MouseControl {
    volatile int deltaX;      // X movement delta
    volatile int deltaY;      // Y movement delta
    volatile int active;      // Signal flag (1 = new data, 0 = processed)
    volatile int signature;   // Validation signature (0xDEADBEEF)
};

// Global control block at a fixed section for predictable addressing
#pragma section(".shared", read, write, shared)
__declspec(allocate(".shared")) __declspec(align(4096)) MouseControl g_mouseControl = { 0, 0, 0, (int)0xDEADBEEF };

std::atomic<bool> g_running(true);

// Console control handler for clean shutdown
BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) {
        g_running = false;
        return TRUE;
    }
    return FALSE;
}

int main() {
    std::cout << "=== DMA Mouse Handler ===" << std::endl;
    std::cout << "Main PC Program" << std::endl;
    std::cout << "========================\n" << std::endl;
    
    // Set console handler for clean shutdown
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);
    
    // Initialize the control structure
    g_mouseControl.deltaX = 0;
    g_mouseControl.deltaY = 0;
    g_mouseControl.active = 0;
    g_mouseControl.signature = (int)0xDEADBEEF;
    
    // Calculate offset from module base (like cheat devs do)
    HMODULE hModule = GetModuleHandle(NULL);
    uintptr_t moduleBase = reinterpret_cast<uintptr_t>(hModule);
    uintptr_t controlAddress = reinterpret_cast<uintptr_t>(&g_mouseControl);
    uintptr_t offset = controlAddress - moduleBase;
    
    DWORD pid = GetCurrentProcessId();
    
    std::cout << "PID: " << std::dec << pid << std::endl;
    std::cout << "Module Base: 0x" << std::hex << moduleBase << std::endl;
    std::cout << "Control Address: 0x" << controlAddress << std::endl;
    std::cout << "Offset: 0x" << offset << std::dec << "\n" << std::endl;
    
    std::cout << "Ready! Press Ctrl+C to exit.\n" << std::endl;
    
    int movementCount = 0;
    
    // Main polling loop
    while (g_running) {
        // Check if new data is available
        if (g_mouseControl.active == 1) {
            // Validate signature
            if (g_mouseControl.signature == (int)0xDEADBEEF) {
                int dx = g_mouseControl.deltaX;
                int dy = g_mouseControl.deltaY;
                
                // Move the mouse
                mouse_event(MOUSEEVENTF_MOVE, dx, dy, 0, 0);
                
                movementCount++;
                
                // Reset the control structure
                g_mouseControl.deltaX = 0;
                g_mouseControl.deltaY = 0;
                g_mouseControl.active = 0;
            }
        }
        
        // 200Hz polling rate
        Sleep(5);
    }
    
    std::cout << "\nShutdown complete. Total movements: " << movementCount << std::endl;
    return 0;
}

