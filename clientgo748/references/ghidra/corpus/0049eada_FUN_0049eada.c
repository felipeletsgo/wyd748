// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049eada | Name: FUN_0049eada


uint __thiscall
FUN_0049eada(undefined4 param_1,undefined4 param_2,undefined4 param_3,int param_4,int param_5)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  char *pcVar4;
  int local_11c;
  int local_118;
  char local_114 [10];
  char local_10a [246];
  char local_14 [12];
  undefined *local_8;
  
  iVar1 = FUN_0058f716(param_3,&DAT_005b9258,param_1);
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else {
    local_118 = 300;
    local_8 = (undefined *)0x0;
    local_11c = 0;
    while( true ) {
      if ((99 < local_11c) || (iVar3 = FUN_00590a06(local_114,0x100,iVar1), iVar3 == 0))
      goto LAB_0049ec47;
      _memset(local_14,0,0xb);
      _strncpy(local_14,local_114,10);
      FUN_005909d2(local_14,s__d__x_005b925c,&local_118,&local_8);
      if (local_118 == param_4) break;
      local_11c = local_11c + 1;
    }
    if ((local_8 == &DAT_00ffaaaa) && (5 < param_5)) {
      local_8 = (undefined *)0x0;
    }
    else if ((local_118 < 0x15f) || (5 < param_5)) {
      pcVar4 = _strstr(local_114,&DAT_005b9264);
      if (pcVar4 != (char *)0x0) {
        *pcVar4 = '\0';
      }
      if (local_10a[0] == ' ') {
        FUN_0058f078(param_2,&DAT_005b9268,local_10a);
      }
    }
    else {
      local_8 = (undefined *)0x0;
    }
LAB_0049ec47:
    FUN_0058f62c(iVar1);
    uVar2 = (uint)local_8 | 0xff000000;
  }
  return uVar2;
}

