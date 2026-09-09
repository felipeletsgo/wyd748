// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00494e10 | Name: FUN_00494e10


void __fastcall FUN_00494e10(int *param_1)

{
  if (param_1[10] != 0) {
    if ((undefined4 *)param_1[10] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[10])(1);
    }
    param_1[10] = 0;
  }
  (**(code **)(*param_1 + 0x4c))();
  return;
}

