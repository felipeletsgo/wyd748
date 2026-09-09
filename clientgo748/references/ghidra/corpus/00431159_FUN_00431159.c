// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00431159 | Name: FUN_00431159


void FUN_00431159(undefined4 param_1)

{
  _SYSTEMTIME local_41c;
  undefined4 local_40c;
  undefined1 *local_408;
  undefined1 local_404 [1024];
  
  local_408 = &stack0x00000008;
  local_40c = FUN_00590a95(local_404,0x400,param_1,local_408);
  local_408 = (undefined1 *)0x0;
  GetLocalTime(&local_41c);
  FUN_0043125a(s__02d__02d__02d__02d__02d____s_005b4b0c,local_41c._2_4_ & 0xffff,local_41c.wDay,
               local_41c.wHour,local_41c.wMinute,local_41c.wSecond,local_404);
  return;
}

