// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00559f1e | Name: FUN_00559f1e


undefined4 FUN_00559f1e(int param_1)

{
  undefined4 local_8;
  
  local_8 = 0;
  if ((((param_1 == 0x21f) || (param_1 == 0x221)) || (param_1 == 0x6e6)) ||
     (((param_1 == 0x6e7 || (param_1 == 0x6e8)) ||
      ((param_1 == 0xc77 || ((param_1 == 0xc7a || (param_1 == 0xc7d)))))))) {
    local_8 = 7;
  }
  if (((((param_1 == 0x220) || (param_1 == 0x222)) || (param_1 == 0x6e9)) ||
      ((param_1 == 0x6ea || (param_1 == 0x6eb)))) ||
     ((param_1 == 0xc78 || ((param_1 == 0xc7b || (param_1 == 0xc7e)))))) {
    local_8 = 8;
  }
  if ((((param_1 == 0x224) || (param_1 == 0x225)) || (param_1 == 0x23d)) ||
     (((param_1 == 0xc79 || (param_1 == 0xc7c)) || (param_1 == 0xc7f)))) {
    local_8 = 6;
  }
  return local_8;
}

