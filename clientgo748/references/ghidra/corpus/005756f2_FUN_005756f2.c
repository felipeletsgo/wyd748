// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005756f2 | Name: FUN_005756f2


void FUN_005756f2(int param_1,int param_2,int param_3,int param_4,undefined4 *param_5)

{
  if ((param_1 != 0) && (param_2 != 0)) {
    if (param_3 != 0) {
      *(int *)(param_2 + 0x30) = param_3;
    }
    if (param_5 != (undefined4 *)0x0) {
      *(undefined4 *)(param_2 + 0x34) = *param_5;
      *(undefined4 *)(param_2 + 0x38) = param_5[1];
      *(undefined2 *)(param_2 + 0x3c) = *(undefined2 *)(param_5 + 2);
      if (param_4 == 0) {
        param_4 = 1;
      }
    }
    *(uint *)(param_2 + 8) = *(uint *)(param_2 + 8) | 0x10;
    *(undefined2 *)(param_2 + 0x16) = (undefined2)param_4;
  }
  return;
}

