/**
 * SPDX-FileComment: Minimal ELF64 Scanner
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file elf_scanner.hpp
 * @brief Scans ELF binaries for dependencies (DT_NEEDED) without external libelf dependency.
 * @version 1.0.0
 * @date 2026-02-18
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */
#pragma once
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace depdiscover {

// --- Minimal ELF64 structures (to avoid <elf.h> dependency) ---

using Elf64_Addr = uint64_t;
using Elf64_Off = uint64_t;
using Elf64_Half = uint16_t;
using Elf64_Word = uint32_t;
using Elf64_Xword = uint64_t;

/**
 * @brief ELF64 File Header structure.
 */
struct Elf64_Ehdr {
  unsigned char e_ident[16]; ///< Magic number and other info.
  Elf64_Half e_type;         ///< Object file type.
  Elf64_Half e_machine;      ///< Architecture.
  Elf64_Word e_version;      ///< Object file version.
  Elf64_Addr e_entry;        ///< Entry point virtual address.
  Elf64_Off e_phoff;         ///< Program header table file offset.
  Elf64_Off e_shoff;         ///< Section header table file offset.
  Elf64_Word e_flags;        ///< Processor-specific flags.
  Elf64_Half e_ehsize;       ///< ELF header size in bytes.
  Elf64_Half e_phentsize;    ///< Program header table entry size.
  Elf64_Half e_phnum;        ///< Number of program header table entries.
  Elf64_Half e_shentsize;    ///< Section header table entry size.
  Elf64_Half e_shnum;        ///< Number of section header table entries.
  Elf64_Half e_shstrndx;     ///< Section header table index of the entry associated with the section name string table.
};

/**
 * @brief ELF64 Program Header structure.
 */
struct Elf64_Phdr {
  Elf64_Word p_type;    ///< Segment type.
  Elf64_Word p_flags;   ///< Segment flags.
  Elf64_Off p_offset;   ///< Segment file offset.
  Elf64_Addr p_vaddr;   ///< Segment virtual address.
  Elf64_Addr p_paddr;   ///< Segment physical address.
  Elf64_Xword p_filesz; ///< Segment size in file.
  Elf64_Xword p_memsz;  ///< Segment size in memory.
  Elf64_Xword p_align;  ///< Segment alignment.
};

/**
 * @brief ELF64 Dynamic Entry structure (found in .dynamic section).
 */
struct Elf64_Dyn {
  Elf64_Xword d_tag; ///< Entry type (e.g., DT_NEEDED).
  union {
    Elf64_Xword d_val;
    Elf64_Addr d_ptr;
  } d_un;
};

// ELF Constants
constexpr uint32_t PT_LOAD = 1;
constexpr uint32_t PT_DYNAMIC = 2;
constexpr uint64_t DT_NEEDED = 1;
constexpr uint64_t DT_STRTAB = 5;
constexpr uint64_t DT_STRSZ = 10;

/**
 * @brief Converts a virtual memory address (VMA) to a file offset.
 *
 * @param vaddr The virtual address.
 * @param phdrs A list of program headers to search in.
 * @return uint64_t The calculated file offset, or 0 if not found.
 */
inline uint64_t vaddr_to_offset(Elf64_Addr vaddr,
                                const std::vector<Elf64_Phdr> &phdrs) {
  for (const auto &ph : phdrs) {
    if (ph.p_type == PT_LOAD) {
      // Check if the VMA lies within this segment
      if (vaddr >= ph.p_vaddr && vaddr < (ph.p_vaddr + ph.p_memsz)) {
        return vaddr - ph.p_vaddr + ph.p_offset;
      }
    }
  }
  return 0; // Not found
}

/**
 * @brief Scans an ELF binary for shared library dependencies.
 *
 * Reads the ELF header, program headers, and dynamic section to find DT_NEEDED entries.
 *
 * @param binary_path The path to the binary file.
 * @return std::vector<std::string> A list of required shared libraries.
 */
inline std::vector<std::string>
scan_elf_dependencies(const std::string &binary_path) {
  std::vector<std::string> dependencies;
  std::ifstream file(binary_path, std::ios::binary);

  if (!file)
    return dependencies;

  // 1. Read ELF Header
  Elf64_Ehdr ehdr;
  file.read(reinterpret_cast<char *>(&ehdr), sizeof(ehdr));
  if (!file)
    return dependencies;

  // Check Magic Bytes: 0x7F 'E' 'L' 'F'
  if (ehdr.e_ident[0] != 0x7F || ehdr.e_ident[1] != 'E' ||
      ehdr.e_ident[2] != 'L' || ehdr.e_ident[3] != 'F') {
    return dependencies; // Not an ELF file
  }

  // Check Class (e_ident[4]): 2 = 64-bit. Only 64-bit supported here for simplicity.
  if (ehdr.e_ident[4] != 2) {
    return dependencies;
  }

  // 2. Read Program Headers
  file.seekg(ehdr.e_phoff);
  std::vector<Elf64_Phdr> phdrs(ehdr.e_phnum);
  file.read(reinterpret_cast<char *>(phdrs.data()),
            ehdr.e_phnum * sizeof(Elf64_Phdr));

  // 3. Find Dynamic Segment
  Elf64_Phdr *dyn_phdr = nullptr;
  for (auto &ph : phdrs) {
    if (ph.p_type == PT_DYNAMIC) {
      dyn_phdr = &ph;
      break;
    }
  }

  if (!dyn_phdr)
    return dependencies; // Not dynamically linked

  // 4. Read Dynamic Entries
  size_t num_dyns = dyn_phdr->p_filesz / sizeof(Elf64_Dyn);
  std::vector<Elf64_Dyn> dyns(num_dyns);

  file.seekg(dyn_phdr->p_offset);
  file.read(reinterpret_cast<char *>(dyns.data()), dyn_phdr->p_filesz);

  // 5. Find String Table (DT_STRTAB)
  Elf64_Addr strtab_vaddr = 0;
  Elf64_Xword strtab_size = 0;

  for (const auto &dyn : dyns) {
    if (dyn.d_tag == DT_STRTAB)
      strtab_vaddr = dyn.d_un.d_ptr;
    if (dyn.d_tag == DT_STRSZ)
      strtab_size = dyn.d_un.d_val;
  }

  if (strtab_vaddr == 0 || strtab_size == 0)
    return dependencies;

  // Calculate address of String Table in file
  uint64_t strtab_offset = vaddr_to_offset(strtab_vaddr, phdrs);
  if (strtab_offset == 0)
    return dependencies;

  // 6. Load String Table into memory
  std::vector<char> strtab(strtab_size);
  file.seekg(strtab_offset);
  file.read(strtab.data(), strtab_size);

  // 7. Extract dependencies (DT_NEEDED)
  for (const auto &dyn : dyns) {
    if (dyn.d_tag == DT_NEEDED) {
      // d_val is the index into the String Table
      if (dyn.d_un.d_val < strtab_size) {
        // Safe string extraction: find the null-terminator within the remaining buffer
        const char *start = &strtab[dyn.d_un.d_val];
        size_t max_len = strtab_size - dyn.d_un.d_val;
        size_t actual_len = 0;
        while (actual_len < max_len && start[actual_len] != '\0') {
          actual_len++;
        }
        if (actual_len > 0) {
          std::string lib_name(start, actual_len);
          dependencies.push_back(lib_name);
        }
      }
    }
  }

  return dependencies;
}

} // namespace depdiscover
