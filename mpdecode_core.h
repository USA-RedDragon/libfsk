/*
  FILE...: mpdecode_core.h
  AUTHOR.: David Rowe
  CREATED: Sep 2016

  C-callable core functions for MpDecode, so they can be used for
  Octave and C programs.  Also some convenience functions to help use
  the C-callable LDPC decoder in C programs.
*/

#ifndef __MPDECODE_CORE__
#define __MPDECODE_CORE__

void Somap(float bit_likelihood[], float symbol_likelihood[], int M, int bps, int number_symbols);
void fsk_rx_filt_to_llrs(float llr[], float rx_filt[], float v_est, float SNRest, int M, int nsyms);

#endif
