// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054ede8 | Name: FUN_0054ede8


int FUN_0054ede8(int param_1,undefined1 param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int local_8;
  
  local_8 = 0;
  iVar3 = 0;
  for (iVar2 = 0; iVar2 < 0x10; iVar2 = iVar2 + 1) {
    iVar1 = (int)*(short *)(param_1 + 0x5c + iVar2 * 8);
    if ((iVar1 != 0) &&
       (iVar3 = FUN_0054cd07(param_1 + 0x5c + iVar2 * 8,CONCAT31((int3)((uint)iVar2 >> 8),param_2),
                             iVar1,iVar2,iVar3), local_8 < iVar3)) {
      local_8 = iVar3;
    }
  }
  return local_8;
}

