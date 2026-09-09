// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044ef69 | Name: FUN_0044ef69


void __thiscall FUN_0044ef69(int param_1,int param_2)

{
  undefined1 local_28 [4];
  undefined2 local_24;
  undefined2 local_22;
  int local_1c;
  undefined2 local_18;
  undefined4 local_14;
  uint local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  _memset(local_28,0,0x24);
  local_22 = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
  local_24 = 0x398;
  local_18 = *(undefined2 *)(param_1 + 0x275ae);
  local_1c = param_2 + -0x28d;
  local_14 = *(undefined4 *)(param_1 + 0x2757c + local_1c * 4);
  local_10 = (uint)*(ushort *)(param_1 + 0x275ac);
  local_c = *(undefined4 *)(param_1 + 0x27510 + local_1c * 8);
  local_8 = *(undefined4 *)(param_1 + 0x27514 + local_1c * 8);
  FUN_0055f2dd(local_28,0x24);
  return;
}

