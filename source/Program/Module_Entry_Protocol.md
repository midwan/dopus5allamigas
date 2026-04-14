# Directory Opus 5 - Module_Entry Execution Protocol (68k)

This document describes the calling convention and data structures for the `Module_Entry` function in Directory Opus 5 on the Amiga (68k).

## 1. Module Entry Point
The primary entry point for a Directory Opus 5 module is the `Module_Entry` function (internal name `L_Module_Entry`). It is defined as follows:

```c
int LIBFUNC L_Module_Entry(
    REG(a0, struct List *files),
    REG(a1, struct Screen *screen),
    REG(a2, IPCData *ipc),
    REG(a3, IPCData *main_ipc),
    REG(d0, ULONG mod_id),
    REG(d1, EXT_FUNC(func_callback))
);
```

### Reference:
- `source/Include/defines/module.h:16`
- `source/Include/inline/module.h:16`

## 2. 68k Register Allocation
The `Module_Entry` function uses standard Amiga library register-passing for its 6 parameters:

| Register | Type | Description |
| :--- | :--- | :--- |
| **A0** | `struct List *` / `char *` | **Polymorphic**: Pointer to a list of files or a raw argument string. |
| **A1** | `struct Screen *` | Pointer to the current screen for GUI operations. |
| **A2** | `IPCData *` | The module's own IPC structure (if running as a process). |
| **A3** | `IPCData *` | The main DOpus IPC structure for system communication. |
| **D0** | `ULONG` | Function ID. `0xffffffff` indicates global startup/shutdown. |
| **D1** | `EXT_FUNC` | Callback function provided by the DOpus core. |

### Polymorphism in A0
The value in `A0` depends on the module type and how it is invoked:
- **Function Modules** (e.g., `play.module`): Typically receive a `struct List *` containing selected file entries.
- **Command Modules** (e.g., `themes.module`): Typically receive a `char *argstring` containing raw command line arguments.

Modules must determine the type of `A0` based on their `ModuleInfo` flags or the `mod_id`.

## 3. Data Structures

### IPCData
Central structure for Inter-Process Communication (IPC).
- **Definition**: `source/Include/libraries/dopus5.h:1385`
- **Key Fields**:
    - `command_port`: Port for receiving commands.
    - `reply_port`: Port for sending replies.
    - `userdata`: Module-specific data (accessible via the `IPCDATA()` macro).

### IPCMessage
Message structure sent between IPC processes.
- **Definition**: `source/Include/libraries/dopus5.h:1366`

### List / Node (Files)
Standard Amiga `Exec/List`. Used to pass a collection of files to modules.
- **Access**: `lh_Head` points to the first node. `ln_Name` of each node contains the file path.

## 4. Core Callback Protocol (D1)
Modules interact with the DOpus core using the `func_callback` passed in `D1`.

### Signature:
```c
unsigned long ASM (*func_callback)(
    REG(d0, ULONG command),
    REG(a0, APTR handle),
    REG(a1, APTR packet)
);
```

### Common Commands:
- `EXTCMD_GET_SOURCE` (0): Get current source path.
- `EXTCMD_GET_ENTRY` (3): Get a selected file/directory entry.
- `EXTCMD_END_ENTRY` (4): Mark an entry as processed.
- `EXTCMD_GET_PORT` (18): Get the ARexx port name.

### Reference:
- `source/Include/libraries/module.h:80` (Enum of commands)
- `source/Program/function_internal.c:266` (Core implementation)

## 5. Return Values
The `Module_Entry` function returns an `int` representing the result:
- `1`: Success.
- `0`: Failure/Error.
- `-1`: User Abort.

## 6. Calling Patterns
- **Internal invocation**: `source/Program/function_internal.c:207`
- **Specific handlers**: `source/Program/function_show.c:238` (for `play.module`)
- **Global startup**: `source/Program/main.c:331` (for `update.module`)
