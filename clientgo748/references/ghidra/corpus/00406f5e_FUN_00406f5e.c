// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00406f5e | Name: FUN_00406f5e


undefined4 __thiscall FUN_00406f5e(int *param_1,char *param_2)

{
  undefined4 uVar1;
  
  if (param_1[0xc] == 1) {
    _strncpy((char *)(param_1 + 0x3d3),param_2,0x100);
    (**(code **)(*param_1 + 0x54))();
    uVar1 = 1;
  }
  else {
    uVar1 = FUN_0040bfb0(param_2);
  }
  return uVar1;
}

