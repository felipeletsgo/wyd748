// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005787b9 | Name: FUN_005787b9


void FUN_005787b9(int param_1)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int local_8;
  
  iVar1 = *(int *)(param_1 + 4);
  iVar4 = 0;
  iVar3 = 0;
  for (piVar2 = *(int **)(iVar1 + 0x40); piVar2 != (int *)0x0; piVar2 = (int *)piVar2[9]) {
    if (*piVar2 == 0) {
      iVar4 = iVar4 + piVar2[3] * piVar2[2];
      iVar3 = iVar3 + piVar2[1] * piVar2[2];
    }
  }
  for (piVar2 = *(int **)(iVar1 + 0x44); piVar2 != (int *)0x0; piVar2 = (int *)piVar2[9]) {
    if (*piVar2 == 0) {
      iVar4 = iVar4 + piVar2[3] * piVar2[2] * 0x80;
      iVar3 = iVar3 + piVar2[1] * piVar2[2] * 0x80;
    }
  }
  if (0 < iVar4) {
    local_8 = FUN_005840d5(param_1,iVar4,iVar3,*(undefined4 *)(iVar1 + 0x48));
    if (local_8 < iVar3) {
      local_8 = local_8 / iVar4;
      if (local_8 < 1) {
        local_8 = 1;
      }
    }
    else {
      local_8 = 1000000000;
    }
    for (piVar2 = *(int **)(iVar1 + 0x40); piVar2 != (int *)0x0; piVar2 = (int *)piVar2[9]) {
      if (*piVar2 == 0) {
        iVar3 = piVar2[1];
        if (local_8 < (int)((iVar3 - 1U) / (uint)piVar2[3] + 1)) {
          piVar2[4] = piVar2[3] * local_8;
          FUN_005840da(param_1,piVar2 + 10,piVar2[2] * iVar3);
          *(undefined1 *)((int)piVar2 + 0x22) = 1;
        }
        else {
          piVar2[4] = iVar3;
        }
        iVar3 = FUN_005785c7(param_1,1,piVar2[2],piVar2[4]);
        *piVar2 = iVar3;
        iVar3 = *(int *)(iVar1 + 0x4c);
        piVar2[6] = 0;
        piVar2[7] = 0;
        *(undefined1 *)((int)piVar2 + 0x21) = 0;
        piVar2[5] = iVar3;
      }
    }
    for (piVar2 = *(int **)(iVar1 + 0x44); piVar2 != (int *)0x0; piVar2 = (int *)piVar2[9]) {
      if (*piVar2 == 0) {
        iVar3 = piVar2[1];
        if (local_8 < (int)((iVar3 - 1U) / (uint)piVar2[3] + 1)) {
          piVar2[4] = piVar2[3] * local_8;
          FUN_005840da(param_1,piVar2 + 10,piVar2[2] * iVar3 * 0x80);
          *(undefined1 *)((int)piVar2 + 0x22) = 1;
        }
        else {
          piVar2[4] = iVar3;
        }
        iVar3 = FUN_00578659(param_1,1,piVar2[2],piVar2[4]);
        *piVar2 = iVar3;
        iVar3 = *(int *)(iVar1 + 0x4c);
        piVar2[6] = 0;
        piVar2[7] = 0;
        *(undefined1 *)((int)piVar2 + 0x21) = 0;
        piVar2[5] = iVar3;
      }
    }
  }
  return;
}

