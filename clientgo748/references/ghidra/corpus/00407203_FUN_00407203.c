// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00407203 | Name: FUN_00407203


undefined4 * __thiscall
FUN_00407203(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,
            undefined4 param_9,undefined4 param_10,undefined4 param_11)

{
  FUN_00401d03(0xffffffff,param_2,param_3,param_4,param_5,param_6,param_7,param_8,param_9,param_10,
               param_11);
  *param_1 = &PTR_FUN_005a3a08;
  param_1[0x10] = 7;
  param_1[0x393] = 0;
  return param_1;
}

