// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00591891 | Name: FUN_00591891


undefined4 FUN_00591891(LPCSTR param_1)

{
  BOOL BVar1;
  DWORD DVar2;
  
  BVar1 = DeleteFileA(param_1);
  if (BVar1 == 0) {
    DVar2 = GetLastError();
  }
  else {
    DVar2 = 0;
  }
  if (DVar2 != 0) {
    FUN_00594baa(DVar2);
    return 0xffffffff;
  }
  return 0;
}

