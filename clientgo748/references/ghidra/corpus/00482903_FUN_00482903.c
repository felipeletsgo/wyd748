// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00482903 | Name: FUN_00482903


bool __thiscall FUN_00482903(undefined4 param_1,int param_2)

{
  bool bVar1;
  undefined1 local_18 [4];
  undefined2 local_14;
  undefined2 local_12;
  undefined2 local_c;
  int local_8;
  
  local_8 = (**(code **)(*DAT_013b71e8 + 0x34))(*(undefined2 *)(param_2 + 6),param_1);
  bVar1 = local_8 == 0;
  if (bVar1) {
    _memset(local_18,0,0x10);
    local_12 = (undefined2)DAT_013b71e8[0x317];
    local_14 = 0x369;
    local_c = *(undefined2 *)(param_2 + 6);
    FUN_0055f2dd(local_18,0x10);
  }
  return bVar1;
}

