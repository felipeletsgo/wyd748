// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004770ad | Name: FUN_004770ad


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_004770ad(int param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined1 local_d4;
  undefined4 local_d3;
  int local_94;
  undefined1 local_90 [128];
  int local_10;
  int local_c;
  int local_8;
  
  if (((*(int *)(param_1 + 0x4c) == 0) || (DAT_005b892c != 2)) || (*(int *)(param_1 + 0x287fc) == 0)
     ) {
    uVar1 = 0;
  }
  else {
    local_8 = 0;
    local_c = 0;
    (**(code **)(**(int **)(param_1 + 0x28710) + 0x60))(0);
    (**(code **)(**(int **)(param_1 + 0x28710) + 0x80))(&DAT_005d049c,0);
    for (local_8 = 0; local_8 < 0x10; local_8 = local_8 + 1) {
      if (*(byte *)(*(int *)(param_1 + 0x4c) + 0x6a4 + local_8 * 8) < 0x29) {
        local_94 = (*(int *)(*(int *)(param_1 + 0x4c) + 0x6a8 + local_8 * 8) * 8 + -4) -
                   (uint)(param_2 - *(int *)(param_1 + 0x27480 + local_8 * 4)) / 1000;
        FUN_00477513(local_90,local_94,
                     *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x6a8 + local_8 * 8),local_8);
        local_10 = *(int *)(*(int *)(param_1 + 0x4c) + 0x6a8 + local_8 * 8);
        if ((((*(int *)(param_1 + 0x286d0 + local_8 * 4) != 0) && (local_8 != 0)) &&
            (*(char *)(*(int *)(param_1 + 0x4c) + 0x6a4 + local_8 * 8) != '\0')) ||
           ((local_8 == 0 && (0 < local_10)))) {
          (**(code **)(**(int **)(param_1 + 0x286d0 + local_8 * 4) + 0x70))
                    (*(float *)(*(int *)(param_1 + 0x27a44) + 0x4c) +
                     *(float *)(*(int *)(param_1 + 0x27a44) + 0x54) + _DAT_005a40f0 +
                     (float)(local_c * 0x17),0x40a00000);
          local_c = local_c + 1;
          if (*(int *)(*(int *)(param_1 + 0x286d0 + local_8 * 4) + 0x34) == 1) {
            local_d4 = DAT_005d04a0;
            puVar3 = &local_d3;
            for (iVar2 = 0xf; iVar2 != 0; iVar2 = iVar2 + -1) {
              *puVar3 = 0;
              puVar3 = puVar3 + 1;
            }
            *(undefined2 *)puVar3 = 0;
            *(undefined1 *)((int)puVar3 + 2) = 0;
            FUN_0058f078(&local_d4,s__s____s_005b86a8,
                         &DAT_005c0d38 +
                         (uint)*(byte *)(*(int *)(param_1 + 0x4c) + 0x6a4 + local_8 * 8) * 0x18,
                         local_90);
            (**(code **)(**(int **)(param_1 + 0x28710) + 0x80))(&local_d4,0);
            *(undefined4 *)(*(int *)(param_1 + 0x28710) + 0x4c) =
                 *(undefined4 *)(*(int *)(param_1 + 0x286d0 + local_8 * 4) + 0x4c);
            *(float *)(*(int *)(param_1 + 0x28710) + 0x50) =
                 *(float *)(*(int *)(param_1 + 0x286d0 + local_8 * 4) + 0x50) + _DAT_005a3c68;
            (**(code **)(**(int **)(param_1 + 0x28710) + 0x60))(1);
          }
          if (((*(int *)(param_1 + 0x28e84 + local_8 * 4) == 0) && (local_94 < 0xb)) &&
             (1 < local_94)) {
            *(int *)(param_1 + 0x28e84 + local_8 * 4) = param_2;
          }
          else if ((*(int *)(param_1 + 0x28e84 + local_8 * 4) == 0) || (9 < local_94)) {
            (**(code **)(**(int **)(param_1 + 0x286d0 + local_8 * 4) + 0x60))(1);
          }
          else if (local_10 * local_10 * 3 + 200U <=
                   (uint)(param_2 - *(int *)(param_1 + 0x28e84 + local_8 * 4))) {
            (**(code **)(**(int **)(param_1 + 0x286d0 + local_8 * 4) + 0x60))
                      (*(int *)(*(int *)(param_1 + 0x286d0 + local_8 * 4) + 0x28) == 0);
            *(int *)(param_1 + 0x28e84 + local_8 * 4) = param_2;
          }
          if (9 < local_10) {
            (**(code **)(**(int **)(param_1 + 0x286d0 + local_8 * 4) + 0x60))(1);
            *(undefined4 *)(param_1 + 0x28e84 + local_8 * 4) = 0;
          }
        }
      }
    }
    uVar1 = 1;
  }
  return uVar1;
}

