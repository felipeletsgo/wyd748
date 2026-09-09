// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059cb91 | Name: FUN_0059cb91


undefined4 FUN_0059cb91(uint param_1,uint param_2,uint *param_3)

{
  uint uVar1;
  undefined4 uVar2;
  
  uVar2 = 0;
  uVar1 = param_1 + param_2;
  if ((uVar1 < param_1) || (uVar1 < param_2)) {
    uVar2 = 1;
  }
  *param_3 = uVar1;
  return uVar2;
}

