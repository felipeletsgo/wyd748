// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004afff7 | Name: FUN_004afff7


void __fastcall FUN_004afff7(int param_1)

{
  undefined4 *puVar1;
  DWORD DVar2;
  int local_c;
  
  DVar2 = timeGetTime();
  if (*(int *)(param_1 + 4) + 0x4b0U <= DVar2) {
    *(DWORD *)(param_1 + 4) = DVar2;
    for (local_c = 0; local_c < 0xbe8; local_c = local_c + 1) {
      if (((*(int *)(param_1 + 0x48 + local_c * 0x58) != 0) &&
          (*(uint *)(*(int *)(param_1 + 0x48 + local_c * 0x58) + 0x324) <
           DVar2 - *(int *)(*(int *)(param_1 + 0x48 + local_c * 0x58) + 800))) &&
         (*(int *)(param_1 + 0x48 + local_c * 0x58) != 0)) {
        puVar1 = *(undefined4 **)(param_1 + 0x48 + local_c * 0x58);
        if (puVar1 != (undefined4 *)0x0) {
          (**(code **)*puVar1)(1);
        }
        *(undefined4 *)(param_1 + 0x48 + local_c * 0x58) = 0;
      }
    }
  }
  return;
}

