// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013d4000 | Name: FUN_013d4000


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_013d4000(void)

{
  int iVar1;
  int extraout_EDX;
  int extraout_EDX_00;
  int extraout_EDX_01;
  int extraout_EDX_02;
  int extraout_EDX_03;
  int unaff_EBP;
  int *piVar2;
  int *piVar3;
  int *piVar4;
  
  piVar3 = *(int **)(unaff_EBP + -0x200);
  iVar1 = *(int *)piVar3[0x25];
  if (iVar1 == 0x1f) {
    if (*piVar3 == 4) {
      FUN_013d43fb();
    }
  }
  else if (iVar1 == 0x30) {
    if (*piVar3 == 1) {
      FUN_013d43fb();
      piVar2 = piVar3 + 0x16;
      piVar4 = (int *)(extraout_EDX + 0x74);
      for (iVar1 = 4; iVar1 != 0; iVar1 = iVar1 + -1) {
        *piVar4 = *piVar2;
        piVar2 = piVar2 + 1;
        piVar4 = piVar4 + 1;
      }
      piVar2 = piVar3 + 0x1a;
      piVar4 = (int *)(extraout_EDX + 0x84);
      for (iVar1 = 4; iVar1 != 0; iVar1 = iVar1 + -1) {
        *piVar4 = *piVar2;
        piVar2 = piVar2 + 1;
        piVar4 = piVar4 + 1;
      }
      *(uint *)(extraout_EDX + 0x84) = *(uint *)(extraout_EDX + 0x84) ^ 0x80000000;
      *(uint *)(extraout_EDX + 0x88) = *(uint *)(extraout_EDX + 0x88) ^ 0x80000000;
      *(uint *)(extraout_EDX + 0x8c) = *(uint *)(extraout_EDX + 0x8c) ^ 0x80000000;
      *(uint *)(extraout_EDX + 0x90) = *(uint *)(extraout_EDX + 0x90) ^ 0x80000000;
      piVar3 = piVar3 + 0x12;
      piVar2 = (int *)(extraout_EDX + 0x94);
      for (iVar1 = 4; iVar1 != 0; iVar1 = iVar1 + -1) {
        *piVar2 = *piVar3;
        piVar3 = piVar3 + 1;
        piVar2 = piVar2 + 1;
      }
      *(uint *)(extraout_EDX + 0x94) = *(uint *)(extraout_EDX + 0x94) ^ 0x80000000;
      *(uint *)(extraout_EDX + 0x98) = *(uint *)(extraout_EDX + 0x98) ^ 0x80000000;
      *(uint *)(extraout_EDX + 0x9c) = *(uint *)(extraout_EDX + 0x9c) ^ 0x80000000;
      *(uint *)(extraout_EDX + 0xa0) = *(uint *)(extraout_EDX + 0xa0) ^ 0x80000000;
    }
  }
  else if (iVar1 == 0x31) {
    if (*piVar3 == 3) {
      FUN_013d43fb();
      piVar2 = piVar3 + 0x12;
      piVar4 = (int *)(extraout_EDX_00 + 0x74);
      for (iVar1 = 4; iVar1 != 0; iVar1 = iVar1 + -1) {
        *piVar4 = *piVar2;
        piVar2 = piVar2 + 1;
        piVar4 = piVar4 + 1;
      }
      *(uint *)(extraout_EDX_00 + 0x74) = *(uint *)(extraout_EDX_00 + 0x74) ^ 0x80000000;
      *(uint *)(extraout_EDX_00 + 0x78) = *(uint *)(extraout_EDX_00 + 0x78) ^ 0x80000000;
      *(uint *)(extraout_EDX_00 + 0x7c) = *(uint *)(extraout_EDX_00 + 0x7c) ^ 0x80000000;
      *(uint *)(extraout_EDX_00 + 0x80) = *(uint *)(extraout_EDX_00 + 0x80) ^ 0x80000000;
      piVar2 = piVar3 + 0x1a;
      piVar4 = (int *)(extraout_EDX_00 + 0x84);
      for (iVar1 = 4; iVar1 != 0; iVar1 = iVar1 + -1) {
        *piVar4 = *piVar2;
        piVar2 = piVar2 + 1;
        piVar4 = piVar4 + 1;
      }
      *(uint *)(extraout_EDX_00 + 0x84) = *(uint *)(extraout_EDX_00 + 0x84) ^ 0x80000000;
      *(uint *)(extraout_EDX_00 + 0x88) = *(uint *)(extraout_EDX_00 + 0x88) ^ 0x80000000;
      *(uint *)(extraout_EDX_00 + 0x8c) = *(uint *)(extraout_EDX_00 + 0x8c) ^ 0x80000000;
      *(uint *)(extraout_EDX_00 + 0x90) = *(uint *)(extraout_EDX_00 + 0x90) ^ 0x80000000;
      piVar3 = piVar3 + 0x16;
      piVar2 = (int *)(extraout_EDX_00 + 0x94);
      for (iVar1 = 4; iVar1 != 0; iVar1 = iVar1 + -1) {
        *piVar2 = *piVar3;
        piVar3 = piVar3 + 1;
        piVar2 = piVar2 + 1;
      }
      *(uint *)(extraout_EDX_00 + 0x94) = *(uint *)(extraout_EDX_00 + 0x94) ^ 0x80000000;
      *(uint *)(extraout_EDX_00 + 0x98) = *(uint *)(extraout_EDX_00 + 0x98) ^ 0x80000000;
      *(uint *)(extraout_EDX_00 + 0x9c) = *(uint *)(extraout_EDX_00 + 0x9c) ^ 0x80000000;
      *(uint *)(extraout_EDX_00 + 0xa0) = *(uint *)(extraout_EDX_00 + 0xa0) ^ 0x80000000;
    }
  }
  else if (iVar1 == 0x32) {
    if (*piVar3 == 2) {
      FUN_013d43fb();
      piVar2 = piVar3 + 0x12;
      piVar4 = (int *)(extraout_EDX_02 + 0x74);
      for (iVar1 = 4; iVar1 != 0; iVar1 = iVar1 + -1) {
        *piVar4 = *piVar2;
        piVar2 = piVar2 + 1;
        piVar4 = piVar4 + 1;
      }
      *(uint *)(extraout_EDX_02 + 0x74) = *(uint *)(extraout_EDX_02 + 0x74) ^ 0x80000000;
      *(uint *)(extraout_EDX_02 + 0x78) = *(uint *)(extraout_EDX_02 + 0x78) ^ 0x80000000;
      *(uint *)(extraout_EDX_02 + 0x7c) = *(uint *)(extraout_EDX_02 + 0x7c) ^ 0x80000000;
      *(uint *)(extraout_EDX_02 + 0x80) = *(uint *)(extraout_EDX_02 + 0x80) ^ 0x80000000;
      piVar2 = piVar3 + 0x1a;
      piVar4 = (int *)(extraout_EDX_02 + 0x84);
      for (iVar1 = 4; iVar1 != 0; iVar1 = iVar1 + -1) {
        *piVar4 = *piVar2;
        piVar2 = piVar2 + 1;
        piVar4 = piVar4 + 1;
      }
      *(uint *)(extraout_EDX_02 + 0x84) = *(uint *)(extraout_EDX_02 + 0x84) ^ 0x80000000;
      *(uint *)(extraout_EDX_02 + 0x88) = *(uint *)(extraout_EDX_02 + 0x88) ^ 0x80000000;
      *(uint *)(extraout_EDX_02 + 0x8c) = *(uint *)(extraout_EDX_02 + 0x8c) ^ 0x80000000;
      *(uint *)(extraout_EDX_02 + 0x90) = *(uint *)(extraout_EDX_02 + 0x90) ^ 0x80000000;
      piVar3 = piVar3 + 0x16;
      piVar2 = (int *)(extraout_EDX_02 + 0x94);
      for (iVar1 = 4; iVar1 != 0; iVar1 = iVar1 + -1) {
        *piVar2 = *piVar3;
        piVar3 = piVar3 + 1;
        piVar2 = piVar2 + 1;
      }
      *(uint *)(extraout_EDX_02 + 0x94) = *(uint *)(extraout_EDX_02 + 0x94) ^ 0x80000000;
      *(uint *)(extraout_EDX_02 + 0x98) = *(uint *)(extraout_EDX_02 + 0x98) ^ 0x80000000;
      *(uint *)(extraout_EDX_02 + 0x9c) = *(uint *)(extraout_EDX_02 + 0x9c) ^ 0x80000000;
      *(uint *)(extraout_EDX_02 + 0xa0) = *(uint *)(extraout_EDX_02 + 0xa0) ^ 0x80000000;
    }
  }
  else if (iVar1 == 0x33) {
    if (*piVar3 == 4) {
      FUN_013d43fb();
      piVar2 = piVar3 + 0x1a;
      piVar4 = (int *)(extraout_EDX_03 + 0x84);
      for (iVar1 = 4; iVar1 != 0; iVar1 = iVar1 + -1) {
        *piVar4 = *piVar2;
        piVar2 = piVar2 + 1;
        piVar4 = piVar4 + 1;
      }
      piVar3 = piVar3 + 0x16;
      piVar2 = (int *)(extraout_EDX_03 + 0x94);
      for (iVar1 = 4; iVar1 != 0; iVar1 = iVar1 + -1) {
        *piVar2 = *piVar3;
        piVar3 = piVar3 + 1;
        piVar2 = piVar2 + 1;
      }
      *(uint *)(extraout_EDX_03 + 0x94) = *(uint *)(extraout_EDX_03 + 0x94) ^ 0x80000000;
      *(uint *)(extraout_EDX_03 + 0x98) = *(uint *)(extraout_EDX_03 + 0x98) ^ 0x80000000;
      *(uint *)(extraout_EDX_03 + 0x9c) = *(uint *)(extraout_EDX_03 + 0x9c) ^ 0x80000000;
      *(uint *)(extraout_EDX_03 + 0xa0) = *(uint *)(extraout_EDX_03 + 0xa0) ^ 0x80000000;
    }
  }
  else if ((iVar1 == 0x3b) && (*piVar3 == 3)) {
    FUN_013d43fb();
    *(float *)(extraout_EDX_01 + 0x74) =
         (float)piVar3[0x16] * _DAT_013d3210 + (float)piVar3[0x12] * _DAT_013d320c;
    *(float *)(extraout_EDX_01 + 0x84) =
         (float)piVar3[0x16] * _DAT_013d320c + (float)piVar3[0x12] * _DAT_013d3214;
    *(float *)(extraout_EDX_01 + 0x78) =
         (float)piVar3[0x17] * _DAT_013d3210 + (float)piVar3[0x13] * _DAT_013d320c;
    *(float *)(extraout_EDX_01 + 0x88) =
         (float)piVar3[0x17] * _DAT_013d320c + (float)piVar3[0x13] * _DAT_013d3214;
    *(float *)(extraout_EDX_01 + 0x7c) =
         (float)piVar3[0x18] * _DAT_013d3210 + (float)piVar3[0x14] * _DAT_013d320c;
    *(float *)(extraout_EDX_01 + 0x8c) =
         (float)piVar3[0x18] * _DAT_013d320c + (float)piVar3[0x14] * _DAT_013d3214;
    *(float *)(extraout_EDX_01 + 0x80) =
         (float)piVar3[0x19] * _DAT_013d3210 + (float)piVar3[0x15] * _DAT_013d320c;
    *(float *)(extraout_EDX_01 + 0x90) =
         (float)piVar3[0x19] * _DAT_013d320c + (float)piVar3[0x15] * _DAT_013d3214;
  }
  FUN_004c3642();
  return;
}

