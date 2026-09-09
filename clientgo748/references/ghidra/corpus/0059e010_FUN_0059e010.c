// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059e010 | Name: FUN_0059e010


undefined4 FUN_0059e010(HWND param_1)

{
  undefined4 *puVar1;
  int iVar2;
  OLECHAR *lpWideCharStr;
  BSTR pOVar3;
  int *unaff_EDI;
  int *piStack_8;
  LPCSTR pCStack_4;
  
  puVar1 = (undefined4 *)GetWindowLongA(param_1,-0x15);
  iVar2 = (*(code *)**(undefined4 **)*puVar1)();
  if (iVar2 != 0) {
    return 0xfffffffb;
  }
  VariantInit((VARIANTARG *)&stack0xffffffe4);
  iVar2 = MultiByteToWideChar(0,0,pCStack_4,-1,(LPWSTR)0x0,0);
  lpWideCharStr = GlobalAlloc(0,iVar2 * 2);
  if (lpWideCharStr != (OLECHAR *)0x0) {
    MultiByteToWideChar(0,0,pCStack_4,-1,lpWideCharStr,iVar2);
    pOVar3 = SysAllocString(lpWideCharStr);
    GlobalFree(lpWideCharStr);
    if (pOVar3 != (BSTR)0x0) {
      (**(code **)(*piStack_8 + 0xd0))(piStack_8,&stack0xffffffe4,0,0);
      VariantClear((VARIANTARG *)&stack0xffffffcc);
      (**(code **)(*unaff_EDI + 8))(unaff_EDI);
      return 0;
    }
  }
  (**(code **)(*piStack_8 + 8))();
  return 0xfffffffa;
}

