// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00409872 | Name: FUN_00409872


void __thiscall
FUN_00409872(int param_1,undefined4 param_2,float param_3,undefined4 param_4,undefined4 param_5,
            undefined4 param_6)

{
  int *piVar1;
  bool bVar2;
  int iVar3;
  undefined4 *puVar4;
  char *_Str;
  size_t sVar5;
  float10 extraout_ST0;
  float10 extraout_ST1;
  int local_24;
  int local_10;
  
  FUN_00401637(param_2,param_3,param_4,param_5,param_6);
  if (*(int *)(param_1 + 0x210) - *(int *)(param_1 + 0x1f8) < *(int *)(param_1 + 500)) {
    local_24 = *(int *)(param_1 + 0x210) - *(int *)(param_1 + 0x1f8);
  }
  else {
    local_24 = *(int *)(param_1 + 500);
  }
  bVar2 = false;
  for (local_10 = 0; local_10 < local_24; local_10 = local_10 + 1) {
    iVar3 = FUN_013c0c80(param_5,(uint)(*(int *)(param_1 + 0x1f8) + local_10 ==
                                       *(int *)(param_1 + 0x1fc)) & *(uint *)(param_1 + 0x3c));
    puVar4 = (undefined4 *)
             FUN_00430f20(param_3 + *(float *)(param_1 + 0x4c),
                          (float)((extraout_ST0 * (float10)*(float *)(iVar3 + 0x58)) /
                                  (float10)*(int *)(param_1 + 500) + extraout_ST1));
    (**(code **)(**(int **)(param_1 + 0x214 + (*(int *)(param_1 + 0x1f8) + local_10) * 4) + 0x58))
              (param_2,*puVar4,puVar4[1]);
    if ((!bVar2) &&
       (piVar1 = *(int **)(param_1 + 0x214 + (*(int *)(param_1 + 0x1f8) + local_10) * 4),
       piVar1 != (int *)0x0)) {
      _Str = (char *)(**(code **)(*piVar1 + 0x88))();
      sVar5 = _strlen(_Str);
      if (sVar5 != 0) {
        bVar2 = true;
      }
    }
  }
  if ((*(char *)(param_1 + 0x1e9) == '\x01') && (*(int *)(param_1 + 0x208) != 0)) {
    (**(code **)(**(int **)(param_1 + 0x208) + 0x70))
              (0,((float)((int)*(short *)(param_1 + 0x1ea) - *(int *)(param_1 + 0x1f8)) *
                 *(float *)(param_1 + 0x58)) / (float)*(int *)(param_1 + 500));
    if (((int)*(short *)(param_1 + 0x1ea) - *(int *)(param_1 + 0x1f8) < 0) ||
       (local_24 < (int)*(short *)(param_1 + 0x1ea) - *(int *)(param_1 + 0x1f8))) {
      (**(code **)(**(int **)(param_1 + 0x208) + 0x60))(0);
    }
    else {
      (**(code **)(**(int **)(param_1 + 0x208) + 0x60))(1);
    }
  }
  return;
}

