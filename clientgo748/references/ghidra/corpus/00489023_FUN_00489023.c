// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00489023 | Name: FUN_00489023


undefined4 __thiscall FUN_00489023(undefined4 param_1,int param_2)

{
  undefined4 local_c;
  
  FUN_0058f220(DAT_013b71e8 + 0xef6,param_2 + 0xc,0x14,param_1);
  for (local_c = 0; local_c < 0x14; local_c = local_c + 1) {
    if ((-1 < *(char *)(DAT_013b71e8 + local_c + 0xef6)) &&
       (*(char *)(DAT_013b71e8 + local_c + 0xef6) < '\x18')) {
      *(char *)(DAT_013b71e8 + local_c + 0xef6) =
           *(char *)(DAT_013b71e8 + local_c + 0xef6) + *(char *)(DAT_013b71e8 + 0x700) * '\x18';
    }
  }
  FUN_004470b9();
  return 1;
}

