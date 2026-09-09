// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00424bae | Name: FUN_00424bae


undefined4 __thiscall FUN_00424bae(int param_1,int param_2)

{
  undefined4 uVar1;
  undefined4 local_c;
  
  if (*(int *)(param_1 + 0xc) + 0xc4U < 0x20000) {
    for (local_c = 0; local_c < 0xc4; local_c = local_c + 1) {
      *(undefined1 *)(*(int *)(param_1 + 4) + *(int *)(param_1 + 0xc) + local_c) =
           *(undefined1 *)(param_2 + local_c);
    }
    *(int *)(param_1 + 0xc) = *(int *)(param_1 + 0xc) + 0xc4;
    uVar1 = FUN_00425266();
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

