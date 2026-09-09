// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042fbb1 | Name: FUN_0042fbb1


void __thiscall FUN_0042fbb1(int param_1)

{
  undefined4 in_stack_00000014;
  undefined4 in_stack_00000018;
  undefined4 in_stack_0000001c;
  undefined4 in_stack_00000020;
  float in_stack_00000024;
  int in_stack_00000028;
  undefined4 in_stack_0000002c;
  undefined4 in_stack_00000030;
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
  FUN_00430f20(in_stack_0000001c,in_stack_00000020);
  FUN_00430f20(in_stack_00000014,in_stack_00000018);
  FUN_00430f20(in_stack_0000002c,in_stack_00000030);
  if ((in_stack_00000028 != 0) && (*(int *)(param_1 + 0x2a704) != 0)) {
    (**(code **)(**(int **)(param_1 + 0x2a704) + 0x14))
              (*(undefined4 *)(param_1 + 0x2a704),in_stack_00000028,&local_1c,local_24,local_2c,
               -in_stack_00000024,local_c,0xffffffff);
  }
  return;
}

