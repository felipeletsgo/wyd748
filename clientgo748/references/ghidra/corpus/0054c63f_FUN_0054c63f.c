// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054c63f | Name: FUN_0054c63f


void FUN_0054c63f(undefined2 *param_1,undefined4 param_2)

{
  undefined1 local_120 [256];
  undefined2 local_20 [2];
  undefined1 local_1c [4];
  undefined1 local_18 [4];
  undefined1 local_14 [4];
  undefined1 local_10 [4];
  undefined1 local_c [4];
  undefined1 local_8 [4];
  
  FUN_0058ee20(local_120,param_2);
  FUN_0054fc81(local_120);
  _memset(param_1,0,8);
  _memset(local_20,0,0x1c);
  FUN_005909d2(local_120,s__d__d__d__d__d__d__d_005c3fc8,local_20,local_1c,local_18,local_14,
               local_10,local_c,local_8);
  *param_1 = local_20[0];
  *(undefined1 *)(param_1 + 1) = local_1c[0];
  *(undefined1 *)((int)param_1 + 3) = local_18[0];
  *(undefined1 *)(param_1 + 2) = local_14[0];
  *(undefined1 *)((int)param_1 + 5) = local_10[0];
  *(undefined1 *)(param_1 + 3) = local_c[0];
  *(undefined1 *)((int)param_1 + 7) = local_8[0];
  return;
}

