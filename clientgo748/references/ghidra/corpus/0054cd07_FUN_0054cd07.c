// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054cd07 | Name: FUN_0054cd07


uint FUN_0054cd07(short *param_1,byte param_2)

{
  ushort uVar1;
  int iVar2;
  uint local_38;
  int local_34;
  int local_24;
  int local_20;
  int local_1c;
  int local_10;
  uint local_c;
  
  local_c = 0;
  iVar2 = (int)*param_1;
  if ((iVar2 < 1) || (0x1964 < iVar2)) {
    local_c = 0;
  }
  else if ((iVar2 < 0xc80) || (0xce4 < iVar2)) {
    uVar1 = *(ushort *)(&DAT_00d449d6 + iVar2 * 0x8c);
    if (((param_2 == 0x43) || (param_2 == 0x44)) &&
       ((*(short *)(&DAT_00d449d4 + iVar2 * 0x8c) < 0x29 ||
        (0x32 < *(short *)(&DAT_00d449d4 + iVar2 * 0x8c))))) {
      local_c = 0;
    }
    else {
      if ((param_2 == 0x2a) && (((char)param_1[2] == 'G' || ((char)param_1[3] == 'G')))) {
        param_2 = 0x47;
      }
      if (((param_2 == 2) && (uVar1 == 0x20)) &&
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
        for (local_20 = 0; local_20 < 0xc; local_20 = local_20 + 1) {
          if (*(ushort *)(&DAT_00d449a0 + local_20 * 4 + iVar2 * 0x8c) == (ushort)param_2) {
            local_24 = (int)*(short *)(&DAT_00d449a2 + local_20 * 4 + iVar2 * 0x8c);
            if ((param_2 == 0x1a) && (local_24 == 1)) {
              local_24 = 10;
            }
            local_c = local_c + local_24;
          }
        }
      }
      if ((*param_1 < 0x91a) || (0x955 < *param_1)) {
        if ((*param_1 < 0xf8c) || (3999 < *param_1)) {
          for (local_10 = 0; local_10 < 3; local_10 = local_10 + 1) {
            if ((int)(char)param_1[local_10 + 1] == (uint)param_2) {
              local_34 = (int)*(char *)((int)param_1 + local_10 * 2 + 3);
              if ((param_2 == 0x1a) && (local_34 == 1)) {
                local_34 = 10;
              }
              local_c = local_c + local_34;
            }
          }
          local_1c = FUN_0054e06c(param_1);
          if (*param_1 < 0x29) {
            local_1c = 0;
          }
          if ((8 < local_1c) && ((uVar1 & 0xf00) != 0)) {
            local_1c = local_1c + 1;
          }
          if (((((((local_1c != 0) && (param_2 != 0x21)) && (param_2 != 0x12)) &&
                (((param_2 != 0x11 && (param_2 != 0x15)) &&
                 ((param_2 != 0x1b && ((param_2 != 1 && (param_2 != 0x16)))))))) &&
               (param_2 != 0x17)) && (((param_2 != 0x18 && (param_2 != 0x19)) && (param_2 != 0x26)))
              ) && (((param_2 != 0x4e && (param_2 != 0x54)) &&
                    (((param_2 != 0x56 && ((param_2 != 0x70 && (param_2 != 0x57)))) &&
                     (param_2 != 0x58)))))) {
            if (local_1c < 0xb) {
              local_c = (int)(local_c * (local_1c + 10)) / 10;
            }
            else if (10 < local_1c) {
              iVar2 = (int)(local_c * 0x14) / 10;
              local_c = iVar2 + (iVar2 * 10) / 100;
            }
          }
          if (param_2 == 0x1d) {
            if (2 < (int)local_c) {
              local_c = 2;
            }
            if ((0 < (int)local_c) && (8 < local_1c)) {
              local_c = local_c + 1;
            }
          }
          if ((param_2 == 0x38) || (param_2 == 0x39)) {
            local_38 = local_c & 0xff;
            local_c = local_38;
          }
          if ((param_2 == 0x21) && (((int)local_c < 0 || (7 < (int)local_c)))) {
            local_c = 0;
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
  }
  else {
    local_c = 0;
  }
  return local_c;
}

