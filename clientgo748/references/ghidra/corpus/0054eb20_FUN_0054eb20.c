// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054eb20 | Name: FUN_0054eb20


int FUN_0054eb20(int param_1,byte param_2)

{
  short sVar1;
  short sVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  uint uVar8;
  int local_70;
  int aiStack_4c [16];
  uint local_c;
  int local_8;
  
  local_8 = 0;
  if (param_2 == 0x1b) {
    local_8 = FUN_0054ede8(param_1,0x1b);
    if (((local_8 < 2) && (*(char *)(param_1 + 0x14) == '\x03')) &&
       ((*(uint *)(param_1 + 0x2dc) & 0x100000) != 0)) {
      local_8 = 2;
    }
  }
  else {
    for (local_c = 0; (int)local_c < 0x10; local_c = local_c + 1) {
      aiStack_4c[local_c] = 0;
      uVar3 = local_c;
      uVar8 = (uint)*(short *)(param_1 + 0x5c + local_c * 8);
      if ((uVar8 != 0) || (local_c == 7)) {
        if ((0 < (int)local_c) && ((int)local_c < 6)) {
          aiStack_4c[local_c] = (int)*(short *)(&DAT_00d449d4 + uVar8 * 0x8c);
          uVar8 = uVar3;
        }
        if (((param_2 != 2) || (local_c != 6)) && ((param_2 != 0x3c || (local_c != 7)))) {
          if ((local_c == 7) && (uVar8 = (uint)param_2, uVar8 == 2)) {
            iVar4 = FUN_0054cd07(param_1 + 0x8c,param_2);
            iVar5 = FUN_0054cd07(param_1 + 0x8c,0x49);
            iVar4 = iVar4 + iVar5;
            iVar5 = FUN_0054cd07(param_1 + 0x94,param_2);
            iVar6 = FUN_0054cd07(param_1 + 0x94,0x49);
            iVar5 = iVar5 + iVar6;
            iVar7 = (int)*(short *)(param_1 + 0x8c);
            iVar6 = (int)*(short *)(param_1 + 0x94);
            sVar1 = 0;
            if ((0 < iVar7) && (iVar7 < 0x1964)) {
              sVar1 = *(short *)(&DAT_00d449d4 + iVar7 * 0x8c);
            }
            sVar2 = 0;
            if ((0 < iVar6) && (iVar6 < 0x1964)) {
              sVar2 = *(short *)(&DAT_00d449d4 + iVar6 * 0x8c);
            }
            if ((sVar1 == 0) || (sVar2 == 0)) {
              if (iVar5 < iVar4) {
                local_8 = local_8 + iVar4;
              }
              else {
                local_8 = local_8 + iVar5;
              }
            }
            else if ((sVar1 == 0x2f) && (sVar2 == 0x2d)) {
              local_8 = local_8 + iVar4;
            }
            else {
              if (sVar1 == sVar2) {
                local_70 = 0x1e;
              }
              else {
                local_70 = 0x14;
              }
              if (iVar5 < iVar4) {
                local_8 = local_8 + iVar4 + (iVar5 * local_70) / 100;
              }
              else {
                local_8 = local_8 + iVar5 + (iVar4 * local_70) / 100;
              }
            }
          }
          else {
            iVar4 = FUN_0054cd07(param_1 + 0x5c + local_c * 8,CONCAT31((int3)(uVar8 >> 8),param_2));
            local_8 = local_8 + iVar4;
          }
        }
      }
    }
    if (local_8 < 0) {
      local_8 = 0;
    }
  }
  return local_8;
}

