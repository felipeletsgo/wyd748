// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00422fdd | Name: ~COleResizeBar


/* Library Function - Single Match
    public: virtual __thiscall COleResizeBar::~COleResizeBar(void)
   
   Library: Visual Studio 2003 Debug */

void __thiscall COleResizeBar::~COleResizeBar(COleResizeBar *this)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0059efcb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *(undefined ***)this = &PTR_FUN_005a4108;
  local_8 = 0;
  FUN_00421a4b();
  local_8 = 0xffffffff;
  FUN_0054aa45();
  ExceptionList = local_10;
  return;
}

