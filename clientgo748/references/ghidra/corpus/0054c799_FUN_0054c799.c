// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054c799 | Name: FUN_0054c799


int FUN_0054c799(short *param_1,byte param_2)

{
  int iVar1;
  int local_30;
  int local_20;
  int local_1c;
  int local_10;
  int local_c;
  
  local_c = 0;
  iVar1 = (int)*param_1;
  if ((iVar1 < 1) || (0x1964 < iVar1)) {
    local_c = 0;
  }
  else if ((iVar1 < 0xc80) || (0xce4 < iVar1)) {
    if (((param_2 == 0x43) || (param_2 == 0x44)) &&
       ((*(short *)(&DAT_00d449d4 + iVar1 * 0x8c) < 0x29 ||
        (0x32 < *(short *)(&DAT_00d449d4 + iVar1 * 0x8c))))) {
      local_c = 0;
    }
    else {
      if ((param_2 == 0x2a) && (((char)param_1[2] == 'G' || ((char)param_1[3] == 'G')))) {
        param_2 = 0x47;
      }
      if (((param_2 == 2) && (*(short *)(&DAT_00d449d6 + iVar1 * 0x8c) == 0x20)) &&
         (((char)param_1[2] == 'I' || ((char)param_1[3] == 'I')))) {
        param_2 = 0x49;
      }
      if ((param_2 == 0x2e) && (((char)param_1[2] == 'F' || ((char)param_1[3] == 'F')))) {
        param_2 = 0x46;
      }
      if ((param_2 == 0x35) && (((char)param_1[2] == 'H' || ((char)param_1[3] == 'H')))) {
        param_2 = 0x48;
      }
      if (param_2 == 1) {
        local_c = (int)*(short *)(&DAT_00d44996 + iVar1 * 0x8c);
      }
      if (param_2 == 0x16) {
        local_c = local_c + *(short *)(&DAT_00d44998 + iVar1 * 0x8c);
      }
      if (param_2 == 0x17) {
        local_c = local_c + *(short *)(&DAT_00d4499a + iVar1 * 0x8c);
      }
      if (param_2 == 0x18) {
        local_c = local_c + *(short *)(&DAT_00d4499c + iVar1 * 0x8c);
      }
      if (param_2 == 0x19) {
        local_c = local_c + *(short *)(&DAT_00d4499e + iVar1 * 0x8c);
      }
      if (param_2 == 0x11) {
        local_c = local_c + (uint)*(ushort *)(&DAT_00d449d6 + iVar1 * 0x8c);
      }
      if (param_2 != 0x4e) {
        for (local_1c = 0; local_1c < 0xc; local_1c = local_1c + 1) {
          if (*(ushort *)(&DAT_00d449a0 + local_1c * 4 + iVar1 * 0x8c) == (ushort)param_2) {
            local_20 = (int)*(short *)(&DAT_00d449a2 + local_1c * 4 + iVar1 * 0x8c);
            if ((param_2 == 0x1a) && (local_20 == 1)) {
              local_20 = 10;
            }
            local_c = local_c + local_20;
          }
        }
      }
      if ((*param_1 < 0x91a) || (0x955 < *param_1)) {
        if ((*param_1 < 0xf8c) || (3999 < *param_1)) {
          for (local_10 = 0; local_10 < 3; local_10 = local_10 + 1) {
            if ((int)(char)param_1[local_10 + 1] == (uint)param_2) {
              local_30 = (int)*(char *)((int)param_1 + local_10 * 2 + 3);
              if ((param_2 == 0x1a) && (local_30 == 1)) {
                local_30 = 10;
              }
              local_c = local_c + local_30;
            }
          }
        }
        else {
          iVar1 = *param_1 + -0xf8c;
          if (param_2 == 2) {
            local_c = (*(int *)(&DAT_005c0aa8 + iVar1 * 0x18) * 0x15) / 100;
          }
          else if (param_2 == 0x3c) {
            local_c = (*(int *)(&DAT_005c0aac + iVar1 * 0x18) << 4) / 100;
          }
          else if (param_2 == 0x28) {
            local_c = *(int *)(&DAT_005c0ab0 + iVar1 * 0x18);
          }
          else if (param_2 == 0x36) {
            local_c = *(int *)(&DAT_005c0ab4 + iVar1 * 0x18);
          }
        }
      }
      else if (param_2 == 0x50) {
        local_c = (int)param_1[1];
      }
      else if (param_2 == 0x51) {
        local_c = (int)(char)param_1[2];
      }
      else if (param_2 == 0x4f) {
        local_c = (int)*(char *)((int)param_1 + 5);
      }
      else if (param_2 == 0x52) {
        local_c = (int)(char)param_1[3];
      }
      else if (param_2 == 0x53) {
        local_c = (int)*(char *)((int)param_1 + 7);
      }
      else if (((0x939 < *param_1) && (*param_1 < 0x956)) && (0 < param_1[1])) {
        iVar1 = *param_1 + -0x938;
        if (param_2 == 2) {
          local_c = (((char)param_1[2] + 0x14) * *(int *)(&DAT_005c07d8 + iVar1 * 0x18)) / 100;
        }
        else if (param_2 == 0x3c) {
          local_c = (((char)param_1[2] + 0xf) * *(int *)(&DAT_005c07dc + iVar1 * 0x18)) / 100;
        }
        else if (param_2 == 0x28) {
          local_c = *(int *)(&DAT_005c07e0 + iVar1 * 0x18);
        }
        else if (param_2 == 0x36) {
          local_c = *(int *)(&DAT_005c07e4 + iVar1 * 0x18);
        }
      }
    }
  }
  else {
    local_c = 0;
  }
  return local_c;
}

