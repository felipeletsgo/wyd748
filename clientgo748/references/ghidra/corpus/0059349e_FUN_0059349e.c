// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059349e | Name: FUN_0059349e


undefined4
FUN_0059349e(int *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,int *param_5,
            int param_6,undefined4 param_7,uint param_8)

{
  undefined4 uVar1;
  
  if (*param_5 != 0x19930520) {
    FUN_00593ebb();
  }
  if ((*(byte *)(param_1 + 1) & 0x66) == 0) {
    if (param_5[3] != 0) {
      if (((*param_1 == -0x1f928c9d) && (0x19930520 < (uint)param_1[5])) &&
         (*(code **)(param_1[7] + 8) != (code *)0x0)) {
        uVar1 = (**(code **)(param_1[7] + 8))
                          (param_1,param_2,param_3,param_4,param_5,param_6,param_7,param_8 & 0xff);
        return uVar1;
      }
      FUN_00593539(param_1,param_2,param_3,param_4,param_5,param_8,param_6,param_7);
    }
  }
  else if ((param_5[1] != 0) && (param_6 == 0)) {
    FUN_005937f3(param_2,param_4,param_5,0xffffffff);
  }
  return 1;
}

