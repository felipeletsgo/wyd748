// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040a147 | Name: FUN_0040a147


undefined4 __fastcall FUN_0040a147(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = *(undefined4 *)(param_1 + 0x1e8);
  *(undefined4 *)(param_1 + 0x1e8) = 0;
  if (*(int *)(param_1 + 0x1e4) == 2) {
    *(undefined4 *)(param_1 + 0x1e4) = 0;
  }
  return uVar1;
}

