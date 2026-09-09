// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054c0c4 | Name: FUN_0054c0c4


void FUN_0054c0c4(undefined4 param_1,int param_2,undefined4 param_3)

{
  if ((param_2 < 0) || (0x1b7 < param_2)) {
    FUN_0058ee20(param_1,s__UNHANDLED__005c3f74);
  }
  else {
    FUN_0058f078(param_1,&DAT_00a34178 + param_2 * 0x80,param_3);
  }
  return;
}

