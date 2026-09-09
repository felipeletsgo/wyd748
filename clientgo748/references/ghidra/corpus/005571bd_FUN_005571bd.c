// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005571bd | Name: FUN_005571bd


void FUN_005571bd(void)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  int local_6a8;
  undefined4 local_6a4;
  undefined1 local_6a0 [4];
  undefined1 local_69c [4];
  undefined1 local_698 [4];
  undefined1 local_694 [4];
  undefined1 local_690 [4];
  undefined1 local_68c [4];
  undefined1 local_688 [4];
  undefined1 local_684 [4];
  undefined1 local_680 [4];
  undefined1 local_67c [4];
  int local_678;
  undefined1 local_674 [8];
  undefined1 local_66c [8];
  undefined1 local_664 [4];
  undefined1 local_660 [4];
  undefined1 local_65c [4];
  undefined1 local_658 [4];
  undefined1 local_654 [4];
  undefined1 local_650 [4];
  undefined1 local_64c [4];
  undefined1 local_648 [4];
  int local_644;
  undefined1 local_640 [256];
  undefined1 local_540 [256];
  int local_440;
  int local_43c;
  undefined1 local_438 [24];
  undefined1 local_420 [24];
  int local_408;
  char local_404 [1024];
  
  local_43c = FUN_0058f716(s___SkillData_csv_005c464c,&DAT_005c4648);
  if (local_43c == 0) {
    local_43c = FUN_0058f716(s_______TMSRV_Run_SkillData_csv_005c4660,&DAT_005c465c);
  }
  if (local_43c == 0) {
    MessageBoxA((HWND)0x0,s_There_is_no_file_005c4690,s_SkillData_csv_005c4680,0);
  }
  else {
    _memset(&DAT_0092fef8,0,0x2700);
    local_408 = FUN_00590a06(local_404,0x400,local_43c);
    if (local_408 != 0) {
      while( true ) {
        local_644 = FUN_00590a06(local_404,0x400,local_43c);
        if (local_644 == 0) break;
        for (local_6a8 = 0; local_6a8 < 0x400; local_6a8 = local_6a8 + 1) {
          if (local_404[local_6a8] == ',') {
            local_404[local_6a8] = ' ';
          }
          if (local_404[local_6a8] == '\0') break;
        }
        _memset(&local_6a4,0,0x60);
        local_440 = -1;
        FUN_005909d2(local_404,s__d__d__d__d__d__d__d__d__d__d__d_005c46a4,&local_440,&local_6a4,
                     local_6a0,local_69c,local_698,local_694,local_690,local_68c,local_688,local_684
                     ,local_680,local_67c,&local_678,local_540,local_640,local_664,local_660,
                     local_65c,local_658,local_654,local_650,local_64c,local_648);
        if ((-1 < local_440) && (local_440 < 0x68)) {
          local_678 = (int)(local_678 + (local_678 >> 0x1f & 3U)) >> 2;
          _memset(local_420,0,0x18);
          _memset(local_438,0,0x18);
          _memset(local_674,0,8);
          _memset(local_66c,0,8);
          FUN_005909d2(local_540,s__d__d__d__d__d__d_005c46ec,local_420,local_420 + 4,local_420 + 8,
                       local_420 + 0xc,local_420 + 0x10,local_420 + 0x14);
          FUN_005909d2(local_640,s__d__d__d__d__d__d_005c4700,local_438,local_438 + 4,local_438 + 8,
                       local_438 + 0xc,local_438 + 0x10,local_438 + 0x14);
          for (local_6a8 = 0; local_6a8 < 6; local_6a8 = local_6a8 + 1) {
            local_674[local_6a8] = local_420[local_6a8 * 4];
            local_66c[local_6a8] = local_438[local_6a8 * 4];
          }
          puVar2 = &local_6a4;
          puVar3 = (undefined4 *)(&DAT_0092fef8 + local_440 * 0x60);
          for (iVar1 = 0x18; iVar1 != 0; iVar1 = iVar1 + -1) {
            *puVar3 = *puVar2;
            puVar2 = puVar2 + 1;
            puVar3 = puVar3 + 1;
          }
        }
      }
      FUN_0058f62c(local_43c);
    }
  }
  return;
}

