// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004a89a0 | Name: FUN_004a89a0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_004a89a0(void)

{
  int iVar1;
  BOOL BVar2;
  CHAR local_270 [256];
  CHAR local_170 [256];
  undefined1 local_70 [36];
  undefined1 local_4c [36];
  undefined1 local_28 [36];
  
  iVar1 = FUN_005906a7(s_WYDLauncher_exe_005b9580,local_28);
  if (iVar1 == 0) {
    FUN_0058f078(local_270,s_WYDLauncher_exe_005b9590);
    FUN_0058f078(local_170,s_TMPWYDLauncher_exe_005b95a0);
  }
  else {
    _DAT_005d0500 = 1;
    FUN_0058f078(local_270,s_PWYDLauncher_exe_005b95b4);
    FUN_0058f078(local_170,s_TMPPWYDLauncher_exe_005b95c8);
  }
  iVar1 = FUN_005906a7(local_170,local_4c);
  if (iVar1 == 0) {
    iVar1 = FUN_005906a7(local_270,local_70);
    if (iVar1 == 0) {
      SetFileAttributesA(local_270,0x80);
    }
    BVar2 = CopyFileA(local_170,local_270,0);
    if (BVar2 == 1) {
      DeleteFileA(local_170);
    }
  }
  return;
}

