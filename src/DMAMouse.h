// DMAMouse.h - DMA Mouse Control Library
// Provides simple API for controlling mouse via DMA on remote PC
// 
// Usage:
//   DMAMouse mouse;
//   if (mouse.Initialize("target.exe")) {
//       mouse.MoveMouse(10, -5);  // Move by delta
//   }

#pragma once
#include <Windows.h>

// Forward declare VMMDLL types to avoid requiring vmmdll.h in client code
typedef void* VMMDLL_HANDLE;
typedef unsigned long long QWORD;

class DMAMouse {
public:
    DMAMouse();
    ~DMAMouse();

    // Initialize DMA connection to target process
    // processName: Name of the process running MouseHandler (e.g., "game.exe", "MouseHandler.exe")
    // Returns: true if connection established successfully
    bool Initialize(const char* processName = "MouseHandler.exe");

    // Move mouse by relative delta (immediate movement)
    // deltaX: Horizontal movement in pixels (positive = right, negative = left)
    // deltaY: Vertical movement in pixels (positive = down, negative = up)
    // Returns: true if command sent successfully
    bool MoveMouse(int deltaX, int deltaY);

    // Check if connection is active and valid
    // Returns: true if initialized and ready to send commands
    bool IsConnected() const;

    // Get last error message
    // Returns: Human-readable error description
    const char* GetLastError() const;

    // Clean up and close DMA connection
    void Shutdown();

private:
    // Internal state
    VMMDLL_HANDLE hVMM;
    unsigned long targetPID;
    QWORD targetAddress;
    bool initialized;
    char lastError[256];

    // Internal helpers
    bool ConnectDMA();
    bool FindTargetProcess(const char* processName);
    bool LocateMouseStructure(QWORD moduleBase);
    QWORD FindSignatureInMemory(QWORD scanStart, QWORD scanSize);
    bool WriteMouse(int deltaX, int deltaY);
    void SetError(const char* error);
};
