// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00588794 | Name: FUN_00588794


void FUN_00588794(int param_1,int param_2,int param_3,int param_4,int param_5,int param_6,
                 byte *param_7)

{
  byte bVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  int *piVar7;
  int local_22c [128];
  int local_2c;
  int local_28;
  int local_24;
  byte *local_20;
  int local_1c;
  int local_18;
  int *local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_c = 0;
  piVar7 = local_22c;
  for (iVar3 = 0x80; iVar3 != 0; iVar3 = iVar3 + -1) {
    *piVar7 = 0x7fffffff;
    piVar7 = piVar7 + 1;
  }
  if (0 < param_5) {
    do {
      bVar1 = *(byte *)(local_c + param_6);
      uVar6 = (uint)bVar1;
      piVar7 = *(int **)(param_1 + 0x74);
      iVar2 = param_3 - (uint)*(byte *)(piVar7[1] + uVar6);
      iVar3 = iVar2 * 3;
      iVar5 = param_4 - (uint)*(byte *)(piVar7[2] + uVar6);
      iVar4 = (param_2 - (uint)*(byte *)(uVar6 + *piVar7)) * 2;
      iVar3 = iVar5 * iVar5 + iVar3 * iVar3 + iVar4 * iVar4;
      local_14 = local_22c;
      local_8 = (iVar4 + 8) * 0x20;
      local_20 = param_7;
      local_28 = 4;
      do {
        local_2c = 8;
        local_1c = iVar3;
        local_10 = (iVar2 * 9 + 0x12) * 8;
        do {
          local_24 = 4;
          iVar4 = local_1c;
          local_18 = (iVar5 + 4) * 0x10;
          do {
            if (iVar4 < *local_14) {
              *local_14 = iVar4;
              *local_20 = bVar1;
            }
            iVar4 = iVar4 + local_18;
            local_18 = local_18 + 0x80;
            local_14 = local_14 + 1;
            local_20 = local_20 + 1;
            local_24 = local_24 + -1;
          } while (local_24 != 0);
          local_1c = local_1c + local_10;
          local_10 = local_10 + 0x120;
          local_2c = local_2c + -1;
        } while (local_2c != 0);
        iVar3 = iVar3 + local_8;
        local_8 = local_8 + 0x200;
        local_28 = local_28 + -1;
      } while (local_28 != 0);
      local_c = local_c + 1;
    } while (local_c < param_5);
  }
  return;
}

