# elf_scanner.hpp

## File Header Information

| Field | Value |
| :--- | :--- |
| **SPDX Comment** | Minimal ELF64 Scanner |
| **SPDX Type** | SOURCE |
| **Contributor** | ZHENG Robert |
| **License ID** | MIT |
| **File** | `elf_scanner.hpp` |
| **Description** | Scans ELF binaries for dependencies (DT_NEEDED) without external libelf dependency. |
| **Version** | 1.0.0 |
| **Date** | 2026-02-18 |
| **Author** | ZHENG Robert (robert@hase-zheng.net) |
| **Copyright** | Copyright (c) 2026 ZHENG Robert |
| **License** | MIT License |

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [API Documentation](#api-documentation)
  - [`struct Elf64_Ehdr `](#struct-elf64_ehdr-)
  - [`struct Elf64_Phdr `](#struct-elf64_phdr-)
  - [`struct Elf64_Dyn `](#struct-elf64_dyn-)
  - [`inline uint64_t vaddr_to_offset(Elf64_Addr vaddr,`](#inline-uint64_t-vaddr_to_offsetelf64_addr-vaddr)
  - [`inline std::vector<std::string>`](#inline-stdvectorstdstring)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## API Documentation

### `struct Elf64_Ehdr `

> ELF64 File Header structure.

---

### `struct Elf64_Phdr `

> ELF64 Program Header structure.

---

### `struct Elf64_Dyn `

> ELF64 Dynamic Entry structure (found in .dynamic section).

---

### `inline uint64_t vaddr_to_offset(Elf64_Addr vaddr,`

> Converts a virtual memory address (VMA) to a file offset.

| Parameter | Description |
| --- | --- |
| `vaddr` | The virtual address. |
| `phdrs` | A list of program headers to search in. |

**Returns:** uint64_t The calculated file offset, or 0 if not found.

---

### `inline std::vector<std::string>`

> Scans an ELF binary for shared library dependencies.

| Parameter | Description |
| --- | --- |
| `binary_path` | The path to the binary file. |

**Returns:** std::vector<std::string> A list of required shared libraries.

---

