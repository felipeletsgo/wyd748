// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055f657 | Name: FUN_0055f657


undefined4 FUN_0055f657(char *param_1)

{
  char cVar1;
  size_t sVar2;
  undefined4 uVar3;
  int local_20;
  char local_14 [12];
  char *local_8;
  
  local_8 = param_1;
  _memset(local_14,0,0xc);
  sVar2 = _strlen(param_1);
  if ((int)sVar2 < 8) {
    uVar3 = 0;
  }
  else if ((int)sVar2 < 0x19) {
    cVar1 = *local_8;
    *local_8 = local_8[7];
    local_8[7] = cVar1;
    cVar1 = local_8[3];
    local_8[3] = local_8[6];
    local_8[6] = cVar1;
    for (local_20 = 0; local_20 < (int)sVar2; local_20 = local_20 + 2) {
      if ((local_8[local_20] < '0') || ('9' < local_8[local_20])) {
        if ((local_8[local_20] < 'A') || ('Z' < local_8[local_20])) {
          if ((local_8[local_20] < 'a') || ('z' < local_8[local_20])) {
            return 0;
          }
          local_14[local_20 / 2] = local_8[local_20] + -0x61 + local_8[local_20 + 1];
        }
        else {
          local_14[local_20 / 2] = local_8[local_20] + -0x41 + local_8[local_20 + 1];
        }
      }
      else {
        local_14[local_20 / 2] = local_8[local_20] + -0x30 + local_8[local_20 + 1];
      }
    }
    _memset(param_1,0,0x18);
    FUN_0058f220(param_1,local_14,sVar2);
    uVar3 = 1;
  }
  else {
    uVar3 = 0;
  }
  return uVar3;
}

