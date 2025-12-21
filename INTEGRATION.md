# Integration Guide

This guide shows how to integrate the DMA Mouse library into your existing C++ project.

## Method 1: Quick Integration (Copy Files)

### 1. Copy Required Files

Copy these files to your project:
```
YourProject/
├── DMAMouse.h          (from src/)
├── DMAMouse.cpp        (from src/)
└── external/
    └── memprocfs/
        ├── vmmdll.h
        ├── vmm.lib
        ├── vmm.dll
        ├── leechcore.dll
        └── FTD3XX.dll
```

### 2. Update Your Build Command

```batch
cl YourCheat.cpp DMAMouse.cpp /I. external\memprocfs\vmm.lib user32.lib /Fe:YourCheat.exe
```

### 3. Use in Your Code

```cpp
#include "DMAMouse.h"

int main() {
    DMAMouse mouse;
    
    if (mouse.Initialize("target.exe")) {
        mouse.MoveMouse(100, 0);  // Move right
    }
    
    return 0;
}
```

## Method 2: Visual Studio Project Integration

### 1. Add Files to Project

1. Right-click your project → Add → Existing Item
2. Add `DMAMouse.h` and `DMAMouse.cpp`
3. Add `external/memprocfs/vmmdll.h`

### 2. Configure Project Settings

**Include Directories:**
- Project Properties → C/C++ → General → Additional Include Directories
- Add: `$(ProjectDir)`

**Library Dependencies:**
- Project Properties → Linker → Input → Additional Dependencies
- Add: `external\memprocfs\vmm.lib;user32.lib`

**Library Directories:**
- Project Properties → Linker → General → Additional Library Directories  
- Add: `$(ProjectDir)`

### 3. Copy DLLs to Output

Add Post-Build Event:
```batch
copy "$(ProjectDir)external\memprocfs\*.dll" "$(OutDir)"
```

## Method 3: Static Library (.lib)

### 1. Create Static Library Project

```batch
REM Compile to object file
cl /c /O2 /EHsc DMAMouse.cpp /Fo:DMAMouse.obj

REM Create static library
lib DMAMouse.obj external\memprocfs\vmm.lib /OUT:DMAMouse.lib
```

### 2. Link in Your Project

```batch
cl YourCheat.cpp DMAMouse.lib user32.lib /Fe:YourCheat.exe
```

## Complete Example - Cheat Integration

```cpp
#include "DMAMouse.h"
#include <Windows.h>
#include <iostream>
#include <cmath>

class MyCheat {
private:
    DMAMouse mouse;
    bool initialized;
    
public:
    MyCheat() : initialized(false) {}
    
    bool Init(const char* targetProcess) {
        std::cout << "Initializing DMA mouse..." << std::endl;
        
        if (!mouse.Initialize(targetProcess)) {
            std::cerr << "Error: " << mouse.GetLastError() << std::endl;
            return false;
        }
        
        initialized = true;
        std::cout << "DMA mouse initialized!" << std::endl;
        return true;
    }
    
    void SmoothMoveTo(int targetX, int targetY, int currentX, int currentY) {
        if (!initialized) return;
        
        int deltaX = targetX - currentX;
        int deltaY = targetY - currentY;
        
        // Smooth movement over multiple steps
        int steps = 10;
        for (int i = 0; i < steps; i++) {
            int stepX = deltaX / steps;
            int stepY = deltaY / steps;
            
            mouse.MoveMouse(stepX, stepY);
            Sleep(1);  // 1ms between steps
        }
    }
    
    void RecoilCompensation(int recoilX, int recoilY) {
        if (!initialized) return;
        
        // Compensate for weapon recoil (inverted delta)
        mouse.MoveMouse(-recoilX, -recoilY);
    }
    
    void DrawCircle(int radius) {
        if (!initialized) return;
        
        // Draw circular pattern (for testing)
        for (int angle = 0; angle < 360; angle += 5) {
            double rad = angle * 3.14159 / 180.0;
            int dx = (int)(cos(rad) * radius / 36);  // Small steps
            int dy = (int)(sin(rad) * radius / 36);
            
            mouse.MoveMouse(dx, dy);
            Sleep(2);
        }
    }
    
    ~MyCheat() {
        if (initialized) {
            mouse.Shutdown();
        }
    }
};

int main() {
    MyCheat cheat;
    
    // Initialize with target process
    if (!cheat.Init("game.exe")) {
        std::cout << "Press Enter to exit...";
        std::cin.get();
        return 1;
    }
    
    // Your cheat logic here
    while (true) {
        // Example: Move mouse when hotkey pressed
        if (GetAsyncKeyState(VK_F1) & 0x8000) {
            cheat.SmoothMoveTo(960, 540, 500, 300);
            Sleep(100);
        }
        
        // Example: Recoil compensation
        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            cheat.RecoilCompensation(0, 2);  // Pull down
        }
        
        // Exit on ESC
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }
        
        Sleep(1);
    }
    
    return 0;
}
```

## Common Integration Patterns

### Pattern 1: Aimbot

```cpp
void AimAt(DMAMouse& mouse, int targetX, int targetY) {
    POINT cursor;
    GetCursorPos(&cursor);
    
    int deltaX = targetX - cursor.x;
    int deltaY = targetY - cursor.y;
    
    // Smooth aim
    while (abs(deltaX) > 2 || abs(deltaY) > 2) {
        int stepX = deltaX / 5;
        int stepY = deltaY / 5;
        
        mouse.MoveMouse(stepX, stepY);
        
        deltaX -= stepX;
        deltaY -= stepY;
        Sleep(1);
    }
}
```

### Pattern 2: Recoil Control

```cpp
struct RecoilPattern {
    int frames[30][2];  // {deltaX, deltaY} per frame
    int frameCount;
};

void HandleRecoil(DMAMouse& mouse, const RecoilPattern& pattern) {
    for (int i = 0; i < pattern.frameCount; i++) {
        mouse.MoveMouse(-pattern.frames[i][0], -pattern.frames[i][1]);
        Sleep(10);  // 100Hz
    }
}
```

### Pattern 3: Silent Aim (No visible cursor movement)

```cpp
// DMA mouse moves target PC cursor without affecting radar PC cursor
void SilentAim(DMAMouse& mouse, int targetScreenX, int targetScreenY) {
    // Calculate required delta without moving local cursor
    // This is DMA's advantage - cursor moves on target PC only!
    mouse.MoveMouse(targetScreenX, targetScreenY);
}
```

## Troubleshooting

### Build Errors

**"Cannot find vmmdll.h"**
- Add include path: `/I.` or set Additional Include Directories

**"Unresolved external symbol"**
- Add `external\memprocfs\vmm.lib` to linker input
- Add `user32.lib` for Windows API

**"DLL not found at runtime"**
- Copy DLLs to same directory as .exe
- Or add DLL directory to PATH

### Runtime Errors

**"Failed to initialize MemProcFS"**
- Check DMA card connection
- Run as Administrator
- Verify drivers installed

**"Target process not found"**
- Ensure MouseHandler.exe running on target PC
- Check process name spelling (case-sensitive)

**"Could not locate mouse control structure"**
- Verify MouseHandler.exe version matches
- Rebuild both programs
- Check antivirus not blocking

## Performance Tips

1. **Reduce Sleep Times** - For faster response, reduce sleep to 1ms
2. **Batch Movements** - Send multiple small movements instead of one large one
3. **Check Connection** - Use `IsConnected()` before critical operations
4. **Error Handling** - Always check return values and handle failures

## Need Help?

- Check the Example.cpp for working code
- Read the main README.md for setup instructions
- Open an issue on GitHub for bugs

---

Happy coding! 🚀
