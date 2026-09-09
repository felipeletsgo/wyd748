// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042d600 | Name: FUN_0042d600


void __fastcall FUN_0042d600(int param_1)

{
  if (*(int *)(param_1 + 0x2a85c) != 0) {
    DeleteObject(*(HGDIOBJ *)(param_1 + 0x2a85c));
    DeleteDC(*(HDC *)(param_1 + 0x2a858));
    DeleteObject(*(HGDIOBJ *)(param_1 + 0x2a854));
    *(undefined4 *)(param_1 + 0x2a85c) = 0;
    *(undefined4 *)(param_1 + 0x2a858) = 0;
    *(undefined4 *)(param_1 + 0x2a854) = 0;
  }
  return;
}

