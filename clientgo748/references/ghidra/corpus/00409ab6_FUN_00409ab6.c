// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00409ab6 | Name: FUN_00409ab6


undefined4 * __thiscall
FUN_00409ab6(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6,undefined4 param_7)

{
  FUN_00401466(param_2,param_3,param_4,param_5,param_6,param_7,4);
  param_1[0x79] = 0;
  *param_1 = &PTR_FUN_005a3d78;
  param_1[0x10] = 3;
  param_1[0x7b] = 0xff000000;
  param_1[0x7c] = param_7;
  return param_1;
}

