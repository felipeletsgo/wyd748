// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059e150 | Name: FUN_0059e150


undefined4 FUN_0059e150(HWND param_1)

{
  undefined4 *hMem;
  HRESULT HVar1;
  ULONG UVar2;
  int *unaff_ESI;
  LPUNKNOWN unaff_EDI;
  int *piVar3;
  int *piVar4;
  int *local_18 [6];
  
  hMem = GlobalAlloc(0,0x18);
  if (hMem == (undefined4 *)0x0) {
    return 0xffffffff;
  }
  ((LPOLECLIENTSITE)(hMem + 1))->lpVtbl = (IOleClientSiteVtbl *)&PTR_FUN_005ccd50;
  hMem[2] = &PTR_LAB_005ccdc0;
  hMem[3] = &PTR_LAB_005ccd10;
  hMem[4] = param_1;
  hMem[5] = &PTR_LAB_005ccd78;
  HVar1 = OleCreate((IID *)&DAT_005aa8f8,(IID *)&DAT_005aa8d8,1,(LPFORMATETC)0x0,
                    (LPOLECLIENTSITE)(hMem + 1),(LPSTORAGE)&PTR_PTR_005ccd08,local_18);
  if (HVar1 == 0) {
    *hMem = local_18[0];
    SetWindowLongA(param_1,-0x15,(LONG)hMem);
    piVar3 = (int *)0x0;
    (**(code **)(*local_18[0] + 0x14))(local_18[0],u_My_Host_Name_005ccdfc);
    GetClientRect(param_1,(LPRECT)&stack0xffffffe4);
    HVar1 = OleSetContainedObject(unaff_EDI,1);
    if (HVar1 == 0) {
      UVar2 = (*unaff_EDI->lpVtbl[3].Release)(unaff_EDI);
      if (UVar2 == 0) {
        HVar1 = (*unaff_EDI->lpVtbl->QueryInterface)
                          (unaff_EDI,(IID *)&DAT_005aa908,(void **)&stack0xffffffe0);
        if (HVar1 == 0) {
          (**(code **)(*unaff_ESI + 0x58))(unaff_ESI,0);
          piVar4 = piVar3;
          (**(code **)(*piVar3 + 0x60))(piVar3,0);
          (**(code **)(*local_18[0] + 0x68))(local_18[0],unaff_EDI);
          (**(code **)(*unaff_ESI + 0x70))(unaff_ESI,piVar4);
          (**(code **)(*piVar3 + 8))(piVar3);
          return 0;
        }
      }
    }
    FUN_0059dfe0(param_1);
    return 0xfffffffd;
  }
  GlobalFree(hMem);
  return 0xfffffffe;
}

