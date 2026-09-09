// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00532390 | Name: FUN_00532390


undefined4 * __fastcall FUN_00532390(undefined4 *param_1)

{
  int iVar1;
  int iVar2;
  int local_24;
  int local_20;
  int local_1c;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a2599;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0054a9e0(0);
  local_8 = 0;
  FUN_00421910(param_1 + 0x5033,0xc,0x1000,CSimpleArray<>);
  FUN_00421910(param_1 + 0x8044,0x2c,4,CTraceSnapshot::CTraceSnapshot);
  FUN_00421910(param_1 + 0x8070,0x18,4,FUN_004cb8e0);
  FUN_00430f10();
  FUN_00430f10();
  FUN_00430f10();
  *param_1 = &PTR_FUN_005a5870;
  param_1[0x2b] = 0;
  param_1[0x2c] = 0;
  param_1[0x2d] = 0;
  param_1[0x2e] = 0;
  param_1[0x8297] = 0;
  param_1[0x2f] = 1;
  param_1[0x31] = 0;
  param_1[0x30] = 0;
  param_1[0x29] = 0;
  param_1[0x2a] = 0;
  *(undefined1 *)(param_1 + 0x828c) = 0;
  *(undefined1 *)((int)param_1 + 0x20a31) = 0;
  *(undefined1 *)((int)param_1 + 0x20a32) = 0;
  *(undefined1 *)((int)param_1 + 0x20a33) = 0;
  param_1[0x8298] = 0x40000000;
  param_1[0x829b] = 0;
  for (local_14 = 0; local_14 < 4; local_14 = local_14 + 1) {
    param_1[local_14 * 0xb + 0x804a] = 0xffffff;
    param_1[local_14 * 6 + 0x8073] = 0xffffff;
  }
  for (local_14 = 0; local_14 < 0x80; local_14 = local_14 + 1) {
    _memset(param_1 + local_14 * 0x20 + 0x3033,0,0x80);
    _memset(param_1 + local_14 * 0x20 + 0x4033,0,0x80);
  }
  for (local_14 = 0; local_14 < 10; local_14 = local_14 + 1) {
    param_1[local_14 + 0x828d] = 0;
  }
  if (DAT_005bde18 == 1) {
    iVar1 = FUN_0058f716(s_cdata_bin_005bde24,&DAT_005bde20);
    if (iVar1 == 0) {
      FUN_00431159(s_DataFile_Not_Found_005bde60);
      if (*(int *)(DAT_0067cf38 + 0x54) == 0) {
        FUN_0049ed58(4,0,0,0,0);
      }
      *(undefined4 *)(DAT_0067cf38 + 0x54) = 1;
      ExceptionList = local_10;
      return param_1;
    }
    FUN_00590d68(&DAT_005bbe18,0x2000,1,iVar1);
    local_20 = 0;
    for (local_1c = 0; local_1c < 0x40; local_1c = local_1c + 1) {
      for (local_24 = 0; local_24 < 0x20; local_24 = local_24 + 1) {
        local_20 = local_24 * 0x10 + local_20 +
                   local_1c * 8 + *(int *)(&DAT_005bbe18 + local_24 * 4 + local_1c * 0x80) * 4;
      }
    }
    iVar2 = FUN_00494dcf();
    if ((iVar2 == 30000) && (local_20 != 0x231122b4)) {
      FUN_0058f62c(iVar1);
      FUN_00431159(s_DataFile_Error_005bde30);
      MessageBoxA(*(HWND *)(DAT_013b71e0 + 4),s_DataFile_Error__005bde50,s_File_Error_005bde44,0);
      PostMessageA(*(HWND *)(DAT_013b71e0 + 4),0x10,0,0);
      ExceptionList = local_10;
      return param_1;
    }
    FUN_0058f62c(iVar1);
    DAT_005bde18 = 0;
  }
  FUN_0053277e(0,0);
  ExceptionList = local_10;
  return param_1;
}

