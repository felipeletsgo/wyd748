// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00558290 | Name: FUN_00558290


undefined4 FUN_00558290(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  int local_47c;
  int local_474;
  int local_470;
  int local_46c;
  undefined4 local_468 [8];
  undefined1 local_448 [32];
  undefined1 local_428 [32];
  undefined4 local_408;
  char local_404 [1024];
  
  iVar1 = FUN_0058f716(param_1,&DAT_005c4848);
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else {
    _memset(&DAT_00d42700,0,0x1e00);
    local_408 = 0;
    local_474 = 0;
    local_46c = FUN_00590a06(local_404,0x400,iVar1);
    if (local_46c == 0) {
      FUN_0058f62c(iVar1);
      uVar2 = 0;
    }
    else {
      FUN_005909d2(local_404,&DAT_005c484c,&DAT_00e37d28);
      if (DAT_00e37d28 < 0x50) {
        local_470 = 0;
        while ((local_470 < DAT_00e37d28 &&
               (iVar3 = FUN_00590a06(local_404,0x400,iVar1), iVar3 != 0))) {
          for (local_47c = 0; local_47c < 0x400; local_47c = local_47c + 1) {
            if (local_404[local_47c] == ',') {
              local_404[local_47c] = ' ';
            }
            if (local_404[local_47c] == '\0') break;
          }
          local_474 = -1;
          _memset(local_468,0,0x60);
          iVar3 = FUN_005909d2(local_404,s__d__s__s__s_005c4850,&local_474,local_468,local_448,
                               local_428);
          if (iVar3 == -1) break;
          FUN_0054fc81(local_468);
          FUN_0054fc81(local_448);
          FUN_0054fc81(local_428);
          if ((0 < local_474) && (local_474 < 0x4f)) {
            puVar4 = local_468;
            puVar5 = (undefined4 *)(&DAT_00d42700 + local_474 * 0x60);
            for (iVar3 = 0x18; iVar3 != 0; iVar3 = iVar3 + -1) {
              *puVar5 = *puVar4;
              puVar4 = puVar4 + 1;
              puVar5 = puVar5 + 1;
            }
          }
          local_470 = local_470 + 1;
        }
        FUN_0058f62c(iVar1);
        uVar2 = 1;
      }
      else {
        FUN_0058f62c(iVar1);
        uVar2 = 0;
      }
    }
  }
  return uVar2;
}

