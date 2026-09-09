// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0048529b | Name: FUN_0048529b


undefined4 __thiscall FUN_0048529b(int param_1,int param_2)

{
  int local_8;
  
  (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(0,1);
  (**(code **)(*DAT_0092e654 + 4))(*(undefined4 *)(param_2 + 8));
  DAT_013b71e8[0x317] = (uint)*(ushort *)(param_2 + 0x306);
  FUN_0058f220(DAT_013b71e8 + 0x1bb,param_2 + 0x10,0x2f4);
  DAT_013b71e8[0x314] = *(int *)(param_2 + 0x31c);
  *(undefined2 *)(DAT_013b71e8 + 0x1c3) = *(undefined2 *)(param_2 + 0xc);
  *(undefined2 *)((int)DAT_013b71e8 + 0x70e) = *(undefined2 *)(param_2 + 0xe);
  FUN_0058f220((int)DAT_013b71e8 + 0xef6,DAT_013b71e8 + 0x275,4);
  FUN_0058f220((int)DAT_013b71e8 + 0xefa,param_2 + 0x30a,0x10);
  for (local_8 = 0; local_8 < 0x14; local_8 = local_8 + 1) {
    if ((-1 < *(char *)((int)DAT_013b71e8 + local_8 + 0xef6)) &&
       (*(char *)((int)DAT_013b71e8 + local_8 + 0xef6) < '\x18')) {
      *(char *)((int)DAT_013b71e8 + local_8 + 0xef6) =
           *(char *)((int)DAT_013b71e8 + local_8 + 0xef6) + (char)DAT_013b71e8[0x1c0] * '\x18';
    }
  }
  DAT_005d03e4 = (uint)*(ushort *)(param_2 + 0x308);
  (**(code **)(**(int **)(param_1 + 0x27acc) + 0x98))();
  (**(code **)(**(int **)(param_1 + 0x27ad8) + 0x98))();
  (**(code **)(**(int **)(param_1 + 0x27ad0) + 0x98))();
  (**(code **)(**(int **)(param_1 + 0x27adc) + 0x98))();
  if (*(int *)(param_1 + 0x27ad4) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27ad4) + 0x98))();
  }
  if (*(int *)(param_1 + 0x27ae0) != 0) {
    (**(code **)(**(int **)(param_1 + 0x27ae0) + 0x98))();
  }
  (**(code **)(*DAT_013b71e8 + 0x54))(9);
  return 1;
}

