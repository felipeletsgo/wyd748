// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b15cc | Name: FUN_004b15cc


int __thiscall FUN_004b15cc(int param_1,char *param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  while( true ) {
    if (0x3ff < local_8) {
      return -1;
    }
    if ((*(int *)(param_1 + 0x41808 + local_8 * 0x58) != 0) &&
       (iVar1 = _strcmp((char *)(param_1 + 0x417c8 + local_8 * 0x58),param_2), iVar1 == 0)) break;
    local_8 = local_8 + 1;
  }
  return local_8;
}

