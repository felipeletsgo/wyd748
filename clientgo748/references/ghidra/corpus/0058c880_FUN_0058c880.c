// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058c880 | Name: FUN_0058c880


int FUN_0058c880(undefined1 *param_1,int param_2,int param_3)

{
  undefined4 *puVar1;
  int iVar2;
  undefined1 *puVar3;
  undefined4 uVar4;
  uint uVar5;
  undefined4 *puVar6;
  uint uVar7;
  undefined4 *puVar8;
  undefined4 *local_c;
  
  iVar2 = (int)param_1;
  puVar1 = *(undefined4 **)((int)param_1 + 0x30);
  local_c = *(undefined4 **)(param_2 + 0xc);
  puVar6 = *(undefined4 **)((int)param_1 + 0x34);
  if (puVar6 < puVar1) {
    puVar6 = *(undefined4 **)((int)param_1 + 0x2c);
  }
  uVar5 = *(uint *)(param_2 + 0x10);
  uVar7 = (int)puVar6 - (int)puVar1;
  if (uVar5 < (uint)((int)puVar6 - (int)puVar1)) {
    uVar7 = uVar5;
  }
  if ((uVar7 != 0) && (param_3 == -5)) {
    param_3 = 0;
  }
  *(int *)(param_2 + 0x14) = *(int *)(param_2 + 0x14) + uVar7;
  *(uint *)(param_2 + 0x10) = uVar5 - uVar7;
  if (*(code **)((int)param_1 + 0x38) != (code *)0x0) {
    uVar4 = (**(code **)((int)param_1 + 0x38))(*(undefined4 *)((int)param_1 + 0x3c),puVar1,uVar7);
    *(undefined4 *)((int)param_1 + 0x3c) = uVar4;
    *(undefined4 *)(param_2 + 0x30) = uVar4;
  }
  puVar6 = puVar1;
  puVar8 = local_c;
  for (uVar5 = uVar7 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
    *puVar8 = *puVar6;
    puVar6 = puVar6 + 1;
    puVar8 = puVar8 + 1;
  }
  local_c = (undefined4 *)((int)local_c + uVar7);
  for (uVar5 = uVar7 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
    *(undefined1 *)puVar8 = *(undefined1 *)puVar6;
    puVar6 = (undefined4 *)((int)puVar6 + 1);
    puVar8 = (undefined4 *)((int)puVar8 + 1);
  }
  puVar3 = (undefined1 *)((int)puVar1 + uVar7);
  if ((undefined1 *)((int)puVar1 + uVar7) == *(undefined1 **)((int)param_1 + 0x2c)) {
    puVar1 = *(undefined4 **)((int)param_1 + 0x28);
    if (*(undefined1 **)((int)param_1 + 0x34) == *(undefined1 **)((int)param_1 + 0x2c)) {
      *(undefined4 **)((int)param_1 + 0x34) = puVar1;
    }
    uVar5 = *(uint *)(param_2 + 0x10);
    uVar7 = *(int *)((int)param_1 + 0x34) - (int)puVar1;
    if (uVar5 < uVar7) {
      uVar7 = uVar5;
    }
    if ((uVar7 != 0) && (param_3 == -5)) {
      param_3 = 0;
    }
    *(int *)(param_2 + 0x14) = *(int *)(param_2 + 0x14) + uVar7;
    *(uint *)(param_2 + 0x10) = uVar5 - uVar7;
    if (*(code **)((int)param_1 + 0x38) != (code *)0x0) {
      uVar4 = (**(code **)((int)param_1 + 0x38))(*(undefined4 *)((int)param_1 + 0x3c),puVar1,uVar7);
      *(undefined4 *)((int)param_1 + 0x3c) = uVar4;
      *(undefined4 *)(param_2 + 0x30) = uVar4;
    }
    puVar6 = puVar1;
    puVar8 = local_c;
    for (uVar5 = uVar7 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
      *puVar8 = *puVar6;
      puVar6 = puVar6 + 1;
      puVar8 = puVar8 + 1;
    }
    local_c = (undefined4 *)((int)local_c + uVar7);
    for (uVar5 = uVar7 & 3; puVar3 = (undefined1 *)((int)puVar1 + uVar7), uVar5 != 0;
        uVar5 = uVar5 - 1) {
      *(undefined1 *)puVar8 = *(undefined1 *)puVar6;
      puVar6 = (undefined4 *)((int)puVar6 + 1);
      puVar8 = (undefined4 *)((int)puVar8 + 1);
    }
  }
  param_1 = puVar3;
  *(undefined4 **)(param_2 + 0xc) = local_c;
  *(undefined1 **)(iVar2 + 0x30) = param_1;
  return param_3;
}

