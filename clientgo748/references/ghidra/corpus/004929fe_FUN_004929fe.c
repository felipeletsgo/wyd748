// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004929fe | Name: FUN_004929fe


undefined4 __thiscall FUN_004929fe(int param_1,int param_2)

{
  undefined4 uVar1;
  undefined4 local_8;
  
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  *(undefined4 *)(param_1 + 0x274d8) = uVar1;
  if (*(int *)(param_2 + 0xc) < 0) {
    *(undefined4 *)(param_2 + 0xc) = 0;
  }
  if (4 < *(int *)(param_2 + 0xc)) {
    *(undefined4 *)(param_2 + 0xc) = 4;
  }
  for (local_8 = 0; local_8 < 0x60; local_8 = local_8 + 1) {
    *(byte *)(param_2 + local_8 + 0x10) = *(byte *)(param_2 + local_8 + 0x10) ^ 0x61;
  }
  *(undefined1 *)(param_2 + 0x6f) = 0;
  FUN_0047e8e2(*(undefined4 *)(param_2 + 0xc),param_2 + 0x10);
  return 1;
}

