#ifndef HEXSPEAK_H_1FOXEPIS
#define HEXSPEAK_H_1FOXEPIS

// 32 bit
#define HEX_DEAD_BEEF       0xDEADBEAF
#define HEX_DEAD_STATEMENT  0xDEAD57A7
#define HEX_ICE_BUDDHA      0x1CEB00DA
#define HEX_ICE_ICE_BABY    0x1CE1CEBB
#define HEX_FACE_BOOB       0xFACEB00B
#define HEX_DABBA_DOO       0xDABBAD00

// 16 bit
#define HEX_NOOP            0x081A
#define HEX_VOID            0x701D
#define HEX_NULL            0XABCD
#define HEX_DEAD            0xDEAD

#define IS_HEX_SPEAK(number)\
    number == HEX_DEAD_BEEF\
 || number == HEX_ICE_BUDDHA\
 || number == HEX_ICE_ICE_BABY\
 || number == HEX_FACE_BOOB\
 || number == HEX_DABBA_DOO\
 || number == HEX_NOOP\
 || number == HEX_VOID\
 || number == HEX_NULL\
 || number == HEX_DEAD

#endif /* end of include guard: HEXSPEAK_H_1FOXEPIS */
