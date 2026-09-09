// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00463832 | Name: FUN_00463832


void __fastcall FUN_00463832(undefined4 param_1)

{
  bool bVar1;
  undefined4 local_c;
  
  bVar1 = false;
  local_c = 0;
  do {
    if (7 < local_c) {
LAB_0046387a:
      if (bVar1) {
        FUN_0055f2dd(DAT_013b71e8 + 0xd50,0x54,param_1);
      }
      return;
    }
    if (*(char *)(DAT_013b71e8 + local_c + 0xd9c) != -1) {
      bVar1 = true;
      goto LAB_0046387a;
    }
    local_c = local_c + 1;
  } while( true );
}

