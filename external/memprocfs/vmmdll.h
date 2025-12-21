/*
 * vmmdll.h - Minimal MemProcFS header for DMA operations
 * Based on MemProcFS by Ulf Frisk
 * https://github.com/ufrisk/MemProcFS
 */

#ifndef __VMMDLL_H__
#define __VMMDLL_H__

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Handle to MemProcFS instance
 */
typedef HANDLE VMM_HANDLE;

/*
 * Initialize MemProcFS
 * argc = argument count (including program name)
 * argv = array of arguments, e.g., {"-device", "fpga"}
 * Returns: Handle to VMM instance or NULL on failure
 */
VMM_HANDLE VMMDLL_Initialize(_In_ DWORD argc, _In_ LPCSTR argv[]);

/*
 * Close MemProcFS and free resources
 */
VOID VMMDLL_Close(_In_ VMM_HANDLE hVMM);

/*
 * Read memory from target process
 * hVMM = VMM handle
 * dwPID = Process ID
 * qwA = Address to read from
 * pb = Buffer to read into
 * cb = Number of bytes to read
 * Returns: TRUE on success, FALSE on failure
 */
BOOL VMMDLL_MemRead(
    _In_ VMM_HANDLE hVMM,
    _In_ DWORD dwPID,
    _In_ ULONG64 qwA,
    _Out_ PBYTE pb,
    _In_ DWORD cb
);

/*
 * Write memory to target process
 * hVMM = VMM handle
 * dwPID = Process ID
 * qwA = Address to write to
 * pb = Buffer containing data to write
 * cb = Number of bytes to write
 * Returns: TRUE on success, FALSE on failure
 */
BOOL VMMDLL_MemWrite(
    _In_ VMM_HANDLE hVMM,
    _In_ DWORD dwPID,
    _In_ ULONG64 qwA,
    _In_ PBYTE pb,
    _In_ DWORD cb
);

/*
 * Get process information by name
 * hVMM = VMM handle
 * szProcessName = Name of process (e.g., "explorer.exe")
 * pdwPID = Pointer to receive process ID
 * Returns: TRUE on success, FALSE on failure
 */
BOOL VMMDLL_PidGetFromName(
    _In_ VMM_HANDLE hVMM,
    _In_ LPCSTR szProcessName,
    _Out_ PDWORD pdwPID
);

/*
 * Get module base address
 * hVMM = VMM handle
 * dwPID = Process ID
 * uszModuleName = Module name (e.g., "notepad.exe" for main module)
 * Returns: Base address or 0 on failure
 */
ULONG64 VMMDLL_ProcessGetModuleBaseU(
    _In_ VMM_HANDLE hVMM,
    _In_ DWORD dwPID,
    _In_ LPSTR uszModuleName
);

#ifdef __cplusplus
}
#endif

#endif /* __VMMDLL_H__ */
