// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00595528 | Name: FUN_00595528


uint FUN_00595528(undefined4 param_1,uint param_2)

{
  if ((param_2 & 0x7ff00000) != 0x7ff00000) {
    return param_2 & 0x7ff00000;
  }
  return param_2;
}

