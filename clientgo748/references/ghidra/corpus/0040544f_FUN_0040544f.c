// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040544f | Name: FUN_0040544f


void __thiscall FUN_0040544f(int *param_1,int param_2)

{
  if (param_1[0x7a] != param_2) {
    param_1[0x7a] = param_2;
    if (param_2 < 0) {
      param_1[0x7a] = 0;
    }
    (**(code **)(*param_1 + 0x54))();
  }
  return;
}

