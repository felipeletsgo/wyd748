// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00563de5 | Name: FUN_00563de5


undefined4 FUN_00563de5(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined1 local_14 [16];
  
  thunk_FUN_00562c94(local_14,param_2,param_3,param_4);
  thunk_FUN_0056272c(param_1,local_14);
  return param_1;
}

