// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b9acf | Name: FUN_004b9acf


undefined4 __thiscall FUN_004b9acf(int param_1,char *param_2,undefined4 param_3)

{
  bool bVar1;
  int iVar2;
  undefined4 uVar3;
  DWORD DVar4;
  int local_10;
  int local_c;
  
  local_10 = -1;
  bVar1 = false;
  local_c = 0;
  do {
    if (0x7ff < local_c) {
LAB_004b9b36:
      if (((bVar1) && (local_10 < 0x800)) && (-1 < local_10)) {
        if ((*(int *)(*(int *)(param_1 + 0xe8330) + local_10 * 4) == 0) &&
           (iVar2 = FUN_004b9478(local_10), iVar2 == 0)) {
          return 0;
        }
        DVar4 = timeGetTime();
        *(DWORD *)(param_1 + 0x43428 + local_10 * 0x108) = DVar4;
        *(undefined4 *)(param_1 + 0x4342c + local_10 * 0x108) = param_3;
        uVar3 = *(undefined4 *)(*(int *)(param_1 + 0xe8330) + local_10 * 4);
      }
      else {
        uVar3 = 0;
      }
      return uVar3;
    }
    iVar2 = _strcmp(param_2,(char *)(param_1 + 0x43328 + local_c * 0x108));
    if (iVar2 == 0) {
      local_10 = local_c;
      bVar1 = true;
      goto LAB_004b9b36;
    }
    local_c = local_c + 1;
  } while( true );
}

