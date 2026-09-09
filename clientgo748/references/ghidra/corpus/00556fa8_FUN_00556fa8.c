// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00556fa8 | Name: FUN_00556fa8


void FUN_00556fa8(void)

{
  int iVar1;
  int local_824;
  undefined4 local_81c;
  undefined4 local_818;
  undefined4 local_814;
  char local_810 [1024];
  int local_410;
  int local_40c;
  int local_408;
  undefined1 local_404 [1024];
  
  local_40c = FUN_0058f716(s___InitItem_csv_005c45e4,&DAT_005c45e0);
  if (local_40c == 0) {
    local_40c = FUN_0058f716(s_______TMSRV_Run_InitItem_csv_005c45f8,&DAT_005c45f4);
  }
  if (local_40c == 0) {
    MessageBoxA((HWND)0x0,s_There_is_no_file_005c4628,s_InitItem_csv_005c4618,0);
  }
  else {
    local_408 = FUN_00590a06(local_404,0x3ff,local_40c);
    if (local_408 != 0) {
      DAT_00e37d2c = 0;
      while (iVar1 = FUN_00590a06(local_810,0x400,local_40c), iVar1 != 0) {
        for (local_824 = 0; local_824 < 0x400; local_824 = local_824 + 1) {
          if (local_810[local_824] == ',') {
            local_810[local_824] = ' ';
          }
          if (local_810[local_824] == '\0') break;
        }
        local_410 = -1;
        local_818 = 0;
        local_81c = 0;
        local_814 = 0;
        FUN_005909d2(local_810,s__d__d__d__d_005c463c,&local_410,&local_818,&local_81c,&local_814);
        if (local_410 != -1) {
          (&DAT_00e22c00)[DAT_00e37d2c * 4] = (undefined2)local_818;
          (&DAT_00e22c02)[DAT_00e37d2c * 4] = (undefined2)local_81c;
          (&DAT_00e22c04)[DAT_00e37d2c * 4] = (undefined2)local_410;
          (&DAT_00e22c06)[DAT_00e37d2c * 4] = (undefined2)local_814;
          DAT_00e37d2c = DAT_00e37d2c + 1;
        }
      }
      FUN_0058f62c(local_40c);
    }
  }
  return;
}

