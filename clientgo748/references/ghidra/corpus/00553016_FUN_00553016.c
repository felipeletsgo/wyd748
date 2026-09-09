// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00553016 | Name: FUN_00553016


undefined4
FUN_00553016(short *param_1,short *param_2,int param_3,int param_4,int param_5,int param_6)

{
  short sVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int local_64;
  int local_54;
  int local_44;
  int local_38;
  int local_28;
  int local_24;
  int local_18;
  int local_14;
  
  iVar5 = (int)*param_1;
  if ((iVar5 < 1) || (0x1963 < iVar5)) {
    uVar2 = 0;
  }
  else {
    iVar3 = (int)*(short *)(&DAT_00d449d4 + iVar5 * 0x8c);
    if (param_3 == 0xf) {
      uVar2 = 0;
    }
    else {
      if (param_3 != -1) {
        iVar4 = FUN_0054cd07(param_1,0x11);
        if ((iVar4 >> ((byte)param_3 & 0x1f) & 1U) == 0) {
          return 0;
        }
        if ((param_3 == 6) || (param_3 == 7)) {
          if (param_3 == 6) {
            local_54 = 7;
          }
          else {
            local_54 = 6;
          }
          iVar6 = (int)*(short *)(param_5 + local_54 * 8);
          if ((0 < iVar6) && (iVar6 < 0x1964)) {
            sVar1 = *(short *)(&DAT_00d449d4 + iVar6 * 0x8c);
            iVar6 = FUN_0054cd07(param_5 + local_54 * 8,0x11);
            if ((iVar4 == 0x40) || (iVar6 == 0x40)) {
              if (iVar3 == 0x2e) {
                if (iVar6 != 0x80) {
                  return 0;
                }
              }
              else {
                if (sVar1 != 0x2e) {
                  return 0;
                }
                if (iVar4 != 0x80) {
                  return 0;
                }
              }
            }
          }
        }
      }
      if (((0x15 < param_4) && (param_4 < 0x1a)) || (param_4 == 0x20)) {
        param_4 = param_6;
      }
      local_28 = 0;
      if ((0x8fb < iVar5) && (iVar5 < 0x956)) {
        local_28 = (iVar5 + -0x8fc) % 0x1e;
      }
      iVar5 = param_4 % 10;
      if (((param_3 == 1) && (*param_1 != 0x2eb)) &&
         ((*param_1 != 0xdac &&
          ((((*param_1 != 0xdad && (*param_1 != 0xdae)) && (5 < iVar5)) && (*param_1 != 0xce7))))))
      {
        uVar2 = 0;
      }
      else if (((local_28 == 0x13) || (local_28 == 0x14)) && (iVar5 < 6)) {
        uVar2 = 0;
      }
      else {
        iVar4 = FUN_0054cd07(param_1,0x70);
        if (iVar4 == 1) {
          if (iVar5 < 6) {
            return 0;
          }
        }
        else if ((iVar4 == 2) && (5 < iVar5)) {
          return 0;
        }
        iVar4 = FUN_0054cd07(param_1,0x12);
        if ((iVar4 >> ((byte)(param_4 / 10) & 0x1f) & 1U) == 0) {
          if (iVar5 < 6) {
            return 0;
          }
          iVar4 = FUN_0054cd07(param_1,0x11);
          if (((iVar4 != 0x40) && (iVar4 != 0x80)) && (iVar4 != 0xc0)) {
            return 0;
          }
        }
        if (((iVar3 % 10 < 9) || (0x27 < iVar3)) || (5 < iVar5)) {
          local_44 = FUN_0054cd07(param_1,1);
          local_18 = FUN_0054cd07(param_1,0x16);
          local_24 = FUN_0054cd07(param_1,0x17);
          local_38 = FUN_0054cd07(param_1,0x18);
          local_14 = FUN_0054cd07(param_1,0x19);
          iVar3 = FUN_0054cd07(param_1,0x15);
          iVar3 = iVar3 % 10;
          if ((param_3 == 7) && (iVar3 != 0)) {
            local_64 = 100;
            if ((iVar3 / 10 == 0) && (1 < iVar3)) {
              local_64 = 0x82;
            }
            else if ((iVar3 / 10 == 6) && (1 < iVar3)) {
              local_64 = 0x96;
            }
            local_44 = (local_44 * local_64) / 100;
            local_18 = (local_18 * local_64) / 100;
            local_24 = (local_24 * local_64) / 100;
            local_38 = (local_38 * local_64) / 100;
            local_14 = (local_14 * local_64) / 100;
          }
          if (iVar5 < 6) {
            if (*param_2 < local_44) {
              return 0;
            }
            if (param_2[8] < local_18) {
              return 0;
            }
            if (param_2[9] < local_24) {
              return 0;
            }
            if (param_2[10] < local_38) {
              return 0;
            }
            if (param_2[0xb] < local_14) {
              return 0;
            }
          }
          uVar2 = 1;
        }
        else {
          uVar2 = 0;
        }
      }
    }
  }
  return uVar2;
}

