// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00417920 | Name: FUN_00417920


undefined4
FUN_00417920(undefined1 *param_1,short *param_2,short *param_3,int param_4,int param_5,int param_6,
            int param_7)

{
  short sVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int local_64;
  int local_58;
  int local_48;
  int local_3c;
  int local_28;
  int local_24;
  int local_18;
  int local_14;
  
  if (param_1 == (undefined1 *)0x0) {
    uVar2 = 0;
  }
  else {
    *param_1 = 0;
    param_1[1] = 0;
    param_1[2] = 0;
    param_1[4] = 0;
    param_1[3] = 0;
    param_1[5] = 0;
    iVar5 = (int)*param_2;
    if ((iVar5 < 1) || (0x1963 < iVar5)) {
      uVar2 = 0;
    }
    else {
      iVar3 = (int)*(short *)(&DAT_00d449d4 + iVar5 * 0x8c);
      if (param_4 == 0xf) {
        uVar2 = 0;
      }
      else {
        if (param_4 != -1) {
          iVar4 = FUN_0054cd07(param_2,0x11);
          if ((iVar4 >> ((byte)param_4 & 0x1f) & 1U) == 0) {
            return 0;
          }
          if ((param_4 == 6) || (param_4 == 7)) {
            if (param_4 == 6) {
              local_58 = 7;
            }
            else {
              local_58 = 6;
            }
            iVar6 = (int)*(short *)(param_6 + local_58 * 8);
            if ((0 < iVar6) && (iVar6 < 0x1964)) {
              sVar1 = *(short *)(&DAT_00d449d4 + iVar6 * 0x8c);
              iVar6 = FUN_0054cd07(param_6 + local_58 * 8,0x11);
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
        if (((0x15 < param_5) && (param_5 < 0x1a)) || (param_5 == 0x20)) {
          param_5 = param_7;
        }
        local_28 = 0;
        if ((0x8fb < iVar5) && (iVar5 < 0x956)) {
          local_28 = (iVar5 + -0x8fc) % 0x1e;
        }
        iVar5 = param_5 % 10;
        if (((param_4 == 1) && (*param_2 != 0x2eb)) && (5 < iVar5)) {
          uVar2 = 0;
        }
        else if (((local_28 == 0x13) || (local_28 == 0x14)) && (iVar5 < 6)) {
          uVar2 = 0;
        }
        else {
          *param_1 = 1;
          iVar4 = FUN_0054cd07(param_2,0x70);
          if (iVar4 == 1) {
            if (iVar5 < 6) {
              *param_1 = 0;
            }
          }
          else if ((iVar4 == 2) && (5 < iVar5)) {
            *param_1 = 0;
          }
          iVar4 = FUN_0054cd07(param_2,0x12);
          iVar6 = FUN_0054cd07(param_2,0x11);
          if ((iVar4 >> ((byte)(param_5 / 10) & 0x1f) & 1U) == 0) {
            if (iVar5 < 6) {
              *param_1 = 0;
            }
            else if (((iVar6 != 0x40) && (iVar6 != 0x80)) && (iVar6 != 0xc0)) {
              *param_1 = 0;
            }
          }
          if (((param_5 < 0x20) && (param_5 % 10 != 1)) && (iVar6 == 2)) {
            *param_1 = 0;
          }
          if (((8 < iVar3 % 10) && (iVar3 < 0x28)) && (iVar5 < 6)) {
            *param_1 = 0;
          }
          local_48 = FUN_0054cd07(param_2,1);
          local_18 = FUN_0054cd07(param_2,0x16);
          local_24 = FUN_0054cd07(param_2,0x17);
          local_3c = FUN_0054cd07(param_2,0x18);
          local_14 = FUN_0054cd07(param_2,0x19);
          iVar5 = FUN_0054cd07(param_2,0x15);
          iVar5 = iVar5 % 10;
          if ((param_4 == 7) && (iVar5 != 0)) {
            local_64 = 100;
            if ((iVar5 / 10 == 0) && (1 < iVar5)) {
              local_64 = 0x82;
            }
            else if ((iVar5 / 10 == 6) && (1 < iVar5)) {
              local_64 = 0x96;
            }
            local_48 = (local_48 * local_64) / 100;
            local_18 = (local_18 * local_64) / 100;
            local_24 = (local_24 * local_64) / 100;
            local_3c = (local_3c * local_64) / 100;
            local_14 = (local_14 * local_64) / 100;
          }
          if (local_48 <= *param_3) {
            param_1[1] = 1;
          }
          if (local_18 <= param_3[8]) {
            param_1[2] = 1;
          }
          if (local_24 <= param_3[9]) {
            param_1[4] = 1;
          }
          if (local_3c <= param_3[10]) {
            param_1[3] = 1;
          }
          if (local_14 <= param_3[0xb]) {
            param_1[5] = 1;
          }
          uVar2 = 1;
        }
      }
    }
  }
  return uVar2;
}

