// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b32a2 | Name: FUN_004b32a2


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004b32a2(int param_1,float param_2)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  float10 fVar4;
  undefined4 uVar5;
  float fVar6;
  undefined1 local_b8 [12];
  undefined1 local_ac [12];
  undefined1 local_a0 [12];
  undefined4 local_94;
  undefined4 local_90;
  undefined4 local_8c;
  undefined4 local_48 [16];
  float local_8;
  
  if ((*(int *)(param_1 + 0x1af48) != 0) &&
     (iVar1 = (**(code **)(**(int **)(DAT_005ccf9c + 0x2a3e4) + 0x90))
                        (*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),0,0,3,0,0x3f800000,0), -1 < iVar1))
  {
    local_8 = 0.0;
    if (param_2 < _DAT_005a3660) {
      local_8 = 1.0;
    }
    FUN_00430f50(local_8 * _DAT_005a442c +
                 (*(float *)(*(int *)(param_1 + 0x1af48) + 0x28) - _DAT_005a4214),
                 param_2 + *(float *)(*(int *)(param_1 + 0x1af48) + 0x30) + local_8,
                 *(undefined4 *)(*(int *)(param_1 + 0x1af48) + 0x2c));
    fVar4 = (float10)FUN_00423b00((local_8 * _DAT_005a4148 * _DAT_005a40f0) / _DAT_005a3834 +
                                  *(float *)(*(int *)(param_1 + 0x1af48) + 0x34) + _DAT_005a4148);
    fVar6 = (float)(fVar4 * (float10)_DAT_005a4678);
    uVar5 = 0x3e4ccccd;
    fVar4 = (float10)FUN_00493bf0((local_8 * _DAT_005a4148 * _DAT_005a40f0) / _DAT_005a3834 +
                                  *(float *)(*(int *)(param_1 + 0x1af48) + 0x34) + _DAT_005a4148,
                                  0x3e4ccccd,fVar6);
    FUN_00430f50((float)(fVar4 * (float10)_DAT_005a4678),uVar5,fVar6);
    puVar2 = (undefined4 *)FUN_004b3a50(local_b8,local_a0);
    local_94 = *puVar2;
    local_90 = puVar2[1];
    local_8c = puVar2[2];
    puVar2 = (undefined4 *)(DAT_005ccf9c + 0x2a62c);
    puVar3 = local_48;
    for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
      *puVar3 = *puVar2;
      puVar2 = puVar2 + 1;
      puVar3 = puVar3 + 1;
    }
    FUN_00430f10();
    FUN_00430f50(0,0x3f800000,0);
    FUN_005640d2(DAT_005ccf9c + 0x2a62c,&local_94,local_a0,local_ac);
    (**(code **)(**(int **)(DAT_005ccf9c + 0x2a3e4) + 0x94))
              (*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),2,DAT_005ccf9c + 0x2a62c);
    (**(code **)(**(int **)(param_1 + 0x1af48) + 0x24))();
    puVar2 = local_48;
    puVar3 = (undefined4 *)(DAT_005ccf9c + 0x2a62c);
    for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
      *puVar3 = *puVar2;
      puVar2 = puVar2 + 1;
      puVar3 = puVar3 + 1;
    }
  }
  return;
}

