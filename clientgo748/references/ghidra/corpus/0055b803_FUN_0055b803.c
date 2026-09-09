// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055b803 | Name: FUN_0055b803


void FUN_0055b803(void)

{
  int iVar1;
  int local_108;
  CHAR local_104 [256];
  
  iVar1 = 0;
  GetSystemDirectoryA(local_104,0x100);
  local_108 = 0;
  while( true ) {
    if (0xb < local_108) {
      return;
    }
    wsprintfA(local_104,&DAT_005c4bd8,(&PTR_s_67286729_zip_005c495c)[local_108],iVar1);
    iVar1 = FUN_0058f716(local_104,&DAT_005c4bdc);
    if (iVar1 == 0) break;
    FUN_0058f62c(iVar1);
    DeleteFileA(local_104);
    local_108 = local_108 + 1;
  }
  return;
}

