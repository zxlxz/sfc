#pragma once

// clang-format off

// cat(a, b) => ab
#define SFC_CAT(a, b)       _SFC_CAT_A((a, b))
#define _SFC_CAT_A(...)     _SFC_CAT_B __VA_ARGS__
#define _SFC_CAT_B(a, b)    a##b

// count(...)
#define SFC_PP_CNT(...)     _SFC_PP_CNT(x,##__VA_ARGS__,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)
#define _SFC_PP_CNT(x,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,b0,b1,b2,b3,b4,b5,b6,b7,b8,b9,c0,c1,c2,c3,c4,c5,c6,c7,c8,c9,d0,d1,N, ...) N

#define SFC_PP_EMPTY()
#define SFC_PP_COMMA()  ,

// foreach(...)
#define SFC_PP_FOR(F, ...)      SFC_CAT(_SFC_PP_FOR_,SFC_PP_CNT(__VA_ARGS__))(SFC_PP_EMPTY, F, __VA_ARGS__)
#define SFC_PP_LIST(F, ...)     SFC_CAT(_SFC_PP_FOR_,SFC_PP_CNT(__VA_ARGS__))(SFC_PP_COMMA, F, __VA_ARGS__)

#define _SFC_PP_FOR_0( D,F,x, ...)
#define _SFC_PP_FOR_1( D,F,x, ...) F(x)
#define _SFC_PP_FOR_2( D,F,x, ...) F(x) D() _SFC_PP_FOR_1( D, F,__VA_ARGS__)
#define _SFC_PP_FOR_3( D,F,x, ...) F(x) D() _SFC_PP_FOR_2( D, F,__VA_ARGS__)
#define _SFC_PP_FOR_4( D,F,x, ...) F(x) D() _SFC_PP_FOR_3( D, F,__VA_ARGS__)
#define _SFC_PP_FOR_5( D,F,x, ...) F(x) D() _SFC_PP_FOR_4( D, F,__VA_ARGS__)
#define _SFC_PP_FOR_6( D,F,x, ...) F(x) D() _SFC_PP_FOR_5( D, F,__VA_ARGS__)
#define _SFC_PP_FOR_7( D,F,x, ...) F(x) D() _SFC_PP_FOR_6( D, F,__VA_ARGS__)
#define _SFC_PP_FOR_8( D,F,x, ...) F(x) D() _SFC_PP_FOR_7( D, F,__VA_ARGS__)
#define _SFC_PP_FOR_9( D,F,x, ...) F(x) D() _SFC_PP_FOR_8( D, F,__VA_ARGS__)
#define _SFC_PP_FOR_10(D,F,x, ...) F(x) D() _SFC_PP_FOR_9( D, F,__VA_ARGS__)
#define _SFC_PP_FOR_11(D,F,x, ...) F(x) D() _SFC_PP_FOR_10(D, F,__VA_ARGS__)
#define _SFC_PP_FOR_12(D,F,x, ...) F(x) D() _SFC_PP_FOR_11(D, F,__VA_ARGS__)
#define _SFC_PP_FOR_13(D,F,x, ...) F(x) D() _SFC_PP_FOR_12(D, F,__VA_ARGS__)
#define _SFC_PP_FOR_14(D,F,x, ...) F(x) D() _SFC_PP_FOR_13(D, F,__VA_ARGS__)
#define _SFC_PP_FOR_15(D,F,x, ...) F(x) D() _SFC_PP_FOR_14(D, F,__VA_ARGS__)
#define _SFC_PP_FOR_16(D,F,x, ...) F(x) D() _SFC_PP_FOR_15(D, F,__VA_ARGS__)
#define _SFC_PP_FOR_17(D,F,x, ...) F(x) D() _SFC_PP_FOR_16(D, F,__VA_ARGS__)
#define _SFC_PP_FOR_18(D,F,x, ...) F(x) D() _SFC_PP_FOR_17(D, F,__VA_ARGS__)
#define _SFC_PP_FOR_19(D,F,x, ...) F(x) D() _SFC_PP_FOR_18(D, F,__VA_ARGS__)
#define _SFC_PP_FOR_20(D,F,x, ...) F(x) D() _SFC_PP_FOR_19(D, F,__VA_ARGS__)
#define _SFC_PP_FOR_21(D,F,x, ...) F(x) D() _SFC_PP_FOR_20(D, F,__VA_ARGS__)
#define _SFC_PP_FOR_22(D,F,x, ...) F(x) D() _SFC_PP_FOR_21(D, F,__VA_ARGS__)
#define _SFC_PP_FOR_23(D,F,x, ...) F(x) D() _SFC_PP_FOR_22(D, F,__VA_ARGS__)
#define _SFC_PP_FOR_24(D,F,x, ...) F(x) D() _SFC_PP_FOR_23(D, F,__VA_ARGS__)
#define _SFC_PP_FOR_25(D,F,x, ...) F(x) D() _SFC_PP_FOR_24(D, F,__VA_ARGS__)
#define _SFC_PP_FOR_26(D,F,x, ...) F(x) D() _SFC_PP_FOR_25(D, F,__VA_ARGS__)
#define _SFC_PP_FOR_27(D,F,x, ...) F(x) D() _SFC_PP_FOR_26(D, F,__VA_ARGS__)
#define _SFC_PP_FOR_28(D,F,x, ...) F(x) D() _SFC_PP_FOR_27(D, F,__VA_ARGS__)
#define _SFC_PP_FOR_29(D,F,x, ...) F(x) D() _SFC_PP_FOR_28(D, F,__VA_ARGS__)
#define _SFC_PP_FOR_30(D,F,x, ...) F(x) D() _SFC_PP_FOR_29(D, F,__VA_ARGS__)
#define _SFC_PP_FOR_31(D,F,x, ...) F(x) D() _SFC_PP_FOR_30(D, F,__VA_ARGS__)
#define _SFC_PP_FOR_32(D,F,x, ...) F(x) D() _SFC_PP_FOR_31(D, F,__VA_ARGS__)
// clang-format on
