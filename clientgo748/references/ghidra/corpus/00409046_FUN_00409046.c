// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00409046 | Name: FUN_00409046


undefined4 __thiscall FUN_00409046(int param_1,int param_2)

{
  undefined4 uVar1;
  
  if ((param_2 < *(int *)(param_1 + 0x210)) && (-1 < param_2)) {
    uVar1 = *(undefined4 *)(param_1 + 0x214 + param_2 * 4);
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

