// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00563233 | Name: FUN_00563233


undefined4
FUN_00563233(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,float param_6)

{
  undefined1 local_24 [16];
  undefined1 local_14 [16];
  
  thunk_FUN_005630fc(local_24,param_2,param_5,param_6);
  thunk_FUN_005630fc(local_14,param_3,param_4,param_6);
  param_6 = (1.0 - param_6) * param_6;
  thunk_FUN_005630fc(param_1,local_24,local_14,param_6 + param_6);
  return param_1;
}

