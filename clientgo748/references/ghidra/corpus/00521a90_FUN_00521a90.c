// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00521a90 | Name: FUN_00521a90


undefined4 FUN_00521a90(undefined4 param_1,undefined4 param_2,int param_3)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  int local_1d4;
  int local_1d0;
  int local_1cc [4];
  undefined1 local_1bc [384];
  undefined1 local_3c [8];
  undefined1 local_34 [48];
  
  local_1cc[1] = DAT_0067cf38 + 0x94;
  local_1cc[2] = 0;
  local_1cc[3] = 0x2e;
  do {
    if (local_1cc[3] < 0) {
LAB_00521beb:
      if ((local_1cc[3] < 0) || (0x2e < local_1cc[3])) {
        uVar3 = 0;
      }
      else if ((local_1cc[2] < 0) || (0x2e < local_1cc[2])) {
        uVar3 = 0;
      }
      else {
        FUN_00421910(local_1bc,8,0x30,FUN_00430f10);
        FUN_00430f10();
        FUN_005211be(param_1,param_2,local_34,local_1bc,local_1cc);
        if (local_1cc[0] + -2 < 0) {
          uVar3 = 0;
        }
        else {
          FUN_0040bd30(local_1cc + local_1cc[0] * 2);
          for (local_1cc[3] = local_1cc[0] + -2; local_1cc[3] < 0x30;
              local_1cc[3] = local_1cc[3] + 1) {
            FUN_0040bd30(local_3c);
            if ((local_1cc[2] - (local_1cc[0] + -2)) + local_1cc[3] < 0x30) {
              FUN_0040bd30(param_3 + ((local_1cc[2] - (local_1cc[0] + -2)) + local_1cc[3]) * 8);
            }
            FUN_0040bd30(local_1bc + local_1cc[3] * 8);
          }
          FUN_0058f220(param_3,local_1bc,0x180);
          uVar3 = 1;
        }
      }
      return uVar3;
    }
    iVar1 = __ftol();
    iVar2 = __ftol();
    if (iVar1 == iVar2) {
      iVar1 = __ftol();
      iVar2 = __ftol();
      if (iVar1 != iVar2) goto LAB_00521b39;
    }
    else {
LAB_00521b39:
      local_1d4 = __ftol();
      local_1d0 = __ftol();
      _memset(local_34,0,0x30);
      FUN_00555ad8(param_1,param_2,&local_1d4,&local_1d0,local_34,0xc,local_1cc[1],8);
      iVar1 = __ftol();
      if (local_1d4 == iVar1) {
        iVar1 = __ftol();
        if (local_1d0 == iVar1) {
          local_1cc[2] = local_1cc[3];
          goto LAB_00521beb;
        }
      }
    }
    local_1cc[3] = local_1cc[3] + -1;
  } while( true );
}

