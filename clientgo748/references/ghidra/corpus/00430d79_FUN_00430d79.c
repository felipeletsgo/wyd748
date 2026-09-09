// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00430d79 | Name: FUN_00430d79


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4
FUN_00430d79(undefined4 param_1,undefined4 param_2,undefined4 param_3,int *param_4,int *param_5)

{
  int iVar1;
  undefined1 local_30 [12];
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined1 local_18 [8];
  float local_10;
  int local_8;
  
  FUN_00430f10();
  FUN_00430f10();
  FUN_00430f10();
  local_24 = param_1;
  local_1c = param_3;
  local_20 = param_2;
  thunk_FUN_00561462(local_30,&local_24,DAT_005ccf9c + 0x2a62c);
  thunk_FUN_00561462(local_18,local_30,DAT_005ccf9c + 0x2a5ec);
  if ((_DAT_005a3534 <= local_10) && (local_10 < _DAT_005a3660)) {
    local_8 = __ftol();
    iVar1 = __ftol();
    if ((0 < local_8) &&
       (((local_8 < *(int *)(DAT_005ccf9c + 0x2a504) - *(int *)(DAT_005ccf9c + 0x2a5e4) &&
         (0 < iVar1)) &&
        (iVar1 < *(int *)(DAT_005ccf9c + 0x2a508) - *(int *)(DAT_005ccf9c + 0x2a5e8))))) {
      *param_4 = local_8;
      *param_5 = iVar1;
      return 1;
    }
  }
  return 0;
}

