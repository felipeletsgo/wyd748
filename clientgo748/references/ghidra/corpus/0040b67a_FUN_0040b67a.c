// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040b67a | Name: FUN_0040b67a


/* WARNING: Removing unreachable block (ram,0x0040b69a) */
/* WARNING: Removing unreachable block (ram,0x0040b69c) */
/* WARNING: Removing unreachable block (ram,0x0040b6a5) */

void __fastcall FUN_0040b67a(int param_1)

{
  *(int *)(*(int *)(param_1 + 0x334) + 0xa4) =
       (int)(((ulonglong)*(uint *)(param_1 + 0x394) / 1000000000) % 10);
  *(int *)(*(int *)(param_1 + 0x338) + 0xa4) =
       (int)(((ulonglong)*(uint *)(param_1 + 0x394) / 100000000) % 10);
  *(int *)(*(int *)(param_1 + 0x33c) + 0xa4) =
       (int)(((ulonglong)*(uint *)(param_1 + 0x394) / 10000000) % 10);
  *(int *)(*(int *)(param_1 + 0x340) + 0xa4) =
       (int)(((ulonglong)*(uint *)(param_1 + 0x394) / 1000000) % 10);
  *(int *)(*(int *)(param_1 + 0x344) + 0xa4) =
       (int)(((ulonglong)*(uint *)(param_1 + 0x394) / 100000) % 10);
  *(int *)(*(int *)(param_1 + 0x348) + 0xa4) =
       (int)(((ulonglong)*(uint *)(param_1 + 0x394) / 10000) % 10);
  *(int *)(*(int *)(param_1 + 0x34c) + 0xa4) =
       (int)(((ulonglong)*(uint *)(param_1 + 0x394) / 1000) % 10);
  *(int *)(*(int *)(param_1 + 0x350) + 0xa4) =
       (int)(((ulonglong)*(uint *)(param_1 + 0x394) / 100) % 10);
  *(int *)(*(int *)(param_1 + 0x354) + 0xa4) =
       (int)(((ulonglong)*(uint *)(param_1 + 0x394) / 10) % 10);
  *(uint *)(*(int *)(param_1 + 0x358) + 0xa4) = *(uint *)(param_1 + 0x394) % 10;
  if (*(uint *)(param_1 + 0x394) < 1000000000) {
    *(undefined4 *)(*(int *)(param_1 + 0x334) + 0xa4) = 10;
  }
  if (*(uint *)(param_1 + 0x394) < 100000000) {
    *(undefined4 *)(*(int *)(param_1 + 0x338) + 0xa4) = 10;
  }
  if (*(undefined **)(param_1 + 0x394) < &DAT_00989680) {
    *(undefined4 *)(*(int *)(param_1 + 0x33c) + 0xa4) = 10;
  }
  if (*(uint *)(param_1 + 0x394) < 1000000) {
    *(undefined4 *)(*(int *)(param_1 + 0x340) + 0xa4) = 10;
  }
  if (*(uint *)(param_1 + 0x394) < 100000) {
    *(undefined4 *)(*(int *)(param_1 + 0x344) + 0xa4) = 10;
  }
  if (*(uint *)(param_1 + 0x394) < 10000) {
    *(undefined4 *)(*(int *)(param_1 + 0x348) + 0xa4) = 10;
  }
  if (*(uint *)(param_1 + 0x394) < 1000) {
    *(undefined4 *)(*(int *)(param_1 + 0x34c) + 0xa4) = 10;
  }
  if (*(uint *)(param_1 + 0x394) < 100) {
    *(undefined4 *)(*(int *)(param_1 + 0x350) + 0xa4) = 10;
  }
  if (*(uint *)(param_1 + 0x394) < 10) {
    *(undefined4 *)(*(int *)(param_1 + 0x354) + 0xa4) = 10;
  }
  return;
}

