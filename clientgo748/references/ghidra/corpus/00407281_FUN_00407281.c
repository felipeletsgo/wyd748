// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00407281 | Name: FUN_00407281


void __thiscall
FUN_00407281(int *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,int param_6)

{
  if (param_1[0x393] == 0) {
    if (param_6 == 1) {
      *(undefined1 *)(param_1 + 0x337) = 1;
      (**(code **)(*param_1 + 0x8c))(2);
    }
    else {
      *(undefined1 *)(param_1 + 0x337) = 0;
      (**(code **)(*param_1 + 0x8c))(1);
    }
  }
  FUN_0040268c(param_2,param_3,param_4,param_5,param_6);
  return;
}

