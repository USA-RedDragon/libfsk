/*
  FILE...: mpdecode_core.c
  AUTHOR.: Matthew C. Valenti, Rohit Iyer Seshadri, David Rowe
  CREATED: Sep 2016

  C-callable core functions moved from MpDecode.c, so they can be used for
  Octave and C programs.
*/

#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "mpdecode_core.h"

#ifdef __EMBEDDED__
#include "machdep.h"
#endif

/* Values for linear approximation (DecoderType=5) */

#define AJIAN -0.24904163195436
#define TJIAN 2.50681740420944

/* The linear-log-MAP algorithm */

static float max_star0(
                       float delta1,
                       float delta2 )
{
    register float diff;

    diff = delta2 - delta1;

    if ( diff > TJIAN )
        return( delta2 );
    else if ( diff < -TJIAN )
        return( delta1 );
    else if ( diff > 0 )
        return( delta2 + AJIAN*(diff-TJIAN) );
    else
        return( delta1 - AJIAN*(diff+TJIAN) );
}

void Somap(float  bit_likelihood[],      /* number_bits, bps*number_symbols */
           float  symbol_likelihood[],   /* M*number_symbols                */
           int    M,                     /* constellation size              */
           int    bps,                   /* bits per symbol                 */
           int    number_symbols)
{
    int    n,i,j,k,mask;
    float num[bps], den[bps];
    float metric;

    for (n=0; n<number_symbols; n++) { /* loop over symbols */
        for (k=0;k<bps;k++) {
            /* initialize */
            num[k] = -1000000;
            den[k] = -1000000;
        }

        for (i=0;i<M;i++) {
            metric =  symbol_likelihood[n*M+i]; /* channel metric for this symbol */

            mask = 1 << (bps - 1);
            for (j=0;j<bps;j++) {
                mask = mask >> 1;
            }
            mask = 1 << (bps - 1);

            for (k=0;k<bps;k++) {       /* loop over bits */
                if (mask&i) {
                    /* this bit is a one */
                    num[k] = max_star0( num[k], metric );
                } else {
                    /* this bit is a zero */
                    den[k] = max_star0( den[k], metric );
                }
                mask = mask >> 1;
            }
        }
        for (k=0;k<bps;k++) {
            bit_likelihood[bps*n+k] = num[k] - den[k];
        }
    }
}


/*
   Description: Transforms M-dimensional FSK symbols into ML symbol log-likelihoods

   The calling syntax is:
      [output] = FskDemod( input, EsNo, [csi_flag], [fade_coef] )

   Where:
      output    = M by N matrix of symbol log-likelihoods

      input     = M by N matrix of (complex) matched filter outputs
	  EsNo      = the symbol SNR (in linear, not dB, units)
	  csi_flag  = 0 for coherent reception (default)
	              1 for noncoherent reception w/ perfect amplitude estimates
		      2 for noncoherent reception without amplitude estimates
	  fade_coef = 1 by N matrix of (complex) fading coefficients (defaults to all-ones, i.e. AWGN)

   Copyright (C) 2006, Matthew C. Valenti

   Last updated on May 6, 2006

   Function DemodFSK is part of the Iterative Solutions
   Coded Modulation Library. The Iterative Solutions Coded Modulation
   Library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation; either version 2.1 of the License,
   or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

/* the logI_0 function */
static float logbesseli0(float x)
{
    if (x < 1)
        return( 0.226*x*x+0.0125*x-0.0012 );
    else if (x < 2)
        return( 0.1245*x*x+0.2177*x-0.108 );
    else if (x < 5)
        return( 0.0288*x*x+0.6314*x-0.5645 );
    else if (x < 20)
        return( 0.002*x*x+0.9048*x-1.2997 );
    else
        return(0.9867*x-2.2053);
}

/* Function that does the demodulation (can be used in stand-alone C) */

static void FskDemod(float out[], float yr[], float v_est, float SNR, int M, int number_symbols)
{
    int i, j;
    float y_envelope, scale_factor;

    scale_factor = 2*SNR;
    for (i=0;i<number_symbols;i++) {
        for (j=0;j<M;j++) {
            y_envelope = sqrt( yr[j*number_symbols+i]*yr[j*number_symbols+i]/(v_est*v_est));
            out[i*M+j] = logbesseli0( scale_factor*y_envelope );
        }
    }
}

void fsk_rx_filt_to_llrs(float llr[], float rx_filt[], float v_est, float SNRest, int M, int nsyms) {
    int i;
    int bps = log2(M);
    float symbol_likelihood[M*nsyms];
    float bit_likelihood[bps*nsyms];

    FskDemod(symbol_likelihood, rx_filt, v_est, SNRest, M, nsyms);
    Somap(bit_likelihood, symbol_likelihood, M, bps, nsyms);
    for(i=0; i<bps*nsyms; i++) {
        llr[i] = -bit_likelihood[i];
    }
}
