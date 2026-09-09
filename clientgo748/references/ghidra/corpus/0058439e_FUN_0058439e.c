// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058439e | Name: FUN_0058439e


void FUN_0058439e(void)

{
  undefined4 *puVar1;
  int iVar2;
  uint uVar3;
  undefined4 *puVar4;
  int unaff_ESI;
  int *piVar5;
  int local_10;
  uint local_c;
  int local_8;
  
  local_8 = 0;
  iVar2 = *(int *)(unaff_ESI + 0x180);
  if (0 < *(int *)(unaff_ESI + 0x20)) {
    piVar5 = (int *)(*(int *)(unaff_ESI + 0xc4) + 0xc);
    do {
      uVar3 = piVar5[6] * *piVar5;
      local_10 = (int)uVar3 / *(int *)(unaff_ESI + 0x114);
      local_c = (uint)piVar5[8] % uVar3;
      if ((uint)piVar5[8] % uVar3 == 0) {
        local_c = uVar3;
      }
      if (local_8 == 0) {
        *(int *)(iVar2 + 0x48) = (int)(local_c - 1) / local_10 + 1;
      }
      local_10 = local_10 * 2;
      if (0 < local_10) {
        puVar1 = (undefined4 *)
                 (*(int *)(*(int *)(iVar2 + 0x38 + *(int *)(iVar2 + 0x40) * 4) + local_8 * 4) +
                 local_c * 4);
        puVar4 = puVar1;
        do {
          *puVar4 = puVar1[-1];
          puVar4 = puVar4 + 1;
          local_10 = local_10 + -1;
        } while (local_10 != 0);
      }
      local_8 = local_8 + 1;
      piVar5 = piVar5 + 0x15;
    } while (local_8 < *(int *)(unaff_ESI + 0x20));
  }
  return;
}

