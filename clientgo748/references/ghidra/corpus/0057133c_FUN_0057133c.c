// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057133c | Name: FUN_0057133c


int FUN_0057133c(int param_1)

{
  int iVar1;
  undefined **ppuVar2;
  undefined **ppuVar3;
  int local_8;
  
  if (param_1 == 0) {
    DAT_005c6074 = 0xffff;
    ppuVar2 = &PTR_FUN_005c5170;
    ppuVar3 = &PTR_LAB_005c5088;
    for (iVar1 = 0x39; iVar1 != 0; iVar1 = iVar1 + -1) {
      *ppuVar3 = *ppuVar2;
      ppuVar2 = ppuVar2 + 1;
      ppuVar3 = ppuVar3 + 1;
    }
  }
  else if (DAT_005c6074 == 0xffff) {
    DAT_005c6074 = 0;
    ppuVar2 = &PTR_FUN_005c5170;
    ppuVar3 = &PTR_LAB_005c5088;
    for (iVar1 = 0x39; iVar1 != 0; iVar1 = iVar1 + -1) {
      *ppuVar3 = *ppuVar2;
      ppuVar2 = ppuVar2 + 1;
      ppuVar3 = ppuVar3 + 1;
    }
    FUN_00577df5(&PTR_LAB_005c5088);
    iVar1 = FUN_00571091(4,"DisablePSGP",&param_1,4);
    if (iVar1 == 0) {
      param_1 = 0;
    }
    iVar1 = FUN_00571091(4,"DisableD3DXPSGP",&local_8,4);
    if (iVar1 != 0) {
      param_1 = local_8;
    }
    if (param_1 != 1) {
      iVar1 = FUN_00571288(7);
      if (iVar1 == 0) {
        iVar1 = FUN_00571288(10);
        if (iVar1 == 0) {
          iVar1 = FUN_00571288(6);
          if (iVar1 != 0) {
            FUN_00577480(&PTR_LAB_005c5088);
            DAT_005c6074 = 3;
          }
        }
        else {
          FUN_00577590(&PTR_LAB_005c5088);
          DAT_005c6074 = 2;
        }
      }
      else {
        FUN_00577773(&PTR_LAB_005c5088);
        DAT_005c6074 = 1;
      }
    }
  }
  return DAT_005c6074;
}

