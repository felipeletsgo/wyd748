// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054d4b6 | Name: FUN_0054d4b6


uint FUN_0054d4b6(short *param_1,byte param_2)

{
  ushort uVar1;
  int iVar2;
  uint local_2c;
  int local_1c;
  int local_18;
  int local_14;
  uint local_c;
  
  local_c = 0;
  iVar2 = (int)*param_1;
  if ((iVar2 < 1) || (0x1964 < iVar2)) {
    local_c = 0;
  }
  else if ((iVar2 < 0xc80) || (0xce4 < iVar2)) {
    uVar1 = *(ushort *)(&DAT_00d449d6 + iVar2 * 0x8c);
    if (param_2 == 1) {
      local_c = (uint)*(short *)(&DAT_00d44996 + iVar2 * 0x8c);
    }
    if (param_2 == 0x16) {
      local_c = local_c + (int)*(short *)(&DAT_00d44998 + iVar2 * 0x8c);
    }
    if (param_2 == 0x17) {
      local_c = local_c + (int)*(short *)(&DAT_00d4499a + iVar2 * 0x8c);
    }
    if (param_2 == 0x18) {
      local_c = local_c + (int)*(short *)(&DAT_00d4499c + iVar2 * 0x8c);
    }
    if (param_2 == 0x19) {
      local_c = local_c + (int)*(short *)(&DAT_00d4499e + iVar2 * 0x8c);
    }
    if (param_2 == 0x11) {
      local_c = local_c + *(ushort *)(&DAT_00d449d6 + iVar2 * 0x8c);
    }
    if (param_2 != 0x4e) {
      for (local_18 = 0; local_18 < 0xc; local_18 = local_18 + 1) {
        if (*(ushort *)(&DAT_00d449a0 + local_18 * 4 + iVar2 * 0x8c) == (ushort)param_2) {
          local_1c = (int)*(short *)(&DAT_00d449a2 + local_18 * 4 + iVar2 * 0x8c);
          if ((param_2 == 0x1a) && (local_1c == 1)) {
            local_1c = 10;
          }
          local_c = local_c + local_1c;
        }
      }
    }
    if ((*param_1 < 0x91a) || (0x955 < *param_1)) {
      if ((*param_1 < 0xf8c) || (3999 < *param_1)) {
        local_14 = FUN_0054e06c(param_1);
        if ((8 < local_14) && ((uVar1 & 0xf00) != 0)) {
          local_14 = local_14 + 1;
        }
        if ((((((local_14 != 0) && (param_2 != 0x21)) && (param_2 != 0x12)) &&
             (((param_2 != 0x11 && (param_2 != 0x15)) &&
              ((param_2 != 0x1b && ((param_2 != 1 && (param_2 != 0x16)))))))) && (param_2 != 0x17))
           && ((((param_2 != 0x18 && (param_2 != 0x19)) && (param_2 != 0x26)) &&
               (((param_2 != 0x4e && (param_2 != 0x54)) && (param_2 != 0x56)))))) {
          if (local_14 < 0xb) {
            local_c = (int)(local_c * (local_14 + 10)) / 10;
          }
          else if (10 < local_14) {
            iVar2 = (int)(local_c * 0x14) / 10;
            local_c = iVar2 + (iVar2 * 10) / 100;
          }
        }
        if (param_2 == 0x1d) {
          if (2 < (int)local_c) {
            local_c = 2;
          }
          if ((0 < (int)local_c) && (8 < local_14)) {
            local_c = local_c + 1;
          }
        }
        if ((param_2 == 0x38) || (param_2 == 0x39)) {
          local_2c = local_c & 0xff;
          local_c = local_2c;
        }
      }
      else {
        iVar2 = *param_1 + -0xf8c;
        if (param_2 == 2) {
          local_c = (*(int *)(&DAT_005c0aa8 + iVar2 * 0x18) * 0x15) / 100;
        }
        else if (param_2 == 0x3c) {
          local_c = (*(int *)(&DAT_005c0aac + iVar2 * 0x18) << 4) / 100;
        }
        else if (param_2 == 0x28) {
          local_c = *(uint *)(&DAT_005c0ab0 + iVar2 * 0x18);
        }
        else if (param_2 == 0x36) {
          local_c = *(uint *)(&DAT_005c0ab4 + iVar2 * 0x18);
        }
      }
    }
    else if (param_2 == 0x50) {
      local_c = (uint)param_1[1];
    }
    else if (param_2 == 0x51) {
      local_c = (uint)(char)param_1[2];
    }
    else if (param_2 == 0x4f) {
      local_c = (uint)*(char *)((int)param_1 + 5);
    }
    else if (param_2 == 0x52) {
      local_c = (uint)(char)param_1[3];
    }
    else if (param_2 == 0x53) {
      local_c = (uint)*(char *)((int)param_1 + 7);
    }
    else if (((0x939 < *param_1) && (*param_1 < 0x956)) && (0 < param_1[1])) {
      iVar2 = *param_1 + -0x938;
      if (param_2 == 2) {
        local_c = (((char)param_1[2] + 0x14) * *(int *)(&DAT_005c07d8 + iVar2 * 0x18)) / 100;
      }
      else if (param_2 == 0x3c) {
        local_c = (((char)param_1[2] + 0xf) * *(int *)(&DAT_005c07dc + iVar2 * 0x18)) / 100;
      }
      else if (param_2 == 0x28) {
        local_c = *(uint *)(&DAT_005c07e0 + iVar2 * 0x18);
      }
      else if (param_2 == 0x36) {
        local_c = *(uint *)(&DAT_005c07e4 + iVar2 * 0x18);
      }
    }
  }
  else {
    local_c = 0;
  }
  return local_c;
}

