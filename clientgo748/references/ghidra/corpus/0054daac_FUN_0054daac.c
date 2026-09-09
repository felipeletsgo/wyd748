// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054daac | Name: FUN_0054daac


int FUN_0054daac(short *param_1,byte param_2)

{
  ushort uVar1;
  int iVar2;
  int local_1c;
  int local_18;
  int local_10;
  int local_c;
  
  if ((*param_1 < 0x91a) || (0x955 < *param_1)) {
    if ((*param_1 < 0xf8c) || (3999 < *param_1)) {
      local_c = 0;
      iVar2 = (int)*param_1;
      if ((iVar2 < 1) || (0x1964 < iVar2)) {
        local_c = 0;
      }
      else if ((iVar2 < 0xc80) || (0xce4 < iVar2)) {
        uVar1 = *(ushort *)(&DAT_00d449d6 + iVar2 * 0x8c);
        for (local_10 = 0; local_10 < 3; local_10 = local_10 + 1) {
          if ((int)(char)param_1[local_10 + 1] == (uint)param_2) {
            local_1c = (int)*(char *)((int)param_1 + local_10 * 2 + 3);
            if ((param_2 == 0x1a) && (local_1c == 1)) {
              local_1c = 10;
            }
            local_c = local_c + local_1c;
          }
        }
        local_18 = FUN_0054e06c(param_1);
        if ((8 < local_18) && ((uVar1 & 0xf00) != 0)) {
          local_18 = local_18 + 1;
        }
        if (((((local_18 != 0) && (param_2 != 0x21)) && (param_2 != 0x12)) &&
            (((param_2 != 0x11 && (param_2 != 0x15)) &&
             ((param_2 != 0x1b && ((param_2 != 1 && (param_2 != 0x16)))))))) &&
           ((param_2 != 0x17 &&
            (((((param_2 != 0x18 && (param_2 != 0x19)) && (param_2 != 0x26)) &&
              ((param_2 != 0x4e && (param_2 != 0x54)))) && (param_2 != 0x56)))))) {
          if (local_18 < 0xb) {
            local_c = (local_c * (local_18 + 10)) / 10;
          }
          else if (10 < local_18) {
            local_c = (local_c * 0x14) / 10;
            local_c = local_c + (local_c * 10) / 100;
          }
        }
      }
      else {
        local_c = 0;
      }
    }
    else {
      local_c = 0;
    }
  }
  else {
    local_c = 0;
  }
  return local_c;
}

