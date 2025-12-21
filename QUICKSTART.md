# Quick Start Guide

Get up and running with DMA Mouse Control in 5 minutes!

## 📋 Prerequisites Checklist

- [ ] DMA card installed in radar PC (PCIe FPGA device)
- [ ] MemProcFS drivers installed
- [ ] Visual Studio 2022 (or compatible compiler)
- [ ] Two PCs connected (main PC and radar PC with DMA card)

## 🚀 Fast Setup (3 Steps)

### Step 1: Build Everything

```batch
git clone https://github.com/J4sp3rTM/DMA-Mouse-Input-Injection.git
cd DMA-Mouse-Input-Injection
build.bat
```

**Expected output:**
```
✓ MouseHandler.exe built successfully!
✓ Controller.exe built successfully!
✓ Example.exe built successfully!
✓ DLLs copied successfully!
```

### Step 2: Run on Main PC (Target)

On your **main PC** (where you want mouse control):

```batch
cd bin
MouseHandler.exe
```

**You should see:**
```
=== DMA Mouse Handler ===
Main PC Program
========================

PID: 12345
Module Base: 0x7ff600000000
Control Address: 0x7ff600044000
Offset: 0x44000

Ready! Press Ctrl+C to exit.
```

### Step 3: Run on Radar PC (Controller)

On your **radar PC** (with DMA card):

```batch
cd bin
Example.exe
```

**You should see:**
```
=== DMA Mouse Library Example ===
Connecting to target process...
Connected successfully!

Example 1: Moving mouse right by 100 pixels...
  Success!
```

The mouse on your main PC should move! 🎉

## 🎮 Testing

### Test 1: Basic Movement
Run `Example.exe` - it will draw patterns automatically.

### Test 2: Cursor Tracking
Run `Controller.exe` - move your cursor on radar PC, main PC cursor follows.

### Test 3: Your Own Code
Create your own program using the library:

```cpp
#include "DMAMouse.h"

int main() {
    DMAMouse mouse;
    
    if (mouse.Initialize("MouseHandler.exe")) {
        // Move mouse diagonally
        for (int i = 0; i < 100; i++) {
            mouse.MoveMouse(1, 1);
            Sleep(10);
        }
    }
    
    return 0;
}
```

Compile:
```batch
cl MyTest.cpp DMAMouse.cpp /I. external\memprocfs\vmm.lib user32.lib /Fe:MyTest.exe
```

## 🔧 Troubleshooting

### "Failed to initialize MemProcFS"

**Fix:** Check DMA card connection
```batch
# Run as Administrator
# Check device manager for FPGA device
# Reinstall MemProcFS drivers if needed
```

### "Target process not found"

**Fix:** MouseHandler not running
```batch
# On main PC, verify MouseHandler.exe is running
# Check process name matches exactly
# Try using full process name
```

### "Could not locate mouse control structure"

**Fix:** Rebuild both programs
```batch
# Delete bin folder
# Run build.bat again
# Run MouseHandler.exe first, then controller
```

### DLLs Missing

**Fix:** Copy DLLs manually
```batch
copy external\memprocfs\*.dll bin\
```

## 📁 What Each File Does

| File | Purpose | Runs On |
|------|---------|---------|
| `MouseHandler.exe` | Monitors memory, moves mouse | Main PC |
| `Controller.exe` | Tracks cursor, sends via DMA | Radar PC |
| `Example.exe` | Shows library usage | Radar PC |
| `DMAMouse.h/cpp` | Library for integration | Your code |

## 🎯 Next Steps

1. **Read the full README.md** - Complete documentation
2. **Check INTEGRATION.md** - How to use library in your projects
3. **Modify Example.cpp** - Create your own patterns
4. **Build your cheat** - Integrate into your existing code

## 💡 Usage Examples

### Example 1: Simple Movement
```cpp
mouse.MoveMouse(10, 0);  // Right 10 pixels
```

### Example 2: Smooth Aiming
```cpp
for (int i = 0; i < 100; i++) {
    mouse.MoveMouse(targetX / 100, targetY / 100);
    Sleep(1);
}
```

### Example 3: Recoil Control
```cpp
while (shooting) {
    mouse.MoveMouse(0, -2);  // Pull down
    Sleep(10);
}
```

## ⚠️ Important Notes

- **Always run MouseHandler.exe first** on the main PC
- **Run controller as Administrator** for DMA access
- **Keep both programs running** during operation
- **Offset is 0x44000** for manual verification

## 🐛 Debug Mode

If something doesn't work, enable verbose output:

In Controller.cpp, add after successful connection:
```cpp
std::cout << "Debug: PID=" << pid << " Base=0x" << std::hex << base << std::endl;
```

In MouseHandler.cpp, add to loop:
```cpp
std::cout << "Waiting... active=" << g_mouseControl.active << std::endl;
```

## 📞 Get Help

- **GitHub Issues:** Report bugs or ask questions
- **Check Logs:** Look for error messages in console
- **Test Hardware:** Verify DMA card with other tools first

---

**Ready to go? Run the build and start controlling! 🚀**
