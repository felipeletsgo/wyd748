// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00567afa | Name: FUN_00567afa


undefined4 FUN_00567afa(uint *param_1,uint param_2,int param_3,uint param_4,int param_5,int param_6)

{
  uint *puVar1;
  uint *puVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint *puVar6;
  int local_14;
  uint *local_8;
  
  uVar5 = param_4 * param_5 + param_2;
  if (param_2 < uVar5) {
    param_4 = param_2 + param_3 * 4;
    do {
      local_8 = param_1;
      if (param_2 < param_4) {
        local_14 = ((param_4 - param_2) - 1 >> 3) + 1;
        puVar6 = (uint *)(param_2 + 4);
        do {
          puVar2 = (uint *)((int)puVar6 + param_5);
          uVar3 = *puVar6;
          uVar4 = *(uint *)(param_5 + -4 + (int)puVar6);
          puVar1 = puVar6 + -1;
          puVar6 = puVar6 + 2;
          local_14 = local_14 + -1;
          *local_8 = ((*puVar1 & 0xff00ff) + (uVar4 & 0xff00ff) + (*puVar2 & 0xff00ff) + 0x20002 +
                      (uVar3 & 0xff00ff) & 0x3fc03fc |
                     (*puVar1 & 0xff00) + (uVar4 & 0xff00) + (*puVar2 & 0xff00) + 0x200 +
                     (uVar3 & 0xff00) & 0x3fc00) >> 2;
          local_8 = local_8 + 1;
        } while (local_14 != 0);
      }
      param_1 = (uint *)((int)param_1 + param_6);
      param_4 = param_4 + param_5 * 2;
      param_2 = param_2 + param_5 * 2;
    } while (param_2 < uVar5);
  }
  return 0;
}

