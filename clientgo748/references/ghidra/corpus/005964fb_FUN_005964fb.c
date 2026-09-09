// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005964fb | Name: FUN_005964fb


void FUN_005964fb(undefined **param_1)

{
  VirtualFree(param_1[4],0,0x8000);
  if ((undefined **)PTR_LOOP_005cbf58 == param_1) {
    PTR_LOOP_005cbf58 = param_1[1];
  }
  if (param_1 != &PTR_LOOP_005c9f38) {
    *(undefined **)param_1[1] = *param_1;
    *(undefined **)(*param_1 + 4) = param_1[1];
    HeapFree(DAT_013b8604,0,param_1);
    return;
  }
  DAT_005c9f48 = 0xffffffff;
  return;
}

