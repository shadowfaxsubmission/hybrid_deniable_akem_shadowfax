#include "fe.h"

/*
h = f * g
Can overlap h with f or g.

Preconditions:
   |f| bounded by 1.1*2^26,1.1*2^25,1.1*2^26,1.1*2^25,etc.
   |g| bounded by 1.1*2^26,1.1*2^25,1.1*2^26,1.1*2^25,etc.

Postconditions:
   |h| bounded by 1.1*2^25,1.1*2^24,1.1*2^25,1.1*2^24,etc.
*/

/*
Notes on implementation strategy:

Using schoolbook multiplication.
Karatsuba would save a little in some cost models.

Most multiplications by 2 and 19 are 32-bit precomputations;
cheaper than 64-bit postcomputations.

There is one remaining multiplication by 19 in the carry chain;
one *19 precomputation can be merged into this,
but the resulting data flow is considerably less clean.

There are 12 carries below.
10 of them are 2-way parallelizable and vectorizable.
Can get away with 11 carries, but then data flow is much deeper.

With tighter constraints on inputs can squeeze carries into int32.
*/

void fe_mul(fe h,fe f,fe g)
{
  int32_t f0 = f[0];
  int32_t f1 = f[1];
  int32_t f2 = f[2];
  int32_t f3 = f[3];
  int32_t f4 = f[4];
  int32_t f5 = f[5];
  int32_t f6 = f[6];
  int32_t f7 = f[7];
  int32_t f8 = f[8];
  int32_t f9 = f[9];
  int32_t g0 = g[0];
  int32_t g1 = g[1];
  int32_t g2 = g[2];
  int32_t g3 = g[3];
  int32_t g4 = g[4];
  int32_t g5 = g[5];
  int32_t g6 = g[6];
  int32_t g7 = g[7];
  int32_t g8 = g[8];
  int32_t g9 = g[9];
  int32_t g1_19 = 19 * g1; /* 1.4*2^29 */
  int32_t g2_19 = 19 * g2; /* 1.4*2^30; still ok */
  int32_t g3_19 = 19 * g3;
  int32_t g4_19 = 19 * g4;
  int32_t g5_19 = 19 * g5;
  int32_t g6_19 = 19 * g6;
  int32_t g7_19 = 19 * g7;
  int32_t g8_19 = 19 * g8;
  int32_t g9_19 = 19 * g9;
  int32_t f1_2 = 2 * f1;
  int32_t f3_2 = 2 * f3;
  int32_t f5_2 = 2 * f5;
  int32_t f7_2 = 2 * f7;
  int32_t f9_2 = 2 * f9;
  int64_t f0g0    = f0   * (int64_t) g0;
  int64_t f0g1    = f0   * (int64_t) g1;
  int64_t f0g2    = f0   * (int64_t) g2;
  int64_t f0g3    = f0   * (int64_t) g3;
  int64_t f0g4    = f0   * (int64_t) g4;
  int64_t f0g5    = f0   * (int64_t) g5;
  int64_t f0g6    = f0   * (int64_t) g6;
  int64_t f0g7    = f0   * (int64_t) g7;
  int64_t f0g8    = f0   * (int64_t) g8;
  int64_t f0g9    = f0   * (int64_t) g9;
  int64_t f1g0    = f1   * (int64_t) g0;
  int64_t f1g1_2  = f1_2 * (int64_t) g1;
  int64_t f1g2    = f1   * (int64_t) g2;
  int64_t f1g3_2  = f1_2 * (int64_t) g3;
  int64_t f1g4    = f1   * (int64_t) g4;
  int64_t f1g5_2  = f1_2 * (int64_t) g5;
  int64_t f1g6    = f1   * (int64_t) g6;
  int64_t f1g7_2  = f1_2 * (int64_t) g7;
  int64_t f1g8    = f1   * (int64_t) g8;
  int64_t f1g9_38 = f1_2 * (int64_t) g9_19;
  int64_t f2g0    = f2   * (int64_t) g0;
  int64_t f2g1    = f2   * (int64_t) g1;
  int64_t f2g2    = f2   * (int64_t) g2;
  int64_t f2g3    = f2   * (int64_t) g3;
  int64_t f2g4    = f2   * (int64_t) g4;
  int64_t f2g5    = f2   * (int64_t) g5;
  int64_t f2g6    = f2   * (int64_t) g6;
  int64_t f2g7    = f2   * (int64_t) g7;
  int64_t f2g8_19 = f2   * (int64_t) g8_19;
  int64_t f2g9_19 = f2   * (int64_t) g9_19;
  int64_t f3g0    = f3   * (int64_t) g0;
  int64_t f3g1_2  = f3_2 * (int64_t) g1;
  int64_t f3g2    = f3   * (int64_t) g2;
  int64_t f3g3_2  = f3_2 * (int64_t) g3;
  int64_t f3g4    = f3   * (int64_t) g4;
  int64_t f3g5_2  = f3_2 * (int64_t) g5;
  int64_t f3g6    = f3   * (int64_t) g6;
  int64_t f3g7_38 = f3_2 * (int64_t) g7_19;
  int64_t f3g8_19 = f3   * (int64_t) g8_19;
  int64_t f3g9_38 = f3_2 * (int64_t) g9_19;
  int64_t f4g0    = f4   * (int64_t) g0;
  int64_t f4g1    = f4   * (int64_t) g1;
  int64_t f4g2    = f4   * (int64_t) g2;
  int64_t f4g3    = f4   * (int64_t) g3;
  int64_t f4g4    = f4   * (int64_t) g4;
  int64_t f4g5    = f4   * (int64_t) g5;
  int64_t f4g6_19 = f4   * (int64_t) g6_19;
  int64_t f4g7_19 = f4   * (int64_t) g7_19;
  int64_t f4g8_19 = f4   * (int64_t) g8_19;
  int64_t f4g9_19 = f4   * (int64_t) g9_19;
  int64_t f5g0    = f5   * (int64_t) g0;
  int64_t f5g1_2  = f5_2 * (int64_t) g1;
  int64_t f5g2    = f5   * (int64_t) g2;
  int64_t f5g3_2  = f5_2 * (int64_t) g3;
  int64_t f5g4    = f5   * (int64_t) g4;
  int64_t f5g5_38 = f5_2 * (int64_t) g5_19;
  int64_t f5g6_19 = f5   * (int64_t) g6_19;
  int64_t f5g7_38 = f5_2 * (int64_t) g7_19;
  int64_t f5g8_19 = f5   * (int64_t) g8_19;
  int64_t f5g9_38 = f5_2 * (int64_t) g9_19;
  int64_t f6g0    = f6   * (int64_t) g0;
  int64_t f6g1    = f6   * (int64_t) g1;
  int64_t f6g2    = f6   * (int64_t) g2;
  int64_t f6g3    = f6   * (int64_t) g3;
  int64_t f6g4_19 = f6   * (int64_t) g4_19;
  int64_t f6g5_19 = f6   * (int64_t) g5_19;
  int64_t f6g6_19 = f6   * (int64_t) g6_19;
  int64_t f6g7_19 = f6   * (int64_t) g7_19;
  int64_t f6g8_19 = f6   * (int64_t) g8_19;
  int64_t f6g9_19 = f6   * (int64_t) g9_19;
  int64_t f7g0    = f7   * (int64_t) g0;
  int64_t f7g1_2  = f7_2 * (int64_t) g1;
  int64_t f7g2    = f7   * (int64_t) g2;
  int64_t f7g3_38 = f7_2 * (int64_t) g3_19;
  int64_t f7g4_19 = f7   * (int64_t) g4_19;
  int64_t f7g5_38 = f7_2 * (int64_t) g5_19;
  int64_t f7g6_19 = f7   * (int64_t) g6_19;
  int64_t f7g7_38 = f7_2 * (int64_t) g7_19;
  int64_t f7g8_19 = f7   * (int64_t) g8_19;
  int64_t f7g9_38 = f7_2 * (int64_t) g9_19;
  int64_t f8g0    = f8   * (int64_t) g0;
  int64_t f8g1    = f8   * (int64_t) g1;
  int64_t f8g2_19 = f8   * (int64_t) g2_19;
  int64_t f8g3_19 = f8   * (int64_t) g3_19;
  int64_t f8g4_19 = f8   * (int64_t) g4_19;
  int64_t f8g5_19 = f8   * (int64_t) g5_19;
  int64_t f8g6_19 = f8   * (int64_t) g6_19;
  int64_t f8g7_19 = f8   * (int64_t) g7_19;
  int64_t f8g8_19 = f8   * (int64_t) g8_19;
  int64_t f8g9_19 = f8   * (int64_t) g9_19;
  int64_t f9g0    = f9   * (int64_t) g0;
  int64_t f9g1_38 = f9_2 * (int64_t) g1_19;
  int64_t f9g2_19 = f9   * (int64_t) g2_19;
  int64_t f9g3_38 = f9_2 * (int64_t) g3_19;
  int64_t f9g4_19 = f9   * (int64_t) g4_19;
  int64_t f9g5_38 = f9_2 * (int64_t) g5_19;
  int64_t f9g6_19 = f9   * (int64_t) g6_19;
  int64_t f9g7_38 = f9_2 * (int64_t) g7_19;
  int64_t f9g8_19 = f9   * (int64_t) g8_19;
  int64_t f9g9_38 = f9_2 * (int64_t) g9_19;
  int64_t h0 = f0g0+f1g9_38+f2g8_19+f3g7_38+f4g6_19+f5g5_38+f6g4_19+f7g3_38+f8g2_19+f9g1_38;
  int64_t h1 = f0g1+f1g0   +f2g9_19+f3g8_19+f4g7_19+f5g6_19+f6g5_19+f7g4_19+f8g3_19+f9g2_19;
  int64_t h2 = f0g2+f1g1_2 +f2g0   +f3g9_38+f4g8_19+f5g7_38+f6g6_19+f7g5_38+f8g4_19+f9g3_38;
  int64_t h3 = f0g3+f1g2   +f2g1   +f3g0   +f4g9_19+f5g8_19+f6g7_19+f7g6_19+f8g5_19+f9g4_19;
  int64_t h4 = f0g4+f1g3_2 +f2g2   +f3g1_2 +f4g0   +f5g9_38+f6g8_19+f7g7_38+f8g6_19+f9g5_38;
  int64_t h5 = f0g5+f1g4   +f2g3   +f3g2   +f4g1   +f5g0   +f6g9_19+f7g8_19+f8g7_19+f9g6_19;
  int64_t h6 = f0g6+f1g5_2 +f2g4   +f3g3_2 +f4g2   +f5g1_2 +f6g0   +f7g9_38+f8g8_19+f9g7_38;
  int64_t h7 = f0g7+f1g6   +f2g5   +f3g4   +f4g3   +f5g2   +f6g1   +f7g0   +f8g9_19+f9g8_19;
  int64_t h8 = f0g8+f1g7_2 +f2g6   +f3g5_2 +f4g4   +f5g3_2 +f6g2   +f7g1_2 +f8g0   +f9g9_38;
  int64_t h9 = f0g9+f1g8   +f2g7   +f3g6   +f4g5   +f5g4   +f6g3   +f7g2   +f8g1   +f9g0   ;
  int64_t carry0;
  int64_t carry1;
  int64_t carry2;
  int64_t carry3;
  int64_t carry4;
  int64_t carry5;
  int64_t carry6;
  int64_t carry7;
  int64_t carry8;
  int64_t carry9;

  /*
  |h0| <= (1.1*1.1*2^52*(1+19+19+19+19)+1.1*1.1*2^50*(38+38+38+38+38))
    i.e. |h0| <= 1.2*2^59; narrower ranges for h2, h4, h6, h8
  |h1| <= (1.1*1.1*2^51*(1+1+19+19+19+19+19+19+19+19))
    i.e. |h1| <= 1.5*2^58; narrower ranges for h3, h5, h7, h9
  */

  carry0 = (h0 + (int64_t) (1<<25)) >> 26; h1 += carry0; h0 -= carry0 << 26;
  carry4 = (h4 + (int64_t) (1<<25)) >> 26; h5 += carry4; h4 -= carry4 << 26;
  /* |h0| <= 2^25 */
  /* |h4| <= 2^25 */
  /* |h1| <= 1.51*2^58 */
  /* |h5| <= 1.51*2^58 */

  carry1 = (h1 + (int64_t) (1<<24)) >> 25; h2 += carry1; h1 -= carry1 << 25;
  carry5 = (h5 + (int64_t) (1<<24)) >> 25; h6 += carry5; h5 -= carry5 << 25;
  /* |h1| <= 2^24; from now on fits into int32 */
  /* |h5| <= 2^24; from now on fits into int32 */
  /* |h2| <= 1.21*2^59 */
  /* |h6| <= 1.21*2^59 */

  carry2 = (h2 + (int64_t) (1<<25)) >> 26; h3 += carry2; h2 -= carry2 << 26;
  carry6 = (h6 + (int64_t) (1<<25)) >> 26; h7 += carry6; h6 -= carry6 << 26;
  /* |h2| <= 2^25; from now on fits into int32 unchanged */
  /* |h6| <= 2^25; from now on fits into int32 unchanged */
  /* |h3| <= 1.51*2^58 */
  /* |h7| <= 1.51*2^58 */

  carry3 = (h3 + (int64_t) (1<<24)) >> 25; h4 += carry3; h3 -= carry3 << 25;
  carry7 = (h7 + (int64_t) (1<<24)) >> 25; h8 += carry7; h7 -= carry7 << 25;
  /* |h3| <= 2^24; from now on fits into int32 unchanged */
  /* |h7| <= 2^24; from now on fits into int32 unchanged */
  /* |h4| <= 1.52*2^33 */
  /* |h8| <= 1.52*2^33 */

  carry4 = (h4 + (int64_t) (1<<25)) >> 26; h5 += carry4; h4 -= carry4 << 26;
  carry8 = (h8 + (int64_t) (1<<25)) >> 26; h9 += carry8; h8 -= carry8 << 26;
  /* |h4| <= 2^25; from now on fits into int32 unchanged */
  /* |h8| <= 2^25; from now on fits into int32 unchanged */
  /* |h5| <= 1.01*2^24 */
  /* |h9| <= 1.51*2^58 */

  carry9 = (h9 + (int64_t) (1<<24)) >> 25; h0 += carry9 * 19; h9 -= carry9 << 25;
  /* |h9| <= 2^24; from now on fits into int32 unchanged */
  /* |h0| <= 1.8*2^37 */

  carry0 = (h0 + (int64_t) (1<<25)) >> 26; h1 += carry0; h0 -= carry0 << 26;
  /* |h0| <= 2^25; from now on fits into int32 unchanged */
  /* |h1| <= 1.01*2^24 */

  h[0] = h0;
  h[1] = h1;
  h[2] = h2;
  h[3] = h3;
  h[4] = h4;
  h[5] = h5;
  h[6] = h6;
  h[7] = h7;
  h[8] = h8;
  h[9] = h9;
}
