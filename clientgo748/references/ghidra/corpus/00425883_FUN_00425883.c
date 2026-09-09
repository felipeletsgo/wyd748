// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00425883 | Name: FUN_00425883


undefined4 __thiscall FUN_00425883(int param_1,int param_2)

{
  undefined4 uVar1;
  undefined4 local_10;
  
  if (*(int *)(param_1 + 0xc) + 0x88 < 0x20000) {
    for (local_10 = 0; local_10 < 0x88; local_10 = local_10 + 1) {
      *(undefined1 *)(*(int *)(param_1 + 4) + *(int *)(param_1 + 0xc) + local_10) =
           *(undefined1 *)(param_2 + local_10);
    }
    *(int *)(param_1 + 0xc) = *(int *)(param_1 + 0xc) + 0x88;
    uVar1 = FUN_00425266();
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

