// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005683e5 | Name: FUN_005683e5


undefined4 __fastcall FUN_005683e5(int *param_1)

{
  byte *pbVar1;
  int iVar2;
  byte *pbVar3;
  undefined1 *puVar4;
  byte *pbVar5;
  byte *pbVar6;
  int iVar7;
  undefined1 *local_c;
  
  puVar4 = *(undefined1 **)(param_1[1] + 0x18);
  iVar2 = *param_1;
  pbVar3 = *(byte **)(iVar2 + 0x18);
  iVar7 = *(int *)(iVar2 + 0x1050);
  pbVar5 = pbVar3 + *(int *)(iVar2 + 0x105c) * iVar7;
  for (; pbVar3 < pbVar5; pbVar3 = pbVar3 + iVar7 * 2) {
    iVar2 = *(int *)(iVar2 + 0x1058);
    pbVar6 = pbVar3 + iVar7;
    local_c = puVar4;
    for (pbVar1 = pbVar3; pbVar1 < pbVar3 + iVar2; pbVar1 = pbVar1 + 2) {
      *local_c = (char)((uint)pbVar6[1] + (uint)pbVar1[1] + (uint)*pbVar6 + 2 + (uint)*pbVar1 >> 2);
      pbVar6 = pbVar6 + 2;
      local_c = local_c + 1;
    }
    puVar4 = puVar4 + *(int *)(param_1[1] + 0x1050);
    iVar2 = *param_1;
    iVar7 = *(int *)(iVar2 + 0x1050);
  }
  return 0;
}

