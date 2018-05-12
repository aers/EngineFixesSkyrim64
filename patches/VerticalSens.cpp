// static int magic = 0x3CC0C0C0; // 1 / 42.5
// RelocPtr<float> TimerStep_WithoutSlow(0x02F9294C);
// RelocAddr<uintptr_t> TimerHook(0x00850D81);
//
// Xbyak::Label retnLabel;
// Xbyak::Label magicLabel;
// Xbyak::Label timerLabel;
//
// // enter 850D81
//
// // r8 is unused
// //.text:0000000140850D81                 movss   xmm4, cs:frame_timer_without_slow
// // use magic instead
// mov(r8, ptr[rip + magicLabel]);
// movss(xmm4, dword[r8]);
// //.text:0000000140850D89                 movaps  xmm3, xmm4
// // use timer
// mov(r8, ptr[rip + timerLabel]);
// movss(xmm3, dword[r8]);
//
// // exit 850D8C
// jmp(ptr[rip + retnLabel]);
//
// L(retnLabel);
// dq(TimerHook.GetUIntPtr() + 0xB);
//
// L(magicLabel);
// dq(uintptr_t(&magic));
//
// L(timerLabel);
// dq(TimerStep_WithoutSlow.GetUIntPtr());
