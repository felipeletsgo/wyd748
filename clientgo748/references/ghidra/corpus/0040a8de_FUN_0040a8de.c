// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040a8de | Name: FUN_0040a8de


void __thiscall
FUN_0040a8de(int param_1,char param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,
            undefined4 param_6)

{
  undefined4 local_8;
  
  if (*(int *)(param_1 + 0x28) != 0) {
    *(char *)(param_1 + 0x369) = param_2;
    *(undefined4 *)(param_1 + 0x378) = param_3;
    *(undefined4 *)(param_1 + 0x37c) = param_4;
    *(undefined4 *)(param_1 + 0x380) = param_5;
    *(undefined4 *)(param_1 + 0x388) = param_6;
    if (param_2 == '\x01') {
      for (local_8 = 0; local_8 < 9; local_8 = local_8 + 1) {
        (**(code **)(**(int **)(param_1 + 0x2ec + local_8 * 4) + 0x60))(0);
        (**(code **)(**(int **)(param_1 + 0x310 + local_8 * 4) + 0x60))(0);
        *(undefined1 *)(param_1 + local_8 + 0x360) = 0;
      }
      *(undefined4 *)(param_1 + 0x390) = 0;
      *(undefined4 *)(param_1 + 0x3a0) = 0;
    }
  }
  return;
}

