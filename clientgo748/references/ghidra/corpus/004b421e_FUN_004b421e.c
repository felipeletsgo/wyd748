// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b421e | Name: FUN_004b421e


undefined4 FUN_004b421e(int *param_1)

{
  undefined1 local_1fc [500];
  undefined4 local_8;
  
  local_8 = FUN_004b45a6(param_1,1,s__exceptions_log_005b9b64);
  FUN_0058f078(local_1fc,s_____Unhandled_Exception__ExpCode_005b9e20,*(undefined4 *)*param_1,
               *(undefined4 *)(*param_1 + 4),*(undefined4 *)(*param_1 + 0xc));
  return local_8;
}

