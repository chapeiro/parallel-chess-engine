#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cchapeiro.hpp"

typedef chapeiro::bitboard U64;

//source : http://chessprogramming.wikispaces.com/De+Bruijn+Sequence+Generator
class CGenBitScan {
public:
   //==========================================
   // constructor immediately starts the search
   //==========================================
   U64 generateBitScan(FILE * a) {
	  pd = a;
      m_Lock  = 0;
      m_dBCount = 0;
      m_Match4nth  = rand() & ((1<<26)-1);
      if (m_Match4nth <= 1) m_Match4nth = 4061955;
      initPow2();
      try {findDeBruijn(0, 64-6, 0);} catch(U64 e){return e;}
      return 0ull;
   }

private:
   U64 pow2[64];    // single bits
   U64 m_Lock;      // locks each bit used
   int m_dBCount;   // counter
   int m_Match4nth; // to match
   FILE * pd;
   //==========================================
   // on the fly initialization of pow2
   //==========================================
   void initPow2()  {
      pow2[0] = 1;
      for (int i=1; i < 64; i++)
         pow2[i] = 2*pow2[i-1];
   }

   //==========================================
   // print the bitscan routine and throw
   //==========================================
   void bitScanRoutineFound(U64 deBruijn) {
      int index[64], i;
      for (i=0; i<64; i++) // init magic array
         index[ (deBruijn<<i) >> (64-6) ] = i;
      fprintf(pd, "const bitboard magic = 0x%08x%08xull; // the %d.\n\n",
              (int)(deBruijn>>32), (int)(deBruijn), m_dBCount);
      fprintf(pd, "const unsigned int magictable[64] = {");
      for (i=0; i<64; i++) {
         if ( (i & 7) == 0 ) fprintf(pd, "\n\t");
         fprintf(pd, " %2d", index[i]);
         if (i!=63) fprintf(pd, ",");
      }
      fprintf(pd, "\n};\n");
      throw deBruijn; // unwind the stack until catched
   }

   //============================================
   // recursive search
   //============================================
   void findDeBruijn(U64 seq, int depth, int unique) {
      if ( (m_Lock & pow2[unique]) == 0 && unique != 32) {
         if ( depth == 0 ) {
            if ( ++m_dBCount == m_Match4nth )
               bitScanRoutineFound(seq);
         } else {
            m_Lock ^= pow2[unique];
            if ( depth > 2 && unique == 31 ) {
                findDeBruijn(seq | pow2[depth-1], depth-2, 62);
            } else {
                if ( depth > 1 )
                   findDeBruijn(seq, depth-1, (unique*2)&63);
                findDeBruijn(seq | pow2[depth-1], depth-1, (unique*2+1)&63);
            }
            m_Lock ^= pow2[unique];
         }
      }
   }
};
