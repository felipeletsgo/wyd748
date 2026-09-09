// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00404eb1 | Name: FUN_00404eb1


undefined4 __thiscall FUN_00404eb1(int *param_1,char param_2)

{
  if (param_1[0x17] != 0) {
    if (((param_2 == '\r') || (param_2 == 'Y')) || (param_2 == 'y')) {
      (**(code **)(*param_1 + 0x60))(0);
      (*(code *)**(undefined4 **)param_1[0x17])(param_1[0x11],0);
      return 1;
    }
    if ((param_2 == 'N') || (param_2 == 'n')) {
      (**(code **)(*param_1 + 0x60))(0);
      (*(code *)**(undefined4 **)param_1[0x17])(param_1[0x11],1);
      return 1;
    }
  }
  return 0;
}

