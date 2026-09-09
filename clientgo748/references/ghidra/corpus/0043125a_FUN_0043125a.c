// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0043125a | Name: FUN_0043125a


void FUN_0043125a(undefined4 param_1)

{
  int iVar1;
  int iVar2;
  undefined1 local_404 [1024];
  
  iVar1 = FUN_00590a95(local_404,0x400,param_1,&stack0x00000008);
  if ((DAT_005ccfac != -1) &&
     (iVar2 = FUN_00590b78(DAT_005ccfac,local_404,iVar1,iVar1,0), iVar2 == iVar1)) {
    FUN_00590ae5(DAT_005ccfac);
  }
  return;
}

