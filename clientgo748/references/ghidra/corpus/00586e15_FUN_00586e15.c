// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00586e15 | Name: FUN_00586e15


void FUN_00586e15(int param_1,undefined4 *param_2,undefined4 *param_3,int *param_4)

{
  byte bVar1;
  undefined1 uVar2;
  int iVar3;
  undefined4 *puVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  undefined4 *puVar8;
  uint uVar9;
  undefined1 *puVar10;
  undefined4 *puVar11;
  int local_c;
  
  iVar3 = *param_4;
  iVar5 = *(int *)(param_1 + 0x19c) + *(int *)((int)param_2 + 4);
  bVar1 = *(byte *)(iVar5 + 0x8c);
  uVar7 = (uint)bVar1;
  uVar6 = (uint)*(byte *)(iVar5 + 0x96);
  local_c = 0;
  if (0 < *(int *)(param_1 + 0x110)) {
    param_2 = param_3;
    do {
      puVar10 = (undefined1 *)*param_2;
      puVar4 = *(undefined4 **)(iVar3 + local_c * 4);
      puVar8 = (undefined4 *)(*(int *)(param_1 + 0x5c) + (int)puVar4);
      while (puVar4 < puVar8) {
        uVar2 = *puVar10;
        puVar10 = puVar10 + 1;
        if (uVar7 != 0) {
          puVar11 = puVar4;
          for (uVar9 = (uint)(bVar1 >> 2); uVar9 != 0; uVar9 = uVar9 - 1) {
            *puVar11 = CONCAT22(CONCAT11(uVar2,uVar2),CONCAT11(uVar2,uVar2));
            puVar11 = puVar11 + 1;
          }
          for (uVar9 = uVar7 & 3; uVar9 != 0; uVar9 = uVar9 - 1) {
            *(undefined1 *)puVar11 = uVar2;
            puVar11 = (undefined4 *)((int)puVar11 + 1);
          }
          puVar4 = (undefined4 *)((int)puVar4 + uVar7);
        }
      }
      if (1 < uVar6) {
        FUN_00577e59(iVar3,local_c,iVar3,local_c + 1,uVar6 - 1,*(undefined4 *)(param_1 + 0x5c));
      }
      local_c = local_c + uVar6;
      param_2 = param_2 + 1;
    } while (local_c < *(int *)(param_1 + 0x110));
  }
  return;
}

