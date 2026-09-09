// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b3da8 | Name: FUN_004b3da8


undefined4 FUN_004b3da8(int param_1)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  int *local_10;
  int *local_c;
  
  iVar1 = FUN_004b3d86(param_1);
  local_10 = (int *)0x0;
  local_c = (int *)(&DAT_0069cfc8 + iVar1 * 0x524);
  while( true ) {
    if (local_c == (int *)0x0) {
      return 0;
    }
    if (*local_c == param_1) break;
    local_10 = local_c;
    local_c = (int *)local_c[3];
  }
  DAT_0092e5b8 = DAT_0092e5b8 + 1;
  DAT_0092e5c0 = DAT_0092e5c0 + -1;
  if (local_10 == (int *)0x0) {
    if (local_c[3] == 0) {
      _memset(&DAT_0069cfc8 + iVar1 * 0x524,0,0x524);
    }
    else {
      iVar1 = local_c[3];
      piVar3 = (int *)local_c[3];
      for (iVar2 = 0x149; iVar2 != 0; iVar2 = iVar2 + -1) {
        *local_c = *piVar3;
        piVar3 = piVar3 + 1;
        local_c = local_c + 1;
      }
      FUN_0058ffd8(iVar1);
    }
    return 1;
  }
  DAT_0092e5c8 = DAT_0092e5c8 + -1;
  local_10[3] = local_c[3];
  FUN_0058ffd8(local_c);
  return 1;
}

