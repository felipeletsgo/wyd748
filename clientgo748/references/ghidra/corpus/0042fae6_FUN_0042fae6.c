// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042fae6 | Name: FUN_0042fae6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
FUN_0042fae6(int param_1,float param_2,float param_3,float param_4,float param_5,undefined4 param_6,
            undefined4 param_7,int param_8,undefined4 param_9,undefined4 param_10)

{
  undefined1 local_2c [8];
  undefined1 local_24 [8];
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined1 local_c [8];
  
  local_1c = __ftol();
  local_18 = __ftol();
  local_14 = __ftol();
  local_10 = __ftol();
  FUN_00430f20(param_4 / _DAT_005a365c + param_2,param_5 / _DAT_005a365c + param_3);
  FUN_00430f20(param_6,param_7);
  FUN_00430f20(param_9,param_10);
  if ((param_8 != 0) && (*(int *)(param_1 + 0x2a704) != 0)) {
    (**(code **)(**(int **)(param_1 + 0x2a704) + 0x14))
              (*(undefined4 *)(param_1 + 0x2a704),param_8,&local_1c,local_24,local_2c,0,local_c,
               0xffffffff);
  }
  return;
}

