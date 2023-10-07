#ifndef CONSTS_H
#define CONSTS_H

#define STB PORTD7
#define CLK PORTD6
#define DIO PORTD5
#define SPIALL ((1 << STB) | (1 << CLK) | (1 << DIO))
#define DTIME 5

// #define INTPER 65523 // through trial and error, this one is about 32 kHz
// about 15 us of thinking time !
#define INTPER 1

#define NP_0 0x00000004
#define NP_1 0x00200000
#define NP_2 0x02000000
#define NP_3 0x20000000
#define NP_4 0x00000200
#define NP_5 0x00002000
#define NP_6 0x00020000
#define NP_7 0x40000000
#define NP_8 0x00000002
#define NP_9 0x00000020
#define NP_DEC 0x00000001
#define NP_ENT 0x00000400
#define NP_ADD 0x00000040
#define NP_SUB 0x04000000
#define NP_MUL 0x00400000
#define NP_DIV 0x00040000
#define NP_NUM 0x00004000

#define SEG_0 0x3f
#define SEG_1 0x06
#define SEG_2 0x5b
#define SEG_3 0x4f
#define SEG_4 0x66
#define SEG_5 0x6d
#define SEG_6 0x7d
#define SEG_7 0x07
#define SEG_8 0x7f
#define SEG_9 0x6f
#define SEG_A 0x77
#define SEG_B 0x7c
#define SEG_C 0x39
#define SEG_D 0x5e
#define SEG_E 0x79
#define SEG_F 0x71

//     0
//    ---
// 5 | 6 | 1
//    ---
// 4 |   | 2
//    --- .
//     3  7
//

#define SEGPART_0 0x01
#define SEGPART_1 0x02
#define SEGPART_2 0x04
#define SEGPART_3 0x08
#define SEGPART_4 0x10
#define SEGPART_5 0x20
#define SEGPART_6 0x40
#define SEGPART_7 0x80

#endif
