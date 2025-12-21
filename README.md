# DMA Mouse Control Library

A clean, lightweight C++ library for controlling mouse input via DMA (Direct Memory Access) on a remote PC. Designed for game development, automation, and cheat development.

## 🎯 Features

- **Simple API** - Just 3 main functions: `Initialize()`, `MoveMouse()`, `Shutdown()`
- **No Configuration Files** - Automatic process and signature discovery
- **Low Latency** - Direct memory writes bypass Windows input stack
- **DMA-Based** - Uses MemProcFS and FPGA hardware for external memory access
- **Clean Code** - Well-documented, easy to integrate into existing projects

## 🛠️ Requirements

### Hardware
- **DMA Card** - PCIe FPGA device (e.g., Squirrel, DMA35, etc.)
- **Two PCs** - Main PC (target) and Radar PC (controller with DMA card)

### Software
- Windows 10/11
- Visual Studio 2022 (or compatible C++ compiler)
- [MemProcFS](https://github.com/ufrisk/MemProcFS) drivers installed

## 📦 Project Structure

```
DMA-Mouse-Handler/
├── src/
│   ├── DMAMouse.h          # Library header
│   ├── DMAMouse.cpp        # Library implementation
│   ├── MouseHandler.cpp    # Target PC program
│   ├── Controller.cpp      # Full controller example
│   └── Example.cpp         # Simple usage example
├── bin/                    # Compiled executables and DLLs
├── external/
│   └── memprocfs/         # MemProcFS library files
└── build.bat              # Build script

```

## 🚀 Quick Start

### 1. Build the Project

```batch
git clone https://github.com/J4sp3rTM/DMA-Mouse-Input-Injection.git
cd DMA-Mouse-Input-Injection
build.bat
```

### 2. Run MouseHandler on Target PC

```batch
bin\MouseHandler.exe
```

Note the displayed offset (usually `0x44000`).

### 3. Run Your Controller on Radar PC

```batch
bin\Example.exe
```

## 📖 Library Usage

### Basic Example

```cpp
#include "DMAMouse.h"

int main() {
    DMAMouse mouse;
    
    // Connect to target process
    if (mouse.Initialize("game.exe")) {
        // Move mouse right 100 pixels
        mouse.MoveMouse(100, 0);
        
        // Move mouse diagonally
        mouse.MoveMouse(50, -50);
    }
    
    mouse.Shutdown();
    return 0;
}
```

### Integration into Your Cheat/Tool

```cpp
#include "DMAMouse.h"

class MyCheat {
private:
    DMAMouse mouse;
    
public:
    bool Initialize() {
        if (!mouse.Initialize("target.exe")) {
            printf("DMA Init failed: %s\n", mouse.GetLastError());
            return false;
        }
        return true;
    }
    
    void AimAt(int targetX, int targetY, int currentX, int currentY) {
        int deltaX = targetX - currentX;
        int deltaY = targetY - currentY;
        
        // Smooth movement
        while (abs(deltaX) > 1 || abs(deltaY) > 1) {
            int stepX = deltaX / 10;
            int stepY = deltaY / 10;
            
            mouse.MoveMouse(stepX, stepY);
            
            deltaX -= stepX;
            deltaY -= stepY;
            Sleep(1);
        }
    }
};
```

## 🔧 API Reference

### `DMAMouse` Class

#### `bool Initialize(const char* processName)`
Establishes DMA connection to target process.
- **Parameters:** Process name (e.g., "game.exe", "MouseHandler.exe")
- **Returns:** `true` on success, `false` on failure
- **Example:**
  ```cpp
  if (!mouse.Initialize("target.exe")) {
      printf("Error: %s\n", mouse.GetLastError());
  }
  ```

#### `bool MoveMouse(int deltaX, int deltaY)`
Moves mouse by relative delta.
- **Parameters:** 
  - `deltaX`: Horizontal pixels (positive = right, negative = left)
  - `deltaY`: Vertical pixels (positive = down, negative = up)
- **Returns:** `true` on success, `false` on failure
- **Example:**
  ```cpp
  mouse.MoveMouse(10, -5);  // Right 10, up 5
  ```

#### `bool IsConnected()`
Checks if DMA connection is active.
- **Returns:** `true` if initialized and ready

#### `const char* GetLastError()`
Gets human-readable error message.
- **Returns:** Error description string

#### `void Shutdown()`
Closes DMA connection and cleans up resources.

## 🏗️ How It Works

### Architecture

```
Radar PC (DMA Card)          Target PC
┌─────────────────┐          ┌──────────────────┐
│  Your Cheat     │          │ MouseHandler.exe │
│  ┌───────────┐  │          │  ┌─────────────┐ │
│  │ DMAMouse  │  │ DMA Read │  │ g_MouseCtrl │ │
│  │ Library   │──┼──────────┼─>│  Structure  │ │
│  └───────────┘  │ DMA Write│  └─────────────┘ │
│                 │ <────────┼─────────┘        │
└─────────────────┘          │  Polls & Moves   │
                             │  Mouse Cursor    │
                             └──────────────────┘
```

### Memory Structure

```cpp
struct MouseControl {
    int deltaX;      // +0x00: X movement
    int deltaY;      // +0x04: Y movement  
    int active;      // +0x08: Command flag
    int signature;   // +0x0C: 0xDEADBEEF
};
```

### Discovery Process

1. **Offset Method (Fast)** - Tries known offset `0x44000` first
2. **Pattern Scan (Fallback)** - Scans memory for `0xDEADBEEF` signature
3. **Verification** - Validates structure before use

## ⚙️ Build Configuration

### Manual Build Commands

**MouseHandler (Target PC):**
```batch
cl /O2 /EHsc MouseHandler.cpp user32.lib /Fe:MouseHandler.exe
```

**Example (Radar PC):**
```batch
cl /O2 /EHsc Example.cpp DMAMouse.cpp /I. external\memprocfs\vmm.lib user32.lib /Fe:Example.exe
```

### Compiler Flags
- `/O2` - Optimize for speed
- `/EHsc` - Enable C++ exception handling
- `/I.` - Include current directory

## 🐛 Troubleshooting

### "Failed to initialize MemProcFS"
- Check DMA card is properly connected
- Verify MemProcFS drivers are installed
- Run as Administrator

### "Target process not found"
- Ensure MouseHandler.exe is running on target PC
- Check process name matches exactly
- Try truncated name if longer than 14 characters

### "Could not locate mouse control structure"
- Verify MouseHandler.exe is the correct version
- Check if antivirus is interfering
- Try rebuilding both programs

## 📝 License

This project is released under MIT License. Free to use, modify, and distribute.

## ⚠️ Disclaimer

This tool is for **educational purposes** and legitimate use cases (automation, accessibility, development). Users are responsible for complying with applicable laws and game/software terms of service. The authors are not responsible for misuse.

## 🤝 Contributing

Pull requests welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Test your changes
4. Submit a PR with clear description

## 📧 Contact

- **GitHub:** [J4sp3rTM](https://github.com/J4sp3rTM)
- **Repository:** [DMA-Mouse-Input-Injection](https://github.com/J4sp3rTM/DMA-Mouse-Input-Injection)

## 🙏 Credits

- [MemProcFS](https://github.com/ufrisk/MemProcFS) by Ulf Frisk
- Inspired by [DMALib](https://github.com/Spuckwaffel/DMALib)

---

**Star ⭐ this repo if you find it useful!**
