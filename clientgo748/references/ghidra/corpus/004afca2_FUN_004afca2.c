// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004afca2 | Name: FUN_004afca2


undefined4 FUN_004afca2(void)

{
  undefined4 uVar1;
  uint uVar2;
  undefined4 extraout_var;
  int local_2c;
  int local_28;
  int local_24;
  int local_1c;
  int local_18;
  int local_14;
  size_t local_10;
  undefined *local_c;
  int local_8;
  
  local_8 = 0;
  local_c = &DAT_005b98d8;
  local_10 = _strlen(&DAT_005b98d8);
  local_14 = 0;
  local_14 = FUN_0058f716(s_object_bin_005b9988,&DAT_005b9984,extraout_var);
  if (local_14 == 0) {
    MessageBoxA((HWND)0x0,s_Can_t_read_object_bin_005b99c4,s_ERROR_005b99bc,0);
    uVar1 = 0;
  }
  else {
    for (local_24 = 0; local_24 < 0x800; local_24 = local_24 + 1) {
      for (local_28 = 0; local_28 < 0x10; local_28 = local_28 + 1) {
        FUN_00590d68(&DAT_005da288 + local_28 * 0x10 + local_24 * 0x100,0x10,1,local_14);
        for (local_2c = 0; local_2c < 0x10; local_2c = local_2c + 1) {
          uVar2 = (int)(char)(&DAT_005da288)[local_2c + local_24 * 0x100 + local_28 * 0x10] &
                  0x80000003;
          if ((int)uVar2 < 0) {
            uVar2 = (uVar2 - 1 | 0xfffffffc) + 1;
          }
          local_8 = local_24 * 5 + local_8 + uVar2 + local_28 * 2;
        }
      }
    }
    for (local_1c = 0; local_1c < 0x80000; local_1c = local_1c + 1) {
      (&DAT_005da288)[local_1c] =
           ((&DAT_005da288)[local_1c] - local_c[local_1c % (int)local_10]) - (char)local_1c;
    }
    local_18 = 0;
    FUN_00590d68(&local_18,4,1,local_14);
    FUN_0058f62c(local_14);
    if (local_8 == local_18) {
      uVar1 = 1;
    }
    else {
      MessageBoxA((HWND)0x0,s_Object_bin_File_Crashed_005b99a4,s_CheckSum_Error_005b9994,0);
      uVar1 = 0;
    }
  }
  return uVar1;
}

