// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00590fb3 | Name: FUN_00590fb3


void FUN_00590fb3(void)

{
  int iVar1;
  
  if (PTR_FUN_005c9934 != (undefined *)0x0) {
    (*(code *)PTR_FUN_005c9934)();
  }
  iVar1 = FUN_00593d53();
  if (iVar1 == 0) {
    __amsg_exit(0x10);
  }
  if (*(HANDLE *)(iVar1 + 4) != (HANDLE)0xffffffff) {
    CloseHandle(*(HANDLE *)(iVar1 + 4));
  }
  FUN_00593dba(iVar1);
                    /* WARNING: Subroutine does not return */
  ExitThread(0);
}

