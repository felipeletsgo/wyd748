// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005584df | Name: FUN_005584df


undefined4 FUN_005584df(void)

{
  int iVar1;
  int iVar2;
  int local_ac;
  int local_a0;
  undefined4 local_9c;
  undefined4 *local_98;
  undefined1 local_94 [4];
  undefined1 local_90 [4];
  undefined1 local_8c [4];
  int local_88;
  char local_84 [128];
  
  iVar1 = FUN_0058f716(s_GameRoomIP_txt_005c4860,&DAT_005c485c);
  if (iVar1 != 0) {
    local_a0 = 0;
    local_9c = 0;
    local_98 = &local_9c;
    do {
      iVar2 = FUN_00590a06(local_84,0x7d,iVar1);
      if (iVar2 == 0) {
        FUN_0058f62c(iVar1);
        return 1;
      }
      _memset(local_94,0,0x10);
      for (local_ac = 0; local_ac < 0xf; local_ac = local_ac + 1) {
        if (local_84[local_ac] == '.') {
          local_84[local_ac] = ' ';
        }
      }
      FUN_005909d2(local_84,s__d__d__d__d_005c4870,local_94,local_90,local_8c,&local_88);
      *(undefined1 *)local_98 = local_94[0];
      *(undefined1 *)((int)local_98 + 1) = local_90[0];
      *(undefined1 *)((int)local_98 + 2) = local_8c[0];
      if ((local_88 < 1) || (0xfe < local_88)) {
        *(undefined1 *)((int)local_98 + 3) = 0;
      }
      else {
        *(undefined1 *)((int)local_98 + 3) = (undefined1)local_88;
      }
      *(undefined4 *)(&DAT_00e34e34 + local_a0 * 4) = local_9c;
      local_a0 = local_a0 + 1;
    } while (local_a0 < 3000);
    MessageBoxA((HWND)0x0,s_Warning_005c4890,s_PC_Room_IP_is_Over_005c487c,0);
    FUN_0058f62c(iVar1);
  }
  return 0;
}

