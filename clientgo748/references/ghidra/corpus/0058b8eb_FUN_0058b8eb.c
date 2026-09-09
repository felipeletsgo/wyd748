// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058b8eb | Name: FUN_0058b8eb


void FUN_0058b8eb(int param_1,int param_2,short *param_3,int *param_4,int param_5)

{
  *(undefined1 *)(param_5 + *param_4) =
       *(undefined1 *)
        (((int)**(short **)(param_2 + 0x50) * (int)*param_3 + 4 >> 3 & 0x3ffU) + 0x80 +
        *(int *)(param_1 + 0x11c));
  return;
}

