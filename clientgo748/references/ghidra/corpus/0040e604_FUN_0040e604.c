// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040e604 | Name: FUN_0040e604


undefined4 __thiscall
FUN_0040e604(int param_1,int param_2,int param_3,int param_4,int param_5,int param_6)

{
  undefined4 uVar1;
  int local_c;
  int local_8;
  
  if (param_2 == 0) {
    uVar1 = 0;
  }
  else if (*(int *)(param_1 + 0x1ec) < param_3 + param_5) {
    uVar1 = 0;
  }
  else if (*(int *)(param_1 + 0x1e8) < param_4 + param_6) {
    uVar1 = 0;
  }
  else {
    for (local_8 = 0; local_8 < param_6; local_8 = local_8 + 1) {
      for (local_c = 0; local_c < param_5; local_c = local_c + 1) {
        if (*(int *)(param_2 +
                    ((param_4 + local_8) * *(int *)(param_1 + 0x1ec) + param_3 + local_c) * 4) == 1)
        {
          return 0;
        }
      }
    }
    uVar1 = 1;
  }
  return uVar1;
}

