// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00592608 | Name: FUN_00592608


undefined4 FUN_00592608(HANDLE param_1)

{
  BOOL BVar1;
  undefined4 *puVar2;
  
  BVar1 = FindClose(param_1);
  if (BVar1 == 0) {
    puVar2 = (undefined4 *)FUN_00594c1d();
    *puVar2 = 0x16;
    return 0xffffffff;
  }
  return 0;
}

