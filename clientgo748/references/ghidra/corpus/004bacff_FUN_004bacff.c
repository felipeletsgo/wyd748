// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004bacff | Name: FUN_004bacff


void __fastcall FUN_004bacff(int param_1)

{
  uint uVar1;
  uint uVar2;
  float10 fVar3;
  LARGE_INTEGER local_c;
  
  if (*(int *)(param_1 + 0x28) == 0) {
    fVar3 = (float10)FUN_004baee0();
    *(double *)(param_1 + 0x18) =
         ((double)fVar3 - *(double *)(param_1 + 0x20)) + *(double *)(param_1 + 0x18);
    *(undefined4 *)(param_1 + 0x20) = 0;
    *(undefined4 *)(param_1 + 0x24) = 0;
    *(double *)(param_1 + 0x10) = (double)fVar3;
  }
  else {
    QueryPerformanceCounter(&local_c);
    uVar2 = local_c.s.LowPart - *(uint *)(param_1 + 0x30);
    uVar1 = *(uint *)(param_1 + 0x40);
    *(uint *)(param_1 + 0x40) = uVar1 + uVar2;
    *(uint *)(param_1 + 0x44) =
         *(int *)(param_1 + 0x44) +
         ((local_c.s.HighPart - *(int *)(param_1 + 0x34)) -
         (uint)(local_c.s.LowPart < *(uint *)(param_1 + 0x30))) + (uint)CARRY4(uVar1,uVar2);
    *(undefined4 *)(param_1 + 0x30) = 0;
    *(undefined4 *)(param_1 + 0x34) = 0;
    *(DWORD *)(param_1 + 0x38) = local_c.s.LowPart;
    *(LONG *)(param_1 + 0x3c) = local_c.s.HighPart;
  }
  return;
}

