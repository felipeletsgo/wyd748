// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b6d70 | Name: FUN_004b6d70


undefined4 __thiscall FUN_004b6d70(int param_1,uint param_2,char param_3)

{
  uint uVar1;
  char *pcVar2;
  undefined4 uVar3;
  
  uVar1 = FID_conflict_max_size();
  if (uVar1 < param_2) {
    FUN_0059e2a0();
  }
  if (((*(int *)(param_1 + 4) != 0) &&
      (pcVar2 = (char *)FUN_004b6eb0(*(undefined4 *)(param_1 + 4)), *pcVar2 != '\0')) &&
     (pcVar2 = (char *)FUN_004b6eb0(*(undefined4 *)(param_1 + 4)), *pcVar2 != -1)) {
    if (param_2 == 0) {
      pcVar2 = (char *)FUN_004b6eb0(*(undefined4 *)(param_1 + 4));
      *pcVar2 = *pcVar2 + -1;
      FUN_004b6980(0);
      return 0;
    }
    FUN_004b7170(param_2);
    return 1;
  }
  if (param_2 == 0) {
    if (param_3 == '\0') {
      if (*(int *)(param_1 + 4) != 0) {
        FUN_004b6d20(0);
      }
    }
    else {
      FUN_004b6980(1);
    }
    uVar3 = 0;
  }
  else {
    if ((param_3 == '\0') ||
       ((*(uint *)(param_1 + 0xc) < 0x20 && (param_2 <= *(uint *)(param_1 + 0xc))))) {
      if ((param_3 == '\0') && (*(uint *)(param_1 + 0xc) < param_2)) {
        FUN_004b7170(param_2);
      }
    }
    else {
      FUN_004b6980(1);
      FUN_004b7170(param_2);
    }
    uVar3 = 1;
  }
  return uVar3;
}

