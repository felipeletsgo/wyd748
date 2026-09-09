// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ba9fc | Name: FUN_004ba9fc


undefined4 __thiscall FUN_004ba9fc(int param_1,int param_2)

{
  undefined4 uVar1;
  DWORD DVar2;
  
  if ((param_2 < 0x48) && (-1 < param_2)) {
    if (*(int *)(*(int *)(param_1 + 0xe8338) + param_2 * 4) != 0) {
      DVar2 = timeGetTime();
      *(DWORD *)(param_1 + 0xe833c + param_2 * 4) = DVar2;
    }
    uVar1 = *(undefined4 *)(*(int *)(param_1 + 0xe8338) + param_2 * 4);
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

