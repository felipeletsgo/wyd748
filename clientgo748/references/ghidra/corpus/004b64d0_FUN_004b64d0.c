// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b64d0 | Name: FUN_004b64d0


void __fastcall FUN_004b64d0(int param_1)

{
  FUN_004b6ae0(*(undefined4 *)(param_1 + 4),*(undefined4 *)(param_1 + 8));
  FUN_004b6b10(*(undefined4 *)(param_1 + 4),(*(int *)(param_1 + 0xc) - *(int *)(param_1 + 4)) / 0x28
              );
  *(undefined4 *)(param_1 + 4) = 0;
  *(undefined4 *)(param_1 + 8) = 0;
  *(undefined4 *)(param_1 + 0xc) = 0;
  return;
}

