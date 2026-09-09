// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00492ad6 | Name: FUN_00492ad6


undefined4 FUN_00492ad6(undefined4 *param_1)

{
  bool bVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  int local_c;
  
  bVar1 = false;
  local_c = 0;
  while( true ) {
    if (0xb < local_c) goto LAB_00492b48;
    if (*(int *)(DAT_013b71e8 + 0x9ec + local_c * 0x34) == param_1[3]) break;
    local_c = local_c + 1;
  }
  puVar3 = param_1;
  puVar4 = (undefined4 *)(DAT_013b71e8 + 0x9e0 + local_c * 0x34);
  for (iVar2 = 0xd; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar4 = *puVar3;
    puVar3 = puVar3 + 1;
    puVar4 = puVar4 + 1;
  }
  bVar1 = true;
LAB_00492b48:
  if (!bVar1) {
    for (local_c = 0; local_c < 0xc; local_c = local_c + 1) {
      if (*(int *)(DAT_013b71e8 + 0x9ec + local_c * 0x34) == 0) {
        puVar3 = param_1;
        puVar4 = (undefined4 *)(DAT_013b71e8 + 0x9e0 + local_c * 0x34);
        for (iVar2 = 0xd; iVar2 != 0; iVar2 = iVar2 + -1) {
          *puVar4 = *puVar3;
          puVar3 = puVar3 + 1;
          puVar4 = puVar4 + 1;
        }
        bVar1 = true;
        break;
      }
    }
  }
  if (!bVar1) {
    puVar3 = (undefined4 *)(DAT_013b71e8 + 0x9e0);
    for (iVar2 = 0xd; iVar2 != 0; iVar2 = iVar2 + -1) {
      *puVar3 = *param_1;
      param_1 = param_1 + 1;
      puVar3 = puVar3 + 1;
    }
  }
  DAT_005b12c0 = 1;
  return 1;
}

