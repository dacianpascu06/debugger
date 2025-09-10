# Simple x86-64 Debugger

A lightweight debugger for **64-bit compiled binaries**. This tool allows you to set breakpoints and dump CPU registers in real-time. It is designed for educational purposes.

---

## Features

- **Set breakpoints**: Pause execution at specific functions by name.
- **Dump registers**: Inspect all general-purpose registers (`RAX`, `RBX`, `RCX`, etc.) and the instruction pointer (`RIP`).  

---

## Supported Commands

| Command        | Description |
|----------------|-------------|
| `run`          | Start or continue execution of the program. |
| `exit`         | Exit the debugger. |
| `break <func>` | Set a breakpoint at the specified function name. |
| `first`        | Print the value of the `RDI` register (first argument). |
| `second`       | Print the value of the `RSI` register (second argument). |
| `regs`         | Dump all general-purpose registers and the instruction pointer (`RIP`). |

---

## Installation

```bash
make
```

## Example

```bash
cd test_files
make
cd ..
./debugger ./test_files/test2
break add
run
regs
```
