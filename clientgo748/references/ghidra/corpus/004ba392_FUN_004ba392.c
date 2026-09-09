// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ba392 | Name: FUN_004ba392


undefined4 __thiscall FUN_004ba392(int param_1,int param_2)

{
  undefined4 uVar1;
  
  if ((param_2 < 0x48) && (-1 < param_2)) {
    if (*(int *)(*(int *)(param_1 + 0xe8338) + param_2 * 4) == 0) {
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

