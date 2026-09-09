// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0046247e | Name: FUN_0046247e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0046247e(int param_1)

{
  int iVar1;
  short sVar2;
  int extraout_var;
  int iVar3;
  int iVar4;
  undefined1 local_24 [4];
  undefined2 local_20;
  undefined2 local_1e;
  int local_18;
  int local_14;
  undefined4 local_10;
  undefined2 local_c;
  undefined2 local_a;
  
  _memset(local_24,0,0x20);
  iVar1 = *(int *)(DAT_005ccec0 + 0x1e8);
  if ((iVar1 != 0) && (1000 < (uint)(param_1 - _DAT_005d0414))) {
    iVar4 = *(int *)(iVar1 + 0x1d0);
    iVar3 = *(int *)(iVar1 + 0x1d4);
    local_1e = *(undefined2 *)(*(int *)(extraout_var + 0x4c) + 0x20);
    local_20 = 0x272;
    local_10 = 0;
    sVar2 = (**(code **)(**(int **)(iVar1 + 0x1e4) + 0xc0))
                      (*(undefined4 *)(*(int *)(iVar1 + 0x1e4) + 0x1e4),
                       *(undefined4 *)(*(int *)(iVar1 + 0x1e4) + 0x400),extraout_var,iVar3,iVar4);
    local_18 = (int)sVar2;
    if (local_18 != 0) {
      if (local_18 == 1) {
        local_14 = iVar3 * 9 + iVar4;
      }
      else if (local_18 == 2) {
        local_14 = iVar3 * 9 + iVar4;
      }
      else {
        sVar2 = (**(code **)(**(int **)(iVar1 + 0x1e4) + 0xc4))
                          (*(undefined4 *)(*(int *)(iVar1 + 0x1e4) + 0x1e4));
        local_14 = (int)sVar2;
      }
      local_c = __ftol();
      local_a = __ftol();
      FUN_0055f2dd(local_24,0x20);
      _DAT_005d0414 = param_1;
    }
  }
  return;
}

