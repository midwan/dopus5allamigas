# Module Discovery and Identification in Directory Opus 5

## Overview
Directory Opus 5 uses an asynchronous, message-driven system to discover and register modules located in `dopus5:modules/`. This ensures the UI remains responsive while external libraries are scanned and identified.

## Discovery Workflow

### 1. Directory Notification
The system monitors the `dopus5:modules` directory for changes.
- **Initialization**: `source/Program/main.c` calls `start_file_notify("dopus5:modules", NOTIFY_MODULES_CHANGED, GUI->appmsg_port)`.
- **Signal**: When a file is added, removed, or modified, a `NOTIFY_MODULES_CHANGED` message is sent to the main application port.

### 2. Event Handling
The main event loop in `source/Program/event_loop.c` listens for these notifications.
- **Reception**: Upon receiving `NOTIFY_MODULES_CHANGED`, the loop launches a background process named `dopus_module_sniffer`.
- **Command**: The sniffer is started with the `MAIN_SNIFF_MODULES` command and the `SNIFF_BOTH` flag.

### 3. Background Sniffing
The background process (`source/Program/misc_proc.c`) executes the sniffing logic.
- **Execution**: It calls `update_commands()`, which in turn calls `init_commands_scan(SCAN_MODULES)`.
- **Scanning**: `init_commands_scan` uses `MatchFirst`/`MatchNext` to find all `#?.module` files in `dopus5:modules/`.

### 4. Identification (`Module_Identify`)
For each discovered module:
- **Library Open**: The module is opened as a standard Amiga shared library.
- **Module Metadata**: The system calls `Module_Identify(-1)` to retrieve a `ModuleInfo` structure. This contains the module's descriptive name, the number of functions it exports, and capability flags.
- **Function Metadata**: For each function index (`0` to `function_count - 1`), `Module_Identify(num)` is called to retrieve the descriptive name/string for that specific command.

### 5. Registration
Once identified, the module's functions are registered as commands.
- **Add Command**: `add_command()` is called for each function, adding it to `GUI->command_list`.
- **Internal Mapping**: The command stores the module filename and the function ID for later execution.

## Technical Details
- **Location**: `dopus5:modules/`
- **Suffix**: `.module`
- **Exclusion List**: Defined in `source/Program/commands.c` (e.g., `configopus.module`, `read.module`).
- **Data Structure**: `GUI->modules_list` tracks currently loaded modules and their datestamps to prevent unnecessary re-scanning.

## Key Files
- `source/Program/main.c`: Notification initialization.
- `source/Program/event_loop.c`: Notification event handling.
- `source/Program/misc_proc.c`: Background process dispatch.
- `source/Program/commands.c`: Directory scanning, identification, and registration.
