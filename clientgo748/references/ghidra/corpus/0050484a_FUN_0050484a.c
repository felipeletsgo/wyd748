// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0050484a | Name: FUN_0050484a


void __fastcall FUN_0050484a(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  if (*(int *)(param_1 + 0x214) == 0) {
    if (*(short *)(param_1 + 0x680) == 0) {
      *(undefined4 *)(param_1 + 0x878) = 1;
      (**(code **)(**(int **)(param_1 + 0x7e4) + 0x60))(0);
    }
    else {
      uVar1 = FUN_00552e21((int)*(short *)(param_1 + 0x682));
      *(undefined4 *)(param_1 + 0x7e8) = uVar1;
      *(uint *)(param_1 + 0x7f0) = *(ushort *)(param_1 + 0x680) & 0xfff;
      *(undefined4 *)(param_1 + 0x7e8) = uVar1;
      *(int *)(param_1 + 0x7ec) = (int)(uint)*(ushort *)(param_1 + 0x680) >> 0xc;
      *(undefined2 *)(param_1 + 0x880) = *(undefined2 *)(param_1 + 0x682);
      if ((DAT_0067cf38 != 0) && (iVar2 = FUN_0040c0f0(), iVar2 == 0)) {
        FUN_0047dee5(param_1 + 0x7e4);
      }
    }
  }
  return;
}

