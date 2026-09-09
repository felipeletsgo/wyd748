// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00425a8f | Name: FUN_00425a8f


undefined4 FUN_00425a8f(uint *param_1,uint *param_2)

{
  undefined4 uVar1;
  
  if ((int)param_2[3] < (int)param_1[3]) {
    uVar1 = 0xffffffff;
  }
  else if ((int)param_1[3] < (int)param_2[3]) {
    uVar1 = 1;
  }
  else if (*param_1 < *param_2) {
    uVar1 = 0xffffffff;
  }
  else if (*param_2 < *param_1) {
    uVar1 = 1;
  }
  else if (param_1[1] < param_2[1]) {
    uVar1 = 0xffffffff;
  }
  else if (param_2[1] < param_1[1]) {
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

