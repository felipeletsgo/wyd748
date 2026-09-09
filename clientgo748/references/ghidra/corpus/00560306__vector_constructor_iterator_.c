// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00560306 | Name: `vector_constructor_iterator'


/* Library Function - Single Match
    void __stdcall `vector constructor iterator'(void *,unsigned int,int,void * (__thiscall*)(void
   *))
   
   Library: Visual Studio 2003 Release */

void _vector_constructor_iterator_
               (void *param_1,uint param_2,int param_3,_func_void_ptr_void_ptr *param_4)

{
  void *unaff_EDI;
  
  if (-1 < param_3 + -1) {
    do {
      (*param_4)(unaff_EDI);
      param_3 = param_3 + -1;
    } while (param_3 != 0);
  }
  return;
}

