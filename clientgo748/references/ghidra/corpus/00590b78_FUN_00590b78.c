// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00590b78 | Name: FUN_00590b78


undefined4 FUN_00590b78(uint param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  
  if ((param_1 < DAT_013b8720) &&
     ((*(byte *)((&DAT_013b8620)[(int)param_1 >> 5] + 4 + (param_1 & 0x1f) * 0x24) & 1) != 0)) {
    FUN_00594e8f(param_1);
    uVar1 = FUN_00590bdd(param_1,param_2,param_3);
    FUN_00594eee(param_1);
    return uVar1;
  }
  puVar2 = (undefined4 *)FUN_00594c1d();
  *puVar2 = 9;
  puVar2 = (undefined4 *)FUN_00594c26();
  *puVar2 = 0;
  return 0xffffffff;
}

