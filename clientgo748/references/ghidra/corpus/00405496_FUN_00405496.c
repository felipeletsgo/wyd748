// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00405496 | Name: FUN_00405496


void __thiscall FUN_00405496(int *param_1,int param_2)

{
  if (param_1[0x7b] != param_2) {
    param_1[0x7b] = param_2;
    (**(code **)(*param_1 + 0x54))();
  }
  return;
}

