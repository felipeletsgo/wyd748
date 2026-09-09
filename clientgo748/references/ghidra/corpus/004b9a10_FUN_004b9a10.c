// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b9a10 | Name: FUN_004b9a10


undefined4 __thiscall FUN_004b9a10(int param_1,int param_2)

{
  undefined4 uVar1;
  
  if ((param_2 < 0x800) && (-1 < param_2)) {
    if (*(int *)(*(int *)(param_1 + 0xe8330) + param_2 * 4) == 0) {
      uVar1 = 0;
    }
    else {
      uVar1 = 1;
    }
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

