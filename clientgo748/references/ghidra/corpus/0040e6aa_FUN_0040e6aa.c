// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040e6aa | Name: FUN_0040e6aa


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_0040e6aa(int param_1,int *param_2,int param_3,int param_4)

{
  int iVar1;
  int local_14;
  int local_8;
  
  for (local_8 = 0; local_8 < param_2[0x77]; local_8 = local_8 + 1) {
    for (local_14 = 0; local_14 < param_2[0x76]; local_14 = local_14 + 1) {
      if ((param_3 + local_14 < *(int *)(param_1 + 0x1ec)) &&
         (param_4 + local_8 < *(int *)(param_1 + 0x1e8))) {
        *(undefined4 *)
         (*(int *)(param_1 + 0x1f0) +
         ((param_4 + local_8) * *(int *)(param_1 + 0x1ec) + param_3 + local_14) * 4) = 1;
      }
    }
  }
  (**(code **)(*param_2 + 0x94))(param_1);
  param_2[0x74] = param_3;
  param_2[0x75] = param_4;
  *(int **)(param_1 + 0x200 + *(int *)(param_1 + 0x1fc) * 4) = param_2;
  *(int *)(param_1 + 0x1fc) = *(int *)(param_1 + 0x1fc) + 1;
  iVar1 = FUN_004b14fd(param_2[0x27],0,180000);
  if ((iVar1 != 0) && ((float)param_2[0x77] * _DAT_005a40ec < *(float *)(iVar1 + 0x354))) {
    param_2[0x20] = (int)(((float)param_2[0x77] * _DAT_005a40ec) / *(float *)(iVar1 + 0x354));
  }
  return 1;
}

