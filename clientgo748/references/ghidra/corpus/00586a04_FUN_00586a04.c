// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00586a04 | Name: FUN_00586a04


void FUN_00586a04(undefined4 *param_1)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  int iVar4;
  int local_8;
  
  iVar1 = (int)param_1;
  puVar2 = (undefined4 *)(*(code *)**(undefined4 **)((int)param_1 + 4))(param_1,1,0x54);
  local_8 = 0;
  *(undefined4 **)((int)param_1 + 0x198) = puVar2;
  *puVar2 = FUN_0058684d;
  if (0 < *(int *)((int)param_1 + 0x20)) {
    puVar2 = puVar2 + 0xb;
    param_1 = (undefined4 *)(*(int *)((int)param_1 + 0xc4) + 0x50);
    do {
      puVar3 = (undefined4 *)(*(code *)**(undefined4 **)(iVar1 + 4))(iVar1,1,0x100);
      *param_1 = puVar3;
      for (iVar4 = 0x40; iVar4 != 0; iVar4 = iVar4 + -1) {
        *puVar3 = 0;
        puVar3 = puVar3 + 1;
      }
      *puVar2 = 0xffffffff;
      local_8 = local_8 + 1;
      puVar2 = puVar2 + 1;
      param_1 = param_1 + 0x15;
    } while (local_8 < *(int *)(iVar1 + 0x20));
  }
  return;
}

