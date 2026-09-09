// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005586b5 | Name: FUN_005586b5


undefined4 FUN_005586b5(void)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int local_ac;
  int local_a0;
  undefined4 local_9c;
  undefined4 *local_98;
  undefined1 local_94 [4];
  undefined1 local_90 [4];
  undefined1 local_8c [4];
  int local_88;
  char local_84 [128];
  
  iVar1 = FUN_0058f716(s_BlockIP_txt_005c489c,&DAT_005c4898);
  if (iVar1 == 0) {
    uVar2 = 1;
  }
  else {
    local_a0 = 0;
    local_9c = 0;
    local_98 = &local_9c;
    do {
      iVar3 = FUN_00590a06(local_84,0x7d,iVar1);
      if (iVar3 == 0) {
        FUN_0058f62c(iVar1);
        return 1;
      }
      _memset(local_94,0,0x10);
      for (local_ac = 0; local_ac < 0xf; local_ac = local_ac + 1) {
        if (local_84[local_ac] == '.') {
          local_84[local_ac] = ' ';
        }
      }
      FUN_005909d2(local_84,s__d__d__d__d_005c48a8,local_94,local_90,local_8c,&local_88);
      *(undefined1 *)local_98 = local_94[0];
      *(undefined1 *)((int)local_98 + 1) = local_90[0];
      *(undefined1 *)((int)local_98 + 2) = local_8c[0];
      if ((local_88 < 1) || (0xfe < local_88)) {
        *(undefined1 *)((int)local_98 + 3) = 0;
      }
      else {
        *(undefined1 *)((int)local_98 + 3) = (undefined1)local_88;
      }
      *(undefined4 *)(&DAT_00e34b14 + local_a0 * 4) = local_9c;
      local_a0 = local_a0 + 1;
    } while (local_a0 < 200);
    MessageBoxA((HWND)0x0,s_Warning_005c48c8,s_BlockIP_IP_is_Over_005c48b4,0);
    FUN_0058f62c(iVar1);
    uVar2 = 0;
  }
  return uVar2;
}

