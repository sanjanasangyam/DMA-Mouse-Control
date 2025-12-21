// Example.cpp - Simple example of using DMAMouse library
// This demonstrates how to integrate DMA mouse control into your application
//
// Compile with: cl /nologo /O2 /EHsc Example.cpp DMAMouse.cpp /I. external\memprocfs\vmm.lib user32.lib /Fe:bin\Example.exe

#include "DMAMouse.h"
#include <Windows.h>
#include <iostream>

int main() {
    std::cout << "=== DMA Mouse Library Example ===" << std::endl;
    std::cout << "This example shows basic usage of the DMAMouse library\n" << std::endl;

    // Create DMA mouse controller
    DMAMouse mouse;

    // Initialize connection to target process
    std::cout << "Connecting to target process..." << std::endl;
    if (!mouse.Initialize("MouseHandler.exe")) {
        std::cerr << "Failed to initialize: " << mouse.GetLastError() << std::endl;
        std::cout << "\nMake sure:" << std::endl;
        std::cout << "  1. MouseHandler.exe is running on the target PC" << std::endl;
        std::cout << "  2. DMA device is connected" << std::endl;
        std::cout << "  3. MemProcFS drivers are installed" << std::endl;
        std::cout << "\nPress Enter to exit...";
        std::cin.get();
        return 1;
    }

    std::cout << "Connected successfully!\n" << std::endl;

    // Example 1: Simple mouse movement
    std::cout << "Example 1: Moving mouse right by 100 pixels..." << std::endl;
    if (mouse.MoveMouse(100, 0)) {
        std::cout << "  Success!" << std::endl;
    }
    Sleep(1000);

    // Example 2: Circular pattern
    std::cout << "\nExample 2: Drawing a circle..." << std::endl;
    for (int angle = 0; angle < 360; angle += 10) {
        double rad = angle * 3.14159 / 180.0;
        int dx = (int)(cos(rad) * 5);
        int dy = (int)(sin(rad) * 5);
        
        if (!mouse.MoveMouse(dx, dy)) {
            std::cerr << "Error: " << mouse.GetLastError() << std::endl;
            break;
        }
        Sleep(10);
    }
    std::cout << "  Complete!" << std::endl;

    // Example 3: Smooth movement
    std::cout << "\nExample 3: Smooth diagonal movement..." << std::endl;
    for (int i = 0; i < 50; i++) {
        mouse.MoveMouse(2, 2);
        Sleep(10);
    }
    std::cout << "  Complete!" << std::endl;

    // Example 4: Check connection status
    std::cout << "\nConnection status: " << (mouse.IsConnected() ? "Active" : "Disconnected") << std::endl;

    // Cleanup
    mouse.Shutdown();
    
    std::cout << "\nExample complete! Press Enter to exit...";
    std::cin.get();
    return 0;
}
