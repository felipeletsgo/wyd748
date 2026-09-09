// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042dcb7 | Name: FUN_0042dcb7


undefined4 __fastcall FUN_0042dcb7(int param_1)

{
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  FUN_00430fc0(param_1 + 0x2a66c);
  (**(code **)(**(int **)(param_1 + 0x2a3e4) + 0x94))
            (*(undefined4 *)(param_1 + 0x2a3e4),0x100,param_1 + 0x2a66c);
  _memset((void *)(param_1 + 0x2a894),0,0x400);
  for (local_c = 0; local_c < 0x100; local_c = local_c + 1) {
    *(undefined4 *)(param_1 + 0x2a894 + local_c * 4) = 0x7fffffff;
  }
  for (local_8 = 0; local_8 < 8; local_8 = local_8 + 1) {
    for (local_10 = 0; local_10 < 0x1d; local_10 = local_10 + 1) {
      *(undefined4 *)(param_1 + 0x2ac94 + local_8 * 0x74 + local_10 * 4) = 0x7fffffff;
    }
  }
  return 1;
}

