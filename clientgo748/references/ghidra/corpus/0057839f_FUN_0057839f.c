// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057839f | Name: FUN_0057839f


void FUN_0057839f(int *param_1)

{
  undefined4 *puVar1;
  
  puVar1 = (undefined4 *)param_1[99];
  *(undefined1 *)(puVar1 + 4) = 0;
  *(undefined1 *)((int)puVar1 + 0x11) = 0;
  *puVar1 = &LAB_0057830c;
  *(undefined1 *)(puVar1 + 5) = 1;
  (**(code **)(*param_1 + 0x10))(param_1);
  (**(code **)param_1[100])(param_1);
  param_1[0x23] = 0;
  return;
}

