/* Generated automatically by the program `genextract'
   from the machine description file `md'.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"
#include "insn-config.h"
#include "recog.h"
#include "toplev.h"

/* This variable is used as the "location" of any missing operand
   whose numbers are skipped by a given pattern.  */
static rtx junk ATTRIBUTE_UNUSED;

void
insn_extract (rtx insn)
{
  rtx *ro = recog_data.operand;
  rtx **ro_loc = recog_data.operand_loc;
  rtx pat = PATTERN (insn);
  int i ATTRIBUTE_UNUSED; /* only for peepholes */

#ifdef ENABLE_CHECKING
  memset (ro, 0xab, sizeof (*ro) * MAX_RECOG_OPERANDS);
  memset (ro_loc, 0xab, sizeof (*ro_loc) * MAX_RECOG_OPERANDS);
#endif

  switch (INSN_CODE (insn))
    {
    default:
      /* Control reaches here if insn_extract has been called with an
         unrecognizable insn (code -1), or an insn whose INSN_CODE
         corresponds to a DEFINE_EXPAND in the machine description;
         either way, a bug.  */
      if (INSN_CODE (insn) < 0)
        fatal_insn ("unrecognizable insn:", insn);
      else
        fatal_insn ("insn with invalid code number:", insn);

    case 2080:  /* sync_xordi */
    case 2079:  /* sync_iordi */
    case 2078:  /* sync_anddi */
    case 2077:  /* sync_xorsi */
    case 2076:  /* sync_iorsi */
    case 2075:  /* sync_andsi */
    case 2074:  /* sync_xorhi */
    case 2073:  /* sync_iorhi */
    case 2072:  /* sync_andhi */
    case 2071:  /* sync_xorqi */
    case 2070:  /* sync_iorqi */
    case 2069:  /* sync_andqi */
    case 2068:  /* sync_subdi */
    case 2067:  /* sync_subsi */
    case 2066:  /* sync_subhi */
    case 2065:  /* sync_subqi */
    case 2064:  /* sync_adddi */
    case 2063:  /* sync_addsi */
    case 2062:  /* sync_addhi */
    case 2061:  /* sync_addqi */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0), 1));
      recog_data.dup_loc[0] = &XEXP (XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0), 0);
      recog_data.dup_num[0] = 0;
      break;

    case 2060:  /* sync_lock_test_and_setdi */
    case 2059:  /* sync_lock_test_and_setsi */
    case 2058:  /* sync_lock_test_and_sethi */
    case 2057:  /* sync_lock_test_and_setqi */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 1), 1));
      recog_data.dup_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0);
      recog_data.dup_num[0] = 1;
      break;

    case 2056:  /* sync_old_adddi */
    case 2055:  /* sync_old_addsi */
    case 2054:  /* sync_old_addhi */
    case 2053:  /* sync_old_addqi */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 1));
      recog_data.dup_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0);
      recog_data.dup_num[1] = 1;
      break;

    case 2052:  /* *sync_double_compare_and_swapdi_pic */
    case 2051:  /* sync_double_compare_and_swapti */
    case 2050:  /* sync_double_compare_and_swapdi */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 0), 1));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 1));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 2));
      ro[4] = *(ro_loc[4] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 3));
      recog_data.dup_loc[0] = &XVECEXP (XEXP (XEXP (XVECEXP (pat, 0, 2), 1), 0), 0, 3);
      recog_data.dup_num[0] = 4;
      recog_data.dup_loc[1] = &XVECEXP (XEXP (XEXP (XVECEXP (pat, 0, 2), 1), 0), 0, 2);
      recog_data.dup_num[1] = 3;
      recog_data.dup_loc[2] = &XVECEXP (XEXP (XEXP (XVECEXP (pat, 0, 2), 1), 0), 0, 1);
      recog_data.dup_num[2] = 2;
      recog_data.dup_loc[3] = &XVECEXP (XEXP (XEXP (XVECEXP (pat, 0, 2), 1), 0), 0, 0);
      recog_data.dup_num[3] = 1;
      recog_data.dup_loc[4] = &XEXP (XEXP (XVECEXP (pat, 0, 2), 1), 1);
      recog_data.dup_num[4] = 2;
      recog_data.dup_loc[5] = &XEXP (XVECEXP (pat, 0, 1), 0);
      recog_data.dup_num[5] = 1;
      recog_data.dup_loc[6] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 0);
      recog_data.dup_num[6] = 1;
      break;

    case 2049:  /* *sync_compare_and_swapdi */
    case 2048:  /* *sync_compare_and_swapsi */
    case 2047:  /* *sync_compare_and_swaphi */
    case 2046:  /* *sync_compare_and_swapqi */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 0), 1));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 1));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 2));
      recog_data.dup_loc[0] = &XVECEXP (XEXP (XEXP (XVECEXP (pat, 0, 2), 1), 0), 0, 2);
      recog_data.dup_num[0] = 3;
      recog_data.dup_loc[1] = &XVECEXP (XEXP (XEXP (XVECEXP (pat, 0, 2), 1), 0), 0, 1);
      recog_data.dup_num[1] = 2;
      recog_data.dup_loc[2] = &XVECEXP (XEXP (XEXP (XVECEXP (pat, 0, 2), 1), 0), 0, 0);
      recog_data.dup_num[2] = 1;
      recog_data.dup_loc[3] = &XEXP (XEXP (XVECEXP (pat, 0, 2), 1), 1);
      recog_data.dup_num[3] = 2;
      recog_data.dup_loc[4] = &XEXP (XVECEXP (pat, 0, 1), 0);
      recog_data.dup_num[4] = 1;
      recog_data.dup_loc[5] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 0);
      recog_data.dup_num[5] = 1;
      break;

    case 2045:  /* memory_barrier_nosse */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      recog_data.dup_loc[0] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0);
      recog_data.dup_num[0] = 0;
      break;

    case 2032:  /* avx_maskstorepd256 */
    case 2031:  /* avx_maskstoreps256 */
    case 2030:  /* avx_maskstorepd */
    case 2029:  /* avx_maskstoreps */
    case 2028:  /* avx_maskloadpd256 */
    case 2027:  /* avx_maskloadps256 */
    case 2026:  /* avx_maskloadpd */
    case 2025:  /* avx_maskloadps */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (pat, 1), 0, 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (pat, 1), 0, 1));
      recog_data.dup_loc[0] = &XVECEXP (XEXP (pat, 1), 0, 2);
      recog_data.dup_num[0] = 0;
      break;

    case 2012:  /* *avx_vperm2f128v4df_nozero */
    case 2011:  /* *avx_vperm2f128v8sf_nozero */
    case 2010:  /* *avx_vperm2f128v8si_nozero */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (pat, 1), 0), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (pat, 1), 1));
      ro[4] = *(ro_loc[4] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 0));
      break;

    case 2002:  /* *avx_vpermilpv4df */
    case 2001:  /* *avx_vpermilpv8sf */
    case 2000:  /* *avx_vpermilpv2df */
    case 1999:  /* *avx_vpermilpv4sf */
    case 1998:  /* *avx_vperm_broadcast_v4df */
    case 1997:  /* *avx_vperm_broadcast_v8sf */
    case 1996:  /* *avx_vperm_broadcast_v4sf */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (pat, 1), 1));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 0));
      break;

    case 1963:  /* xop_maskcmp_uns2v2di3 */
    case 1962:  /* xop_maskcmp_uns2v4si3 */
    case 1961:  /* xop_maskcmp_uns2v8hi3 */
    case 1960:  /* xop_maskcmp_uns2v16qi3 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (pat, 1), 0, 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 1));
      break;

    case 1949:  /* xop_vmfrczv2df2 */
    case 1948:  /* xop_vmfrczv4sf2 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 1));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XEXP (pat, 1), 0), 0, 0));
      break;

    case 1945:  /* xop_lshlv2di3 */
    case 1944:  /* xop_lshlv4si3 */
    case 1943:  /* xop_lshlv8hi3 */
    case 1942:  /* xop_lshlv16qi3 */
    case 1941:  /* xop_ashlv2di3 */
    case 1940:  /* xop_ashlv4si3 */
    case 1939:  /* xop_ashlv8hi3 */
    case 1938:  /* xop_ashlv16qi3 */
    case 1937:  /* xop_vrotlv2di3 */
    case 1936:  /* xop_vrotlv4si3 */
    case 1935:  /* xop_vrotlv8hi3 */
    case 1934:  /* xop_vrotlv16qi3 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (pat, 1), 1), 1);
      recog_data.dup_num[0] = 2;
      recog_data.dup_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 2), 0);
      recog_data.dup_num[1] = 1;
      recog_data.dup_loc[2] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 2), 1), 0);
      recog_data.dup_num[2] = 2;
      break;

    case 1925:  /* xop_pperm_pack_v8hi_v16qi */
    case 1924:  /* xop_pperm_pack_v4si_v8hi */
    case 1923:  /* xop_pperm_pack_v2di_v4si */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (pat, 0, 1), 0));
      break;

    case 1915:  /* xop_phaddubq */
    case 1909:  /* xop_phaddbq */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0), 0), 0));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 1), 0), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 0), 0), 0);
      recog_data.dup_num[1] = 1;
      recog_data.dup_loc[2] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 1), 0), 0);
      recog_data.dup_num[2] = 1;
      recog_data.dup_loc[3] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 0), 0), 0), 0);
      recog_data.dup_num[3] = 1;
      recog_data.dup_loc[4] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 0), 1), 0), 0);
      recog_data.dup_num[4] = 1;
      recog_data.dup_loc[5] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 1), 0), 0), 0);
      recog_data.dup_num[5] = 1;
      recog_data.dup_loc[6] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 1), 1), 0), 0);
      recog_data.dup_num[6] = 1;
      break;

    case 1917:  /* xop_phadduwq */
    case 1914:  /* xop_phaddubd */
    case 1911:  /* xop_phaddwq */
    case 1908:  /* xop_phaddbd */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0), 0));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 0), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 0), 0), 0);
      recog_data.dup_num[1] = 1;
      recog_data.dup_loc[2] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 1), 0), 0);
      recog_data.dup_num[2] = 1;
      break;

    case 1921:  /* xop_phsubdq */
    case 1920:  /* xop_phsubwd */
    case 1919:  /* xop_phsubbw */
    case 1918:  /* xop_phaddudq */
    case 1916:  /* xop_phadduwd */
    case 1913:  /* xop_phaddubw */
    case 1912:  /* xop_phadddq */
    case 1910:  /* xop_phaddwd */
    case 1907:  /* xop_phaddbw */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 0), 0);
      recog_data.dup_num[0] = 1;
      break;

    case 1906:  /* xop_pcmov_v4df256 */
    case 1905:  /* xop_pcmov_v8sf256 */
    case 1904:  /* xop_pcmov_v4di256 */
    case 1903:  /* xop_pcmov_v8si256 */
    case 1902:  /* xop_pcmov_v16hi256 */
    case 1901:  /* xop_pcmov_v32qi256 */
    case 1900:  /* xop_pcmov_v2df */
    case 1899:  /* xop_pcmov_v4sf */
    case 1898:  /* xop_pcmov_v2di */
    case 1897:  /* xop_pcmov_v4si */
    case 1896:  /* xop_pcmov_v8hi */
    case 1895:  /* xop_pcmov_v16qi */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 1));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (pat, 1), 2));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (pat, 1), 0));
      break;

    case 1894:  /* xop_pmadcswd */
    case 1893:  /* xop_pmadcsswd */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 1), 0), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (pat, 1), 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 0), 0), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 1), 0), 0);
      recog_data.dup_num[1] = 2;
      break;

    case 1892:  /* xop_pmacswd */
    case 1891:  /* xop_pmacsswd */
    case 1889:  /* xop_pmacsdqh */
    case 1887:  /* xop_pmacsdql */
    case 1886:  /* xop_pmacssdqh */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 0), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (pat, 1), 1));
      break;

    case 1885:  /* xop_pmacssdql */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (pat, 1), 1));
      break;

    case 1880:  /* sse4_2_pcmpistr_cconly */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 2), 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 1));
      ro[4] = *(ro_loc[4] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 2));
      break;

    case 1879:  /* sse4_2_pcmpistrm */
    case 1878:  /* sse4_2_pcmpistri */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 1));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 2));
      recog_data.dup_loc[0] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 2);
      recog_data.dup_num[0] = 3;
      recog_data.dup_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 1);
      recog_data.dup_num[1] = 2;
      recog_data.dup_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 0);
      recog_data.dup_num[2] = 1;
      break;

    case 1877:  /* sse4_2_pcmpistr */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 1));
      ro[4] = *(ro_loc[4] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 2));
      recog_data.dup_loc[0] = &XVECEXP (XEXP (XVECEXP (pat, 0, 2), 1), 0, 2);
      recog_data.dup_num[0] = 4;
      recog_data.dup_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 2), 1), 0, 1);
      recog_data.dup_num[1] = 3;
      recog_data.dup_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 2), 1), 0, 0);
      recog_data.dup_num[2] = 2;
      recog_data.dup_loc[3] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 2);
      recog_data.dup_num[3] = 4;
      recog_data.dup_loc[4] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 1);
      recog_data.dup_num[4] = 3;
      recog_data.dup_loc[5] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 0);
      recog_data.dup_num[5] = 2;
      break;

    case 1876:  /* sse4_2_pcmpestr_cconly */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 2), 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 1));
      ro[4] = *(ro_loc[4] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 2));
      ro[5] = *(ro_loc[5] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 3));
      ro[6] = *(ro_loc[6] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 4));
      break;

    case 1875:  /* sse4_2_pcmpestrm */
    case 1874:  /* sse4_2_pcmpestri */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 1));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 2));
      ro[4] = *(ro_loc[4] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 3));
      ro[5] = *(ro_loc[5] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 4));
      recog_data.dup_loc[0] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 4);
      recog_data.dup_num[0] = 5;
      recog_data.dup_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 3);
      recog_data.dup_num[1] = 4;
      recog_data.dup_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 2);
      recog_data.dup_num[2] = 3;
      recog_data.dup_loc[3] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 1);
      recog_data.dup_num[3] = 2;
      recog_data.dup_loc[4] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 0);
      recog_data.dup_num[4] = 1;
      break;

    case 1873:  /* sse4_2_pcmpestr */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 1));
      ro[4] = *(ro_loc[4] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 2));
      ro[5] = *(ro_loc[5] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 3));
      ro[6] = *(ro_loc[6] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 4));
      recog_data.dup_loc[0] = &XVECEXP (XEXP (XVECEXP (pat, 0, 2), 1), 0, 4);
      recog_data.dup_num[0] = 6;
      recog_data.dup_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 2), 1), 0, 3);
      recog_data.dup_num[1] = 5;
      recog_data.dup_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 2), 1), 0, 2);
      recog_data.dup_num[2] = 4;
      recog_data.dup_loc[3] = &XVECEXP (XEXP (XVECEXP (pat, 0, 2), 1), 0, 1);
      recog_data.dup_num[3] = 3;
      recog_data.dup_loc[4] = &XVECEXP (XEXP (XVECEXP (pat, 0, 2), 1), 0, 0);
      recog_data.dup_num[4] = 2;
      recog_data.dup_loc[5] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 4);
      recog_data.dup_num[5] = 6;
      recog_data.dup_loc[6] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 3);
      recog_data.dup_num[6] = 5;
      recog_data.dup_loc[7] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 2);
      recog_data.dup_num[7] = 4;
      recog_data.dup_loc[8] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 1);
      recog_data.dup_num[8] = 3;
      recog_data.dup_loc[9] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 0);
      recog_data.dup_num[9] = 2;
      break;

    case 1872:  /* sse4_1_roundsd */
    case 1871:  /* sse4_1_roundss */
    case 1870:  /* *avx_roundsd */
    case 1869:  /* *avx_roundss */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 1));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XEXP (pat, 1), 0), 0, 0));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XEXP (pat, 1), 0), 0, 1));
      break;

    case 1864:  /* sse4_1_ptest */
    case 1863:  /* avx_ptest256 */
    case 1862:  /* avx_vtestpd256 */
    case 1861:  /* avx_vtestps256 */
    case 1860:  /* avx_vtestpd */
    case 1859:  /* avx_vtestps */
      ro[0] = *(ro_loc[0] = &XVECEXP (XEXP (pat, 1), 0, 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (pat, 1), 0, 1));
      break;

    case 1845:  /* sse4_1_pblendw */
    case 1844:  /* *avx_pblendw */
    case 1828:  /* sse4_1_blendpd */
    case 1827:  /* sse4_1_blendps */
    case 1822:  /* avx_blendpd256 */
    case 1821:  /* avx_blendps256 */
    case 1820:  /* avx_blendpd */
    case 1819:  /* avx_blendps */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 1));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (pat, 1), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (pat, 1), 2));
      break;

    case 1971:  /* xop_vpermil2v4df3 */
    case 1970:  /* xop_vpermil2v8sf3 */
    case 1969:  /* xop_vpermil2v2df3 */
    case 1968:  /* xop_vpermil2v4sf3 */
    case 1817:  /* sse4a_insertqi */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (pat, 1), 0, 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (pat, 1), 0, 1));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (pat, 1), 0, 2));
      ro[4] = *(ro_loc[4] = &XVECEXP (XEXP (pat, 1), 0, 3));
      break;

    case 1789:  /* *ssse3_pmulhrswv4hi3 */
    case 1788:  /* *ssse3_pmulhrswv8hi3 */
    case 1787:  /* *avx_pmulhrswv8hi3 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0), 0), 1), 0));
      break;

    case 1782:  /* ssse3_phsubswv8hi3 */
    case 1781:  /* *avx_phsubswv8hi3 */
    case 1776:  /* ssse3_phsubwv8hi3 */
    case 1775:  /* *avx_phsubwv8hi3 */
    case 1773:  /* ssse3_phaddswv8hi3 */
    case 1772:  /* *avx_phaddswv8hi3 */
    case 1767:  /* ssse3_phaddwv8hi3 */
    case 1766:  /* *avx_phaddwv8hi3 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 0), 0), 0), 0));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0), 1), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 1), 0), 0);
      recog_data.dup_num[1] = 1;
      recog_data.dup_loc[2] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 1), 1), 0);
      recog_data.dup_num[2] = 1;
      recog_data.dup_loc[3] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 0), 0), 0);
      recog_data.dup_num[3] = 1;
      recog_data.dup_loc[4] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 0), 1), 0);
      recog_data.dup_num[4] = 1;
      recog_data.dup_loc[5] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 1), 0), 0);
      recog_data.dup_num[5] = 1;
      recog_data.dup_loc[6] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 1), 1), 0);
      recog_data.dup_num[6] = 1;
      recog_data.dup_loc[7] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 0), 0), 1), 0);
      recog_data.dup_num[7] = 2;
      recog_data.dup_loc[8] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 0), 1), 0), 0);
      recog_data.dup_num[8] = 2;
      recog_data.dup_loc[9] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 0), 1), 1), 0);
      recog_data.dup_num[9] = 2;
      recog_data.dup_loc[10] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 1), 0), 0), 0);
      recog_data.dup_num[10] = 2;
      recog_data.dup_loc[11] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 1), 0), 1), 0);
      recog_data.dup_num[11] = 2;
      recog_data.dup_loc[12] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 1), 1), 0), 0);
      recog_data.dup_num[12] = 2;
      recog_data.dup_loc[13] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 1), 1), 1), 0);
      recog_data.dup_num[13] = 2;
      break;

    case 1763:  /* sse3_mwait */
      ro[0] = *(ro_loc[0] = &XVECEXP (pat, 0, 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (pat, 0, 1));
      break;

    case 1716:  /* sse2_pshufhw_1 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 4));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 5));
      ro[4] = *(ro_loc[4] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 6));
      ro[5] = *(ro_loc[5] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 7));
      break;

    case 1995:  /* avx_vbroadcastf128_v4df */
    case 1994:  /* avx_vbroadcastf128_v8sf */
    case 1993:  /* avx_vbroadcastf128_v4di */
    case 1992:  /* avx_vbroadcastf128_v8si */
    case 1991:  /* avx_vbroadcastf128_v16hi */
    case 1990:  /* avx_vbroadcastf128_v32qi */
    case 1502:  /* *vec_concatv2df_sse3 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 0));
      recog_data.dup_loc[0] = &XEXP (XEXP (pat, 1), 1);
      recog_data.dup_num[0] = 1;
      break;

    case 1488:  /* sse2_shufpd_v2di */
    case 1487:  /* sse2_shufpd_v2df */
    case 1486:  /* *avx_shufpd_v2di */
    case 1485:  /* *avx_shufpd_v2df */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (pat, 1), 0), 1));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 0));
      ro[4] = *(ro_loc[4] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 1));
      break;

    case 1720:  /* *vec_ext_v4si_mem */
    case 1713:  /* *sse4_1_pextrq */
    case 1712:  /* *sse4_1_pextrd */
    case 1711:  /* *sse4_1_pextrw_memory */
    case 1709:  /* *sse4_1_pextrb_memory */
    case 1471:  /* *vec_extract_v4sf_mem */
    case 1470:  /* *sse4_1_extractps */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 0));
      break;

    case 1456:  /* sse4_1_insertps */
    case 1455:  /* *avx_insertps */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (pat, 1), 0, 1));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (pat, 1), 0, 0));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (pat, 1), 0, 2));
      break;

    case 1718:  /* sse2_loadld */
    case 1717:  /* *avx_loadld */
    case 1452:  /* vec_setv4si_0 */
    case 1451:  /* vec_setv4sf_0 */
    case 1450:  /* *vec_setv4si_0_sse2 */
    case 1449:  /* *vec_setv4sf_0_sse2 */
    case 1448:  /* *vec_setv4si_0_sse4_1 */
    case 1447:  /* *vec_setv4sf_0_sse4_1 */
    case 1446:  /* *vec_setv4si_0_avx */
    case 1445:  /* *vec_setv4sf_0_avx */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 1));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      break;

    case 1499:  /* sse2_movsd */
    case 1498:  /* *avx_movsd */
    case 1437:  /* sse_movss */
    case 1436:  /* *avx_movss */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 1));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (pat, 1), 0));
      break;

    case 2023:  /* vec_set_lo_v32qi */
    case 2021:  /* vec_set_lo_v16hi */
    case 2018:  /* vec_set_lo_v8sf */
    case 2017:  /* vec_set_lo_v8si */
    case 2014:  /* vec_set_lo_v4df */
    case 2013:  /* vec_set_lo_v4di */
    case 1495:  /* sse2_loadlpd */
    case 1494:  /* *avx_loadlpd */
    case 1435:  /* sse_loadlps */
    case 1434:  /* *avx_loadlps */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (pat, 1), 0));
      break;

    case 1480:  /* avx_shufpd256_1 */
    case 1428:  /* sse_shufps_v4si */
    case 1427:  /* sse_shufps_v4sf */
    case 1426:  /* *avx_shufps_v4si */
    case 1425:  /* *avx_shufps_v4sf */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (pat, 1), 0), 1));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 0));
      ro[4] = *(ro_loc[4] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 1));
      ro[5] = *(ro_loc[5] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 2));
      ro[6] = *(ro_loc[6] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 3));
      break;

    case 1424:  /* avx_shufps256_1 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (pat, 1), 0), 1));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 0));
      ro[4] = *(ro_loc[4] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 1));
      ro[5] = *(ro_loc[5] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 2));
      ro[6] = *(ro_loc[6] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 3));
      ro[7] = *(ro_loc[7] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 4));
      ro[8] = *(ro_loc[8] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 5));
      ro[9] = *(ro_loc[9] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 6));
      ro[10] = *(ro_loc[10] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 7));
      break;

    case 1423:  /* sse3_movsldup */
    case 1422:  /* avx_movsldup256 */
    case 1421:  /* sse3_movshdup */
    case 1420:  /* avx_movshdup256 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (pat, 1), 0), 1);
      recog_data.dup_num[0] = 1;
      break;

    case 1362:  /* sse_cvtps2pi */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XEXP (pat, 1), 0), 0, 0));
      break;

    case 1405:  /* sse2_cvtss2sd */
    case 1404:  /* *avx_cvtss2sd */
    case 1403:  /* sse2_cvtsd2ss */
    case 1402:  /* *avx_cvtsd2ss */
    case 1389:  /* sse2_cvtsi2sdq */
    case 1388:  /* *avx_cvtsi2sdq */
    case 1387:  /* sse2_cvtsi2sd */
    case 1386:  /* *avx_cvtsi2sd */
    case 1367:  /* sse_cvtsi2ssq */
    case 1366:  /* *avx_cvtsi2ssq */
    case 1365:  /* sse_cvtsi2ss */
    case 1364:  /* *avx_cvtsi2ss */
    case 1361:  /* sse_cvtpi2ps */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 1));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0));
      break;

    case 1360:  /* fma4i_fmsubaddv2df4 */
    case 1359:  /* fma4i_fmsubaddv4sf4 */
    case 1358:  /* fma4i_fmsubaddv4df4 */
    case 1357:  /* fma4i_fmsubaddv8sf4 */
    case 1356:  /* fma4i_fmaddsubv2df4 */
    case 1355:  /* fma4i_fmaddsubv4sf4 */
    case 1354:  /* fma4i_fmaddsubv4df4 */
    case 1353:  /* fma4i_fmaddsubv8sf4 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 0), 0), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 0), 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 1), 0), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 1), 0), 1);
      recog_data.dup_num[1] = 2;
      recog_data.dup_loc[2] = &XEXP (XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 1), 1);
      recog_data.dup_num[2] = 3;
      break;

    case 1352:  /* fma4_fmsubaddv2df4 */
    case 1351:  /* fma4_fmsubaddv4sf4 */
    case 1350:  /* fma4_fmsubaddv4df4 */
    case 1349:  /* fma4_fmsubaddv8sf4 */
    case 1348:  /* fma4_fmaddsubv2df4 */
    case 1347:  /* fma4_fmaddsubv4sf4 */
    case 1346:  /* fma4_fmaddsubv4df4 */
    case 1345:  /* fma4_fmaddsubv8sf4 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XEXP (pat, 1), 0), 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 0), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 0), 1);
      recog_data.dup_num[1] = 2;
      recog_data.dup_loc[2] = &XEXP (XEXP (XEXP (pat, 1), 1), 1);
      recog_data.dup_num[2] = 3;
      break;

    case 1344:  /* fma4i_vmfnmsubv2df4 */
    case 1343:  /* fma4i_vmfnmsubv4sf4 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 0), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 0), 0), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 0), 1));
      recog_data.dup_loc[0] = &XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 1);
      recog_data.dup_num[0] = 0;
      break;

    case 1342:  /* fma4i_vmfnmaddv2df4 */
    case 1341:  /* fma4i_vmfnmaddv4sf4 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 0), 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 0), 1), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 0), 0));
      recog_data.dup_loc[0] = &XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 1);
      recog_data.dup_num[0] = 0;
      break;

    case 1340:  /* fma4i_vmfmsubv2df4 */
    case 1339:  /* fma4i_vmfmsubv4sf4 */
    case 1338:  /* fma4i_vmfmaddv2df4 */
    case 1337:  /* fma4i_vmfmaddv4sf4 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 0), 0), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 0), 1));
      recog_data.dup_loc[0] = &XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 1);
      recog_data.dup_num[0] = 0;
      break;

    case 1336:  /* fma4i_fnmsubv2df4 */
    case 1335:  /* fma4i_fnmsubv4sf4 */
    case 1328:  /* fma4i_fnmsubv4df4256 */
    case 1327:  /* fma4i_fnmsubv8sf4256 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 0), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 1));
      break;

    case 1334:  /* fma4i_fnmaddv2df4 */
    case 1333:  /* fma4i_fnmaddv4sf4 */
    case 1326:  /* fma4i_fnmaddv4df4256 */
    case 1325:  /* fma4i_fnmaddv8sf4256 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 1), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 0));
      break;

    case 1332:  /* fma4i_fmsubv2df4 */
    case 1331:  /* fma4i_fmsubv4sf4 */
    case 1330:  /* fma4i_fmaddv2df4 */
    case 1329:  /* fma4i_fmaddv4sf4 */
    case 1324:  /* fma4i_fmsubv4df4256 */
    case 1323:  /* fma4i_fmsubv8sf4256 */
    case 1322:  /* fma4i_fmaddv4df4256 */
    case 1321:  /* fma4i_fmaddv8sf4256 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 0), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 1));
      break;

    case 1320:  /* fma4_vmfnmsubv2df4 */
    case 1319:  /* fma4_vmfnmsubv4sf4 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XEXP (pat, 1), 0), 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (pat, 1), 1);
      recog_data.dup_num[0] = 0;
      break;

    case 1314:  /* fma4_vmfnmaddv2df4 */
    case 1313:  /* fma4_vmfnmaddv4sf4 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      recog_data.dup_loc[0] = &XEXP (XEXP (pat, 1), 1);
      recog_data.dup_num[0] = 0;
      break;

    case 1308:  /* fma4_vmfmsubv2df4 */
    case 1307:  /* fma4_vmfmsubv4sf4 */
    case 1302:  /* fma4_vmfmaddv2df4 */
    case 1301:  /* fma4_vmfmaddv4sf4 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XEXP (pat, 1), 0), 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (pat, 1), 1);
      recog_data.dup_num[0] = 0;
      break;

    case 1318:  /* fma4_fnmsubv2df4 */
    case 1317:  /* fma4_fnmsubv4sf4 */
    case 1316:  /* fma4_fnmsubdf4 */
    case 1315:  /* fma4_fnmsubsf4 */
    case 1296:  /* fma4_fnmsubv4df4256 */
    case 1295:  /* fma4_fnmsubv8sf4256 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (pat, 1), 0), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (pat, 1), 1));
      break;

    case 1312:  /* fma4_fnmaddv2df4 */
    case 1311:  /* fma4_fnmaddv4sf4 */
    case 1310:  /* fma4_fnmadddf4 */
    case 1309:  /* fma4_fnmaddsf4 */
    case 1294:  /* fma4_fnmaddv4df4256 */
    case 1293:  /* fma4_fnmaddv8sf4256 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (pat, 1), 1), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (pat, 1), 0));
      break;

    case 1248:  /* sse2_ucomi */
    case 1247:  /* sse_ucomi */
    case 1246:  /* sse2_comi */
    case 1245:  /* sse_comi */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 1), 0));
      break;

    case 1244:  /* sse2_vmmaskcmpv2df3 */
    case 1243:  /* sse_vmmaskcmpv4sf3 */
    case 1242:  /* *avx_vmmaskcmpv2df3 */
    case 1241:  /* *avx_vmmaskcmpv4sf3 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (pat, 1), 0), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (pat, 1), 0));
      recog_data.dup_loc[0] = &XEXP (XEXP (pat, 1), 1);
      recog_data.dup_num[0] = 1;
      break;

    case 1232:  /* avx_cmpsdv2df3 */
    case 1231:  /* avx_cmpssv4sf3 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XEXP (pat, 1), 0), 0, 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XEXP (pat, 1), 0), 0, 1));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XEXP (pat, 1), 0), 0, 2));
      recog_data.dup_loc[0] = &XEXP (XEXP (pat, 1), 1);
      recog_data.dup_num[0] = 1;
      break;

    case 1218:  /* avx_hsubv8sf3 */
    case 1217:  /* avx_haddv8sf3 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 0), 0), 0));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0), 1), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 1), 0), 0);
      recog_data.dup_num[1] = 1;
      recog_data.dup_loc[2] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 1), 1), 0);
      recog_data.dup_num[2] = 1;
      recog_data.dup_loc[3] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 0), 1), 0);
      recog_data.dup_num[3] = 2;
      recog_data.dup_loc[4] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 1), 0), 0);
      recog_data.dup_num[4] = 2;
      recog_data.dup_loc[5] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 1), 1), 0);
      recog_data.dup_num[5] = 2;
      recog_data.dup_loc[6] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 0), 0), 0), 0);
      recog_data.dup_num[6] = 1;
      recog_data.dup_loc[7] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 0), 0), 1), 0);
      recog_data.dup_num[7] = 1;
      recog_data.dup_loc[8] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 0), 1), 0), 0);
      recog_data.dup_num[8] = 1;
      recog_data.dup_loc[9] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 0), 1), 1), 0);
      recog_data.dup_num[9] = 1;
      recog_data.dup_loc[10] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 1), 0), 0), 0);
      recog_data.dup_num[10] = 2;
      recog_data.dup_loc[11] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 1), 0), 1), 0);
      recog_data.dup_num[11] = 2;
      recog_data.dup_loc[12] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 1), 1), 0), 0);
      recog_data.dup_num[12] = 2;
      recog_data.dup_loc[13] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 1), 1), 1), 0);
      recog_data.dup_num[13] = 2;
      break;

    case 1783:  /* ssse3_phsubswv4hi3 */
    case 1779:  /* ssse3_phsubdv4si3 */
    case 1778:  /* *avx_phsubdv4si3 */
    case 1777:  /* ssse3_phsubwv4hi3 */
    case 1774:  /* ssse3_phaddswv4hi3 */
    case 1770:  /* ssse3_phadddv4si3 */
    case 1769:  /* *avx_phadddv4si3 */
    case 1768:  /* ssse3_phaddwv4hi3 */
    case 1222:  /* sse3_hsubv4sf3 */
    case 1221:  /* sse3_haddv4sf3 */
    case 1220:  /* *avx_hsubv4sf3 */
    case 1219:  /* *avx_haddv4sf3 */
    case 1216:  /* avx_hsubv4df3 */
    case 1215:  /* avx_haddv4df3 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 0), 0), 0));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 1), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 0), 0);
      recog_data.dup_num[1] = 1;
      recog_data.dup_loc[2] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 1), 0);
      recog_data.dup_num[2] = 1;
      recog_data.dup_loc[3] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 0), 1), 0);
      recog_data.dup_num[3] = 2;
      recog_data.dup_loc[4] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 1), 0), 0);
      recog_data.dup_num[4] = 2;
      recog_data.dup_loc[5] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 1), 1), 0);
      recog_data.dup_num[5] = 2;
      break;

    case 1399:  /* *sse2_cvtpd2dq */
    case 1164:  /* sse_vmrsqrtv4sf2 */
    case 1163:  /* *avx_vmrsqrtv4sf2 */
    case 1152:  /* sse_vmrcpv4sf2 */
    case 1151:  /* *avx_vmrcpv4sf2 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XEXP (pat, 1), 0), 0, 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (pat, 1), 1));
      break;

    case 1196:  /* sse2_vmsminv2df3 */
    case 1195:  /* sse2_vmsmaxv2df3 */
    case 1194:  /* sse_vmsminv4sf3 */
    case 1193:  /* sse_vmsmaxv4sf3 */
    case 1192:  /* *avx_vmsminv2df3 */
    case 1191:  /* *avx_vmsmaxv2df3 */
    case 1190:  /* *avx_vmsminv4sf3 */
    case 1189:  /* *avx_vmsmaxv4sf3 */
    case 1148:  /* sse2_vmdivv2df3 */
    case 1147:  /* sse_vmdivv4sf3 */
    case 1146:  /* *avx_vmdivv2df3 */
    case 1145:  /* *avx_vmdivv4sf3 */
    case 1136:  /* sse2_vmmulv2df3 */
    case 1135:  /* sse_vmmulv4sf3 */
    case 1134:  /* *avx_vmmulv2df3 */
    case 1133:  /* *avx_vmmulv4sf3 */
    case 1126:  /* sse2_vmsubv2df3 */
    case 1125:  /* sse2_vmaddv2df3 */
    case 1124:  /* sse_vmsubv4sf3 */
    case 1123:  /* sse_vmaddv4sf3 */
    case 1122:  /* *avx_vmsubv2df3 */
    case 1121:  /* *avx_vmaddv2df3 */
    case 1120:  /* *avx_vmsubv4sf3 */
    case 1119:  /* *avx_vmaddv4sf3 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (pat, 1), 0), 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (pat, 1), 1);
      recog_data.dup_num[0] = 1;
      break;

    case 1985:  /* *avx_vzeroupper */
    case 1984:  /* *avx_vzeroall */
    case 1062:  /* *mmx_femms */
    case 1061:  /* *mmx_emms */
      ro[0] = *(ro_loc[0] = &PATTERN (insn));
      break;

    case 1756:  /* *sse2_maskmovdqu_rex64 */
    case 1755:  /* *sse2_maskmovdqu */
    case 1060:  /* *mmx_maskmovq_rex */
    case 1059:  /* *mmx_maskmovq */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (pat, 0), 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (pat, 1), 0, 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (pat, 1), 0, 1));
      recog_data.dup_loc[0] = &XEXP (XVECEXP (XEXP (pat, 1), 0, 2), 0);
      recog_data.dup_num[0] = 0;
      break;

    case 1715:  /* sse2_pshuflw_1 */
    case 1714:  /* sse2_pshufd_1 */
    case 1048:  /* mmx_pshufw_1 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 0));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 1));
      ro[4] = *(ro_loc[4] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 2));
      ro[5] = *(ro_loc[5] = &XVECEXP (XEXP (XEXP (pat, 1), 1), 0, 3));
      break;

    case 1710:  /* *sse2_pextrw */
    case 1708:  /* *sse4_1_pextrb */
    case 1047:  /* mmx_pextrw */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 0, 0));
      break;

    case 1707:  /* *sse4_1_pinsrq */
    case 1706:  /* *avx_pinsrq */
    case 1705:  /* *sse4_1_pinsrd */
    case 1704:  /* *sse2_pinsrw */
    case 1703:  /* *sse4_1_pinsrb */
    case 1702:  /* *avx_pinsrd */
    case 1701:  /* *avx_pinsrw */
    case 1700:  /* *avx_pinsrb */
    case 1454:  /* *vec_setv4sf_sse4_1 */
    case 1453:  /* *vec_setv4sf_avx */
    case 1046:  /* *mmx_pinsrw */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 1));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (pat, 1), 2));
      break;

    case 1699:  /* vec_interleave_lowv4si */
    case 1698:  /* *avx_interleave_lowv4si */
    case 1697:  /* vec_interleave_highv4si */
    case 1696:  /* *avx_interleave_highv4si */
    case 1695:  /* vec_interleave_lowv8hi */
    case 1694:  /* *avx_interleave_lowv8hi */
    case 1693:  /* vec_interleave_highv8hi */
    case 1692:  /* *avx_interleave_highv8hi */
    case 1691:  /* vec_interleave_lowv16qi */
    case 1690:  /* *avx_interleave_lowv16qi */
    case 1689:  /* vec_interleave_highv16qi */
    case 1688:  /* *avx_interleave_highv16qi */
    case 1484:  /* vec_interleave_lowv2di */
    case 1483:  /* *avx_interleave_lowv2di */
    case 1482:  /* vec_interleave_highv2di */
    case 1481:  /* *avx_interleave_highv2di */
    case 1479:  /* *sse2_interleave_lowv2df */
    case 1478:  /* *sse3_interleave_lowv2df */
    case 1477:  /* *avx_interleave_lowv2df */
    case 1476:  /* *avx_unpcklpd256 */
    case 1475:  /* *sse2_interleave_highv2df */
    case 1474:  /* *sse3_interleave_highv2df */
    case 1473:  /* *avx_interleave_highv2df */
    case 1472:  /* avx_unpckhpd256 */
    case 1419:  /* vec_interleave_lowv4sf */
    case 1418:  /* *avx_interleave_lowv4sf */
    case 1417:  /* avx_unpcklps256 */
    case 1416:  /* vec_interleave_highv4sf */
    case 1415:  /* *avx_interleave_highv4sf */
    case 1414:  /* avx_unpckhps256 */
    case 1413:  /* sse_movlhps */
    case 1412:  /* *avx_movlhps */
    case 1411:  /* sse_movhlps */
    case 1410:  /* *avx_movhlps */
    case 1045:  /* mmx_punpckldq */
    case 1044:  /* mmx_punpckhdq */
    case 1043:  /* mmx_punpcklwd */
    case 1042:  /* mmx_punpckhwd */
    case 1041:  /* mmx_punpcklbw */
    case 1040:  /* mmx_punpckhbw */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (pat, 1), 0), 1));
      break;

    case 1841:  /* sse4_1_packusdw */
    case 1840:  /* *avx_packusdw */
    case 1687:  /* sse2_packuswb */
    case 1686:  /* *avx_packuswb */
    case 1685:  /* sse2_packssdw */
    case 1684:  /* *avx_packssdw */
    case 1683:  /* sse2_packsswb */
    case 1682:  /* *avx_packsswb */
    case 1039:  /* mmx_packuswb */
    case 1038:  /* mmx_packssdw */
    case 1037:  /* mmx_packsswb */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (pat, 1), 1), 0));
      break;

    case 1890:  /* xop_mulv2div2di3_high */
    case 1888:  /* xop_mulv2div2di3_low */
    case 1547:  /* *sse4_1_mulv2siv2di3 */
    case 1546:  /* *avx_mulv2siv2di3 */
    case 1545:  /* *sse2_umulv2siv2di3 */
    case 1544:  /* *avx_umulv2siv2di3 */
    case 1006:  /* *sse2_umulv1siv1di3 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 0), 0));
      break;

    case 1747:  /* *sse2_uavgv8hi3 */
    case 1746:  /* *avx_uavgv8hi3 */
    case 1745:  /* *sse2_uavgv16qi3 */
    case 1744:  /* *avx_uavgv16qi3 */
    case 1056:  /* *mmx_uavgv4hi3 */
    case 1055:  /* *mmx_uavgv8qi3 */
    case 1005:  /* *mmx_pmulhrwv4hi3 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0), 1), 0));
      break;

    case 1786:  /* ssse3_pmaddubsw */
    case 1785:  /* ssse3_pmaddubsw128 */
    case 1784:  /* *avx_pmaddubsw128 */
    case 1549:  /* *sse2_pmaddwd */
    case 1548:  /* *avx_pmaddwd */
    case 1004:  /* *mmx_pmaddwd */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 0), 0));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 0), 0), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 1), 0), 0);
      recog_data.dup_num[1] = 2;
      break;

    case 1543:  /* *umulv8hi3_highpart */
    case 1542:  /* *avx_umulv8hi3_highpart */
    case 1541:  /* *smulv8hi3_highpart */
    case 1540:  /* *avxv8hi3_highpart */
    case 1003:  /* *mmx_umulv4hi3_highpart */
    case 1002:  /* *mmx_smulv4hi3_highpart */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 1), 0));
      break;

    case 978:  /* mmx_pi2fw */
    case 977:  /* mmx_pf2iw */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0));
      break;

    case 1214:  /* sse3_addsubv2df3 */
    case 1213:  /* *avx_addsubv2df3 */
    case 1212:  /* sse3_addsubv4sf3 */
    case 1211:  /* *avx_addsubv4sf3 */
    case 1210:  /* avx_addsubv4df3 */
    case 1209:  /* avx_addsubv8sf3 */
    case 972:  /* mmx_addsubv2sf3 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (pat, 1), 0), 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (pat, 1), 1), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 1), 1);
      recog_data.dup_num[1] = 2;
      break;

    case 1780:  /* ssse3_phsubdv2si3 */
    case 1771:  /* ssse3_phadddv2si3 */
    case 1226:  /* sse3_hsubv2df3 */
    case 1225:  /* sse3_haddv2df3 */
    case 1224:  /* *avx_hsubv2df3 */
    case 1223:  /* *avx_haddv2df3 */
    case 971:  /* mmx_hsubv2sf3 */
    case 970:  /* mmx_haddv2sf3 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 0), 0));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 1), 0);
      recog_data.dup_num[1] = 2;
      break;

    case 940:  /* *lwp_lwpinsdi3_1 */
    case 939:  /* *lwp_lwpinssi3_1 */
      ro[0] = *(ro_loc[0] = &XVECEXP (XEXP (pat, 1), 0, 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (pat, 1), 0, 1));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (pat, 1), 0, 2));
      break;

    case 1765:  /* sse3_monitor64 */
    case 1764:  /* sse3_monitor */
    case 938:  /* *lwp_lwpvaldi3_1 */
    case 937:  /* *lwp_lwpvalsi3_1 */
      ro[0] = *(ro_loc[0] = &XVECEXP (pat, 0, 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (pat, 0, 1));
      ro[2] = *(ro_loc[2] = &XVECEXP (pat, 0, 2));
      break;

    case 932:  /* *rdtscp_rex64 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 2), 0));
      break;

    case 931:  /* *rdtscp */
    case 930:  /* *rdtsc_rex64 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 1), 0));
      break;

    case 928:  /* *rdpmc_rex64 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      break;

    case 912:  /* *prefetch_sse_rex */
    case 911:  /* *prefetch_sse */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (pat, 2));
      break;

    case 910:  /* *sse_prologue_save_insn */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 0), 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 2), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XVECEXP (pat, 0, 3), 0), 0));
      ro[4] = *(ro_loc[4] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 0), 0), 1));
      break;

    case 2024:  /* vec_set_hi_v32qi */
    case 2022:  /* vec_set_hi_v16hi */
    case 2020:  /* vec_set_hi_v8sf */
    case 2019:  /* vec_set_hi_v8si */
    case 2016:  /* vec_set_hi_v4df */
    case 2015:  /* vec_set_hi_v4di */
    case 1630:  /* *andnottf3 */
    case 1629:  /* sse2_andnotv2di3 */
    case 1628:  /* sse2_andnotv4si3 */
    case 1627:  /* sse2_andnotv8hi3 */
    case 1626:  /* sse2_andnotv16qi3 */
    case 1625:  /* *avx_andnotv2di3 */
    case 1624:  /* *avx_andnotv4si3 */
    case 1623:  /* *avx_andnotv8hi3 */
    case 1622:  /* *avx_andnotv16qi3 */
    case 1621:  /* *sse_andnotv2di3 */
    case 1620:  /* *sse_andnotv4si3 */
    case 1619:  /* *sse_andnotv8hi3 */
    case 1618:  /* *sse_andnotv16qi3 */
    case 1617:  /* *avx_andnotv4di3 */
    case 1616:  /* *avx_andnotv8si3 */
    case 1615:  /* *avx_andnotv16hi3 */
    case 1614:  /* *avx_andnotv32qi3 */
    case 1493:  /* sse2_loadhpd */
    case 1492:  /* *avx_loadhpd */
    case 1431:  /* sse_loadhps */
    case 1430:  /* *avx_loadhps */
    case 1407:  /* *sse2_cvtpd2ps */
    case 1401:  /* *sse2_cvttpd2dq */
    case 1276:  /* *andnotdf3 */
    case 1275:  /* *andnotsf3 */
    case 1274:  /* *avx_andnotdf3 */
    case 1273:  /* *avx_andnotsf3 */
    case 1254:  /* sse2_andnotv2df3 */
    case 1253:  /* sse_andnotv4sf3 */
    case 1252:  /* avx_andnotv4df3 */
    case 1251:  /* avx_andnotv8sf3 */
    case 1250:  /* avx_andnotv2df3 */
    case 1249:  /* avx_andnotv4sf3 */
    case 1160:  /* sse2_vmsqrtv2df2 */
    case 1159:  /* sse_vmsqrtv4sf2 */
    case 1158:  /* *avx_vmsqrtv2df2 */
    case 1157:  /* *avx_vmsqrtv4sf2 */
    case 1027:  /* mmx_andnotv2si3 */
    case 1026:  /* mmx_andnotv4hi3 */
    case 1025:  /* mmx_andnotv8qi3 */
    case 908:  /* *sibcall_value_1_rex64 */
    case 907:  /* *call_value_1_rex64_large */
    case 905:  /* *call_value_1_rex64 */
    case 904:  /* *sibcall_value_1 */
    case 903:  /* *call_value_1 */
    case 901:  /* *call_value_0_rex64 */
    case 900:  /* *call_value_0 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (pat, 1), 1));
      break;

    case 899:  /* *sibcall_value_pop_1 */
    case 898:  /* *call_value_pop_1 */
    case 897:  /* *call_value_pop_0 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 1));
      break;

    case 896:  /* allocate_stack_worker_64 */
    case 895:  /* allocate_stack_worker_32 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      recog_data.dup_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 1);
      recog_data.dup_num[0] = 1;
      break;

    case 894:  /* pro_epilogue_adjust_stack_rex64_2 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1));
      break;

    case 2044:  /* *vec_concatv4df_avx */
    case 2043:  /* *vec_concatv8sf_avx */
    case 2042:  /* *vec_concatv4di_avx */
    case 2041:  /* *vec_concatv8si_avx */
    case 2040:  /* *vec_concatv16hi_avx */
    case 2039:  /* *vec_concatv32qi_avx */
    case 1933:  /* xop_rotrv2di3 */
    case 1932:  /* xop_rotrv4si3 */
    case 1931:  /* xop_rotrv8hi3 */
    case 1930:  /* xop_rotrv16qi3 */
    case 1929:  /* xop_rotlv2di3 */
    case 1928:  /* xop_rotlv4si3 */
    case 1927:  /* xop_rotlv8hi3 */
    case 1926:  /* xop_rotlv16qi3 */
    case 1743:  /* *vec_concatv2di_rex64_sse */
    case 1742:  /* *vec_concatv2di_rex64_sse4_1 */
    case 1741:  /* *vec_concatv2di_rex64_avx */
    case 1740:  /* vec_concatv2di */
    case 1739:  /* *vec_concatv2di_avx */
    case 1738:  /* *vec_concatv4si_1 */
    case 1737:  /* *vec_concatv4si_1_avx */
    case 1736:  /* *vec_concatv2si_sse */
    case 1735:  /* *vec_concatv2si_sse2 */
    case 1734:  /* *vec_concatv2si_sse4_1 */
    case 1733:  /* *vec_concatv2si_avx */
    case 1681:  /* *xortf3 */
    case 1680:  /* *iortf3 */
    case 1679:  /* *andtf3 */
    case 1678:  /* *sse2_xorv2di3 */
    case 1677:  /* *sse2_iorv2di3 */
    case 1676:  /* *sse2_andv2di3 */
    case 1675:  /* *sse2_xorv4si3 */
    case 1674:  /* *sse2_iorv4si3 */
    case 1673:  /* *sse2_andv4si3 */
    case 1672:  /* *sse2_xorv8hi3 */
    case 1671:  /* *sse2_iorv8hi3 */
    case 1670:  /* *sse2_andv8hi3 */
    case 1669:  /* *sse2_xorv16qi3 */
    case 1668:  /* *sse2_iorv16qi3 */
    case 1667:  /* *sse2_andv16qi3 */
    case 1666:  /* *avx_xorv2di3 */
    case 1665:  /* *avx_iorv2di3 */
    case 1664:  /* *avx_andv2di3 */
    case 1663:  /* *avx_xorv4si3 */
    case 1662:  /* *avx_iorv4si3 */
    case 1661:  /* *avx_andv4si3 */
    case 1660:  /* *avx_xorv8hi3 */
    case 1659:  /* *avx_iorv8hi3 */
    case 1658:  /* *avx_andv8hi3 */
    case 1657:  /* *avx_xorv16qi3 */
    case 1656:  /* *avx_iorv16qi3 */
    case 1655:  /* *avx_andv16qi3 */
    case 1654:  /* *sse_xorv2di3 */
    case 1653:  /* *sse_iorv2di3 */
    case 1652:  /* *sse_andv2di3 */
    case 1651:  /* *sse_xorv4si3 */
    case 1650:  /* *sse_iorv4si3 */
    case 1649:  /* *sse_andv4si3 */
    case 1648:  /* *sse_xorv8hi3 */
    case 1647:  /* *sse_iorv8hi3 */
    case 1646:  /* *sse_andv8hi3 */
    case 1645:  /* *sse_xorv16qi3 */
    case 1644:  /* *sse_iorv16qi3 */
    case 1643:  /* *sse_andv16qi3 */
    case 1642:  /* *avx_xorv4di3 */
    case 1641:  /* *avx_iorv4di3 */
    case 1640:  /* *avx_andv4di3 */
    case 1639:  /* *avx_xorv8si3 */
    case 1638:  /* *avx_iorv8si3 */
    case 1637:  /* *avx_andv8si3 */
    case 1636:  /* *avx_xorv16hi3 */
    case 1635:  /* *avx_iorv16hi3 */
    case 1634:  /* *avx_andv16hi3 */
    case 1633:  /* *avx_xorv32qi3 */
    case 1632:  /* *avx_iorv32qi3 */
    case 1631:  /* *avx_andv32qi3 */
    case 1613:  /* sse4_2_gtv2di3 */
    case 1612:  /* sse2_gtv4si3 */
    case 1611:  /* sse2_gtv8hi3 */
    case 1610:  /* sse2_gtv16qi3 */
    case 1609:  /* *avx_gtv2di3 */
    case 1608:  /* *avx_gtv4si3 */
    case 1607:  /* *avx_gtv8hi3 */
    case 1606:  /* *avx_gtv16qi3 */
    case 1605:  /* *sse4_1_eqv2di3 */
    case 1604:  /* *sse2_eqv4si3 */
    case 1603:  /* *sse2_eqv8hi3 */
    case 1602:  /* *sse2_eqv16qi3 */
    case 1601:  /* *avx_eqv2di3 */
    case 1600:  /* *avx_eqv4si3 */
    case 1599:  /* *avx_eqv8hi3 */
    case 1598:  /* *avx_eqv16qi3 */
    case 1597:  /* *sse4_1_uminv4si3 */
    case 1596:  /* *sse4_1_umaxv4si3 */
    case 1595:  /* *sse4_1_uminv8hi3 */
    case 1594:  /* *sse4_1_umaxv8hi3 */
    case 1593:  /* *sse4_1_sminv4si3 */
    case 1592:  /* *sse4_1_smaxv4si3 */
    case 1591:  /* *sse4_1_sminv16qi3 */
    case 1590:  /* *sse4_1_smaxv16qi3 */
    case 1589:  /* *sminv8hi3 */
    case 1588:  /* *smaxv8hi3 */
    case 1587:  /* *uminv16qi3 */
    case 1586:  /* *umaxv16qi3 */
    case 1585:  /* *avx_uminv4si3 */
    case 1584:  /* *avx_umaxv4si3 */
    case 1583:  /* *avx_sminv4si3 */
    case 1582:  /* *avx_smaxv4si3 */
    case 1581:  /* *avx_uminv8hi3 */
    case 1580:  /* *avx_umaxv8hi3 */
    case 1579:  /* *avx_sminv8hi3 */
    case 1578:  /* *avx_smaxv8hi3 */
    case 1577:  /* *avx_uminv16qi3 */
    case 1576:  /* *avx_umaxv16qi3 */
    case 1575:  /* *avx_sminv16qi3 */
    case 1574:  /* *avx_smaxv16qi3 */
    case 1573:  /* ashlv2di3 */
    case 1572:  /* ashlv4si3 */
    case 1571:  /* ashlv8hi3 */
    case 1570:  /* sse2_ashlv1ti3 */
    case 1569:  /* *avx_ashlv2di3 */
    case 1568:  /* *avx_ashlv4si3 */
    case 1567:  /* *avx_ashlv8hi3 */
    case 1566:  /* *avx_ashlv1ti3 */
    case 1565:  /* lshrv2di3 */
    case 1564:  /* lshrv4si3 */
    case 1563:  /* lshrv8hi3 */
    case 1562:  /* sse2_lshrv1ti3 */
    case 1561:  /* *avx_lshrv2di3 */
    case 1560:  /* *avx_lshrv4si3 */
    case 1559:  /* *avx_lshrv8hi3 */
    case 1558:  /* *avx_lshrv1ti3 */
    case 1557:  /* ashrv4si3 */
    case 1556:  /* ashrv8hi3 */
    case 1555:  /* *avx_ashrv4si3 */
    case 1554:  /* *avx_ashrv8hi3 */
    case 1553:  /* mulv2di3 */
    case 1552:  /* *sse2_mulv4si3 */
    case 1551:  /* *sse4_1_mulv4si3 */
    case 1550:  /* *avx_mulv4si3 */
    case 1539:  /* *mulv8hi3 */
    case 1538:  /* *avx_mulv8hi3 */
    case 1537:  /* mulv16qi3 */
    case 1536:  /* *sse2_ussubv8hi3 */
    case 1535:  /* *sse2_sssubv8hi3 */
    case 1534:  /* *sse2_usaddv8hi3 */
    case 1533:  /* *sse2_ssaddv8hi3 */
    case 1532:  /* *sse2_ussubv16qi3 */
    case 1531:  /* *sse2_sssubv16qi3 */
    case 1530:  /* *sse2_usaddv16qi3 */
    case 1529:  /* *sse2_ssaddv16qi3 */
    case 1528:  /* *avx_ussubv8hi3 */
    case 1527:  /* *avx_sssubv8hi3 */
    case 1526:  /* *avx_usaddv8hi3 */
    case 1525:  /* *avx_ssaddv8hi3 */
    case 1524:  /* *avx_ussubv16qi3 */
    case 1523:  /* *avx_sssubv16qi3 */
    case 1522:  /* *avx_usaddv16qi3 */
    case 1521:  /* *avx_ssaddv16qi3 */
    case 1520:  /* *subv2di3 */
    case 1519:  /* *addv2di3 */
    case 1518:  /* *subv4si3 */
    case 1517:  /* *addv4si3 */
    case 1516:  /* *subv8hi3 */
    case 1515:  /* *addv8hi3 */
    case 1514:  /* *subv16qi3 */
    case 1513:  /* *addv16qi3 */
    case 1512:  /* *avx_subv2di3 */
    case 1511:  /* *avx_addv2di3 */
    case 1510:  /* *avx_subv4si3 */
    case 1509:  /* *avx_addv4si3 */
    case 1508:  /* *avx_subv8hi3 */
    case 1507:  /* *avx_addv8hi3 */
    case 1506:  /* *avx_subv16qi3 */
    case 1505:  /* *avx_addv16qi3 */
    case 1504:  /* *vec_concatv2df */
    case 1503:  /* *vec_concatv2df_avx */
    case 1444:  /* *vec_concatv4sf_sse */
    case 1443:  /* *vec_concatv4sf_avx */
    case 1442:  /* *vec_concatv2sf_sse */
    case 1441:  /* *vec_concatv2sf_sse4_1 */
    case 1440:  /* *vec_concatv2sf_avx */
    case 1288:  /* *xordf3 */
    case 1287:  /* *iordf3 */
    case 1286:  /* *anddf3 */
    case 1285:  /* *xorsf3 */
    case 1284:  /* *iorsf3 */
    case 1283:  /* *andsf3 */
    case 1282:  /* *avx_xordf3 */
    case 1281:  /* *avx_iordf3 */
    case 1280:  /* *avx_anddf3 */
    case 1279:  /* *avx_xorsf3 */
    case 1278:  /* *avx_iorsf3 */
    case 1277:  /* *avx_andsf3 */
    case 1272:  /* *xorv2df3 */
    case 1271:  /* *iorv2df3 */
    case 1270:  /* *andv2df3 */
    case 1269:  /* *xorv4sf3 */
    case 1268:  /* *iorv4sf3 */
    case 1267:  /* *andv4sf3 */
    case 1266:  /* *avx_xorv4df3 */
    case 1265:  /* *avx_iorv4df3 */
    case 1264:  /* *avx_andv4df3 */
    case 1263:  /* *avx_xorv8sf3 */
    case 1262:  /* *avx_iorv8sf3 */
    case 1261:  /* *avx_andv8sf3 */
    case 1260:  /* *avx_xorv2df3 */
    case 1259:  /* *avx_iorv2df3 */
    case 1258:  /* *avx_andv2df3 */
    case 1257:  /* *avx_xorv4sf3 */
    case 1256:  /* *avx_iorv4sf3 */
    case 1255:  /* *avx_andv4sf3 */
    case 1188:  /* *sminv2df3 */
    case 1187:  /* *smaxv2df3 */
    case 1186:  /* *sminv4sf3 */
    case 1185:  /* *smaxv4sf3 */
    case 1184:  /* *avx_sminv4df3 */
    case 1183:  /* *avx_smaxv4df3 */
    case 1182:  /* *avx_sminv8sf3 */
    case 1181:  /* *avx_smaxv8sf3 */
    case 1180:  /* *avx_sminv2df3 */
    case 1179:  /* *avx_smaxv2df3 */
    case 1178:  /* *avx_sminv4sf3 */
    case 1177:  /* *avx_smaxv4sf3 */
    case 1176:  /* *sminv2df3_finite */
    case 1175:  /* *smaxv2df3_finite */
    case 1174:  /* *sminv4sf3_finite */
    case 1173:  /* *smaxv4sf3_finite */
    case 1172:  /* *avx_sminv4df3_finite */
    case 1171:  /* *avx_smaxv4df3_finite */
    case 1170:  /* *avx_sminv8sf3_finite */
    case 1169:  /* *avx_smaxv8sf3_finite */
    case 1168:  /* *avx_sminv2df3_finite */
    case 1167:  /* *avx_smaxv2df3_finite */
    case 1166:  /* *avx_sminv4sf3_finite */
    case 1165:  /* *avx_smaxv4sf3_finite */
    case 1144:  /* sse2_divv2df3 */
    case 1143:  /* sse_divv4sf3 */
    case 1142:  /* *avx_divv2df3 */
    case 1141:  /* *avx_divv4sf3 */
    case 1140:  /* avx_divv4df3 */
    case 1139:  /* avx_divv8sf3 */
    case 1138:  /* avx_divv2df3 */
    case 1137:  /* avx_divv4sf3 */
    case 1132:  /* *mulv2df3 */
    case 1131:  /* *mulv4sf3 */
    case 1130:  /* *avx_mulv4df3 */
    case 1129:  /* *avx_mulv8sf3 */
    case 1128:  /* *avx_mulv2df3 */
    case 1127:  /* *avx_mulv4sf3 */
    case 1118:  /* *subv2df3 */
    case 1117:  /* *addv2df3 */
    case 1116:  /* *subv4sf3 */
    case 1115:  /* *addv4sf3 */
    case 1114:  /* *avx_subv4df3 */
    case 1113:  /* *avx_addv4df3 */
    case 1112:  /* *avx_subv8sf3 */
    case 1111:  /* *avx_addv8sf3 */
    case 1110:  /* *avx_subv2df3 */
    case 1109:  /* *avx_addv2df3 */
    case 1108:  /* *avx_subv4sf3 */
    case 1107:  /* *avx_addv4sf3 */
    case 1052:  /* *mmx_concatv2si */
    case 1036:  /* *mmx_xorv2si3 */
    case 1035:  /* *mmx_iorv2si3 */
    case 1034:  /* *mmx_andv2si3 */
    case 1033:  /* *mmx_xorv4hi3 */
    case 1032:  /* *mmx_iorv4hi3 */
    case 1031:  /* *mmx_andv4hi3 */
    case 1030:  /* *mmx_xorv8qi3 */
    case 1029:  /* *mmx_iorv8qi3 */
    case 1028:  /* *mmx_andv8qi3 */
    case 1024:  /* mmx_gtv2si3 */
    case 1023:  /* mmx_gtv4hi3 */
    case 1022:  /* mmx_gtv8qi3 */
    case 1021:  /* *mmx_eqv2si3 */
    case 1020:  /* *mmx_eqv4hi3 */
    case 1019:  /* *mmx_eqv8qi3 */
    case 1018:  /* mmx_ashlv1di3 */
    case 1017:  /* mmx_ashlv2si3 */
    case 1016:  /* mmx_ashlv4hi3 */
    case 1015:  /* mmx_lshrv1di3 */
    case 1014:  /* mmx_lshrv2si3 */
    case 1013:  /* mmx_lshrv4hi3 */
    case 1012:  /* mmx_ashrv2si3 */
    case 1011:  /* mmx_ashrv4hi3 */
    case 1010:  /* *mmx_uminv8qi3 */
    case 1009:  /* *mmx_umaxv8qi3 */
    case 1008:  /* *mmx_sminv4hi3 */
    case 1007:  /* *mmx_smaxv4hi3 */
    case 1001:  /* *mmx_mulv4hi3 */
    case 1000:  /* *mmx_ussubv4hi3 */
    case 999:  /* *mmx_sssubv4hi3 */
    case 998:  /* *mmx_usaddv4hi3 */
    case 997:  /* *mmx_ssaddv4hi3 */
    case 996:  /* *mmx_ussubv8qi3 */
    case 995:  /* *mmx_sssubv8qi3 */
    case 994:  /* *mmx_usaddv8qi3 */
    case 993:  /* *mmx_ssaddv8qi3 */
    case 992:  /* *mmx_subv1di3 */
    case 991:  /* *mmx_addv1di3 */
    case 990:  /* *mmx_subv2si3 */
    case 989:  /* *mmx_addv2si3 */
    case 988:  /* *mmx_subv4hi3 */
    case 987:  /* *mmx_addv4hi3 */
    case 986:  /* *mmx_subv8qi3 */
    case 985:  /* *mmx_addv8qi3 */
    case 982:  /* *mmx_concatv2sf */
    case 975:  /* mmx_gev2sf3 */
    case 974:  /* mmx_gtv2sf3 */
    case 973:  /* *mmx_eqv2sf3 */
    case 964:  /* *mmx_sminv2sf3 */
    case 963:  /* *mmx_smaxv2sf3 */
    case 962:  /* *mmx_sminv2sf3_finite */
    case 961:  /* *mmx_smaxv2sf3_finite */
    case 960:  /* *mmx_mulv2sf3 */
    case 959:  /* *mmx_subv2sf3 */
    case 958:  /* *mmx_addv2sf3 */
    case 883:  /* smindf3 */
    case 882:  /* smaxdf3 */
    case 881:  /* sminsf3 */
    case 880:  /* smaxsf3 */
    case 879:  /* *avx_smindf3 */
    case 878:  /* *avx_smaxdf3 */
    case 877:  /* *avx_sminsf3 */
    case 876:  /* *avx_smaxsf3 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (pat, 1), 1));
      break;

    case 869:  /* *movqicc_noc */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (pat, 1), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (pat, 1), 2));
      ro[4] = *(ro_loc[4] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      break;

    case 875:  /* *xop_pcmov_df */
    case 874:  /* *xop_pcmov_sf */
    case 873:  /* *movxfcc_1 */
    case 872:  /* *movdfcc_1_rex64 */
    case 871:  /* *movdfcc_1 */
    case 870:  /* *movsfcc_1_387 */
    case 868:  /* *movdicc_noc */
    case 867:  /* *movsicc_noc */
    case 866:  /* *movhicc_noc */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (pat, 1), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (pat, 1), 2));
      break;

    case 859:  /* *strlenqi_rex_1 */
    case 858:  /* *strlenqi_1 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 1));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 2));
      ro[4] = *(ro_loc[4] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 3));
      ro[5] = *(ro_loc[5] = &XEXP (XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0), 0));
      break;

    case 857:  /* *cmpstrnqi_rex_1 */
    case 856:  /* *cmpstrnqi_1 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 3), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 4), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 5), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[4] = *(ro_loc[4] = &XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1), 0), 0));
      ro[5] = *(ro_loc[5] = &XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1), 1), 0));
      ro[6] = *(ro_loc[6] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      break;

    case 855:  /* *cmpstrnqi_nz_rex_1 */
    case 854:  /* *cmpstrnqi_nz_1 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 3), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 4), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 5), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (pat, 0, 2), 0));
      ro[4] = *(ro_loc[4] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      ro[5] = *(ro_loc[5] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1), 0));
      ro[6] = *(ro_loc[6] = &XEXP (XVECEXP (pat, 0, 1), 0));
      break;

    case 853:  /* *rep_stosqi_rex64 */
    case 852:  /* *rep_stosqi */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 3), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0));
      ro[4] = *(ro_loc[4] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 1));
      recog_data.dup_loc[0] = &XEXP (XVECEXP (pat, 0, 4), 0);
      recog_data.dup_num[0] = 4;
      recog_data.dup_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 2), 0), 0);
      recog_data.dup_num[1] = 3;
      break;

    case 851:  /* *rep_stossi_rex64 */
    case 850:  /* *rep_stossi */
    case 849:  /* *rep_stosdi_rex64 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 3), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 1));
      ro[4] = *(ro_loc[4] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0), 0));
      recog_data.dup_loc[0] = &XEXP (XVECEXP (pat, 0, 4), 0);
      recog_data.dup_num[0] = 4;
      recog_data.dup_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 2), 0), 0);
      recog_data.dup_num[1] = 3;
      break;

    case 848:  /* *strsetqi_rex_1 */
    case 847:  /* *strsetqi_1 */
    case 846:  /* *strsethi_rex_1 */
    case 845:  /* *strsethi_1 */
    case 844:  /* *strsetsi_rex_1 */
    case 843:  /* *strsetsi_1 */
    case 842:  /* *strsetdi_rex_1 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 0), 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0);
      recog_data.dup_num[0] = 1;
      break;

    case 841:  /* *rep_movqi_rex64 */
    case 840:  /* *rep_movqi */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 2), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0));
      ro[4] = *(ro_loc[4] = &XEXP (XEXP (XVECEXP (pat, 0, 2), 1), 0));
      ro[5] = *(ro_loc[5] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 1));
      recog_data.dup_loc[0] = &XEXP (XVECEXP (pat, 0, 4), 0);
      recog_data.dup_num[0] = 5;
      recog_data.dup_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 3), 0), 0);
      recog_data.dup_num[1] = 3;
      recog_data.dup_loc[2] = &XEXP (XEXP (XVECEXP (pat, 0, 3), 1), 0);
      recog_data.dup_num[2] = 4;
      recog_data.dup_loc[3] = &XEXP (XEXP (XVECEXP (pat, 0, 2), 1), 1);
      recog_data.dup_num[3] = 5;
      break;

    case 839:  /* *rep_movsi_rex64 */
    case 838:  /* *rep_movsi */
    case 837:  /* *rep_movdi_rex64 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 2), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 1));
      ro[4] = *(ro_loc[4] = &XEXP (XEXP (XVECEXP (pat, 0, 2), 1), 1));
      ro[5] = *(ro_loc[5] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0), 0));
      recog_data.dup_loc[0] = &XEXP (XVECEXP (pat, 0, 4), 0);
      recog_data.dup_num[0] = 5;
      recog_data.dup_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 3), 0), 0);
      recog_data.dup_num[1] = 3;
      recog_data.dup_loc[2] = &XEXP (XEXP (XVECEXP (pat, 0, 3), 1), 0);
      recog_data.dup_num[2] = 4;
      recog_data.dup_loc[3] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 2), 1), 0), 0);
      recog_data.dup_num[3] = 5;
      break;

    case 836:  /* *strmovqi_rex_1 */
    case 835:  /* *strmovqi_1 */
    case 834:  /* *strmovhi_rex_1 */
    case 833:  /* *strmovhi_1 */
    case 832:  /* *strmovsi_rex_1 */
    case 831:  /* *strmovsi_1 */
    case 830:  /* *strmovdi_rex_1 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 2), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 0), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      recog_data.dup_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 2), 1), 0);
      recog_data.dup_num[0] = 3;
      recog_data.dup_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0);
      recog_data.dup_num[1] = 2;
      break;

    case 815:  /* fistdi2_ceil_with_temp */
    case 804:  /* fistdi2_floor_with_temp */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (pat, 0, 2), 0));
      ro[4] = *(ro_loc[4] = &XEXP (XVECEXP (pat, 0, 3), 0));
      ro[5] = *(ro_loc[5] = &XEXP (XVECEXP (pat, 0, 4), 0));
      break;

    case 819:  /* fistsi2_ceil_with_temp */
    case 818:  /* fisthi2_ceil_with_temp */
    case 814:  /* fistdi2_ceil */
    case 808:  /* fistsi2_floor_with_temp */
    case 807:  /* fisthi2_floor_with_temp */
    case 803:  /* fistdi2_floor */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (pat, 0, 2), 0));
      ro[4] = *(ro_loc[4] = &XEXP (XVECEXP (pat, 0, 3), 0));
      break;

    case 822:  /* frndintxf2_mask_pm */
    case 820:  /* frndintxf2_trunc */
    case 813:  /* *fistdi2_ceil_1 */
    case 812:  /* *fistsi2_ceil_1 */
    case 811:  /* *fisthi2_ceil_1 */
    case 809:  /* frndintxf2_ceil */
    case 802:  /* *fistdi2_floor_1 */
    case 801:  /* *fistsi2_floor_1 */
    case 800:  /* *fisthi2_floor_1 */
    case 798:  /* frndintxf2_floor */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      break;

    case 823:  /* frndintxf2_mask_pm_i387 */
    case 821:  /* frndintxf2_trunc_i387 */
    case 817:  /* fistsi2_ceil */
    case 816:  /* fisthi2_ceil */
    case 810:  /* frndintxf2_ceil_i387 */
    case 806:  /* fistsi2_floor */
    case 805:  /* fisthi2_floor */
    case 799:  /* frndintxf2_floor_i387 */
    case 791:  /* fistdi2_with_temp */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (pat, 0, 2), 0));
      break;

    case 918:  /* stack_tls_protect_set_di */
    case 917:  /* stack_tls_protect_set_si */
    case 916:  /* stack_protect_set_di */
    case 915:  /* stack_protect_set_si */
    case 797:  /* fistsi2_with_temp */
    case 796:  /* fisthi2_with_temp */
    case 790:  /* fistdi2 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 1), 0));
      break;

    case 2006:  /* avx_vpermilvarv4df3 */
    case 2005:  /* avx_vpermilvarv8sf3 */
    case 2004:  /* avx_vpermilvarv2df3 */
    case 2003:  /* avx_vpermilvarv4sf3 */
    case 1981:  /* aeskeygenassist */
    case 1979:  /* aesdeclast */
    case 1978:  /* *avx_aesdeclast */
    case 1977:  /* aesdec */
    case 1976:  /* *avx_aesdec */
    case 1975:  /* aesenclast */
    case 1974:  /* *avx_aesenclast */
    case 1973:  /* aesenc */
    case 1972:  /* *avx_aesenc */
    case 1868:  /* sse4_1_roundpd */
    case 1867:  /* sse4_1_roundps */
    case 1866:  /* avx_roundpd256 */
    case 1865:  /* avx_roundps256 */
    case 1818:  /* sse4a_insertq */
    case 1816:  /* sse4a_extrq */
    case 1801:  /* ssse3_psignv2si3 */
    case 1800:  /* ssse3_psignv4hi3 */
    case 1799:  /* ssse3_psignv8qi3 */
    case 1798:  /* ssse3_psignv4si3 */
    case 1797:  /* ssse3_psignv8hi3 */
    case 1796:  /* ssse3_psignv16qi3 */
    case 1795:  /* *avx_psignv4si3 */
    case 1794:  /* *avx_psignv8hi3 */
    case 1793:  /* *avx_psignv16qi3 */
    case 1792:  /* ssse3_pshufbv8qi3 */
    case 1791:  /* ssse3_pshufbv16qi3 */
    case 1790:  /* *avx_pshufbv16qi3 */
    case 1749:  /* sse2_psadbw */
    case 1748:  /* *avx_psadbw */
    case 1208:  /* *ieee_smaxv2df3 */
    case 1207:  /* *ieee_smaxv4sf3 */
    case 1206:  /* *ieee_sminv2df3 */
    case 1205:  /* *ieee_sminv4sf3 */
    case 1204:  /* *avx_ieee_smaxv4df3 */
    case 1203:  /* *avx_ieee_smaxv8sf3 */
    case 1202:  /* *avx_ieee_smaxv2df3 */
    case 1201:  /* *avx_ieee_smaxv4sf3 */
    case 1200:  /* *avx_ieee_sminv4df3 */
    case 1199:  /* *avx_ieee_sminv8sf3 */
    case 1198:  /* *avx_ieee_sminv2df3 */
    case 1197:  /* *avx_ieee_sminv4sf3 */
    case 1057:  /* mmx_psadbw */
    case 969:  /* mmx_rsqit1v2sf3 */
    case 967:  /* mmx_rcpit2v2sf3 */
    case 966:  /* mmx_rcpit1v2sf3 */
    case 926:  /* sse4_2_crc32di */
    case 925:  /* sse4_2_crc32si */
    case 924:  /* sse4_2_crc32hi */
    case 923:  /* sse4_2_crc32qi */
    case 891:  /* *ieee_smaxdf3 */
    case 890:  /* *ieee_smaxsf3 */
    case 889:  /* *avx_ieee_smaxdf3 */
    case 888:  /* *avx_ieee_smaxsf3 */
    case 887:  /* *ieee_smindf3 */
    case 886:  /* *ieee_sminsf3 */
    case 885:  /* *avx_ieee_smindf3 */
    case 884:  /* *avx_ieee_sminsf3 */
    case 787:  /* sse4_1_rounddf2 */
    case 786:  /* sse4_1_roundsf2 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (pat, 1), 0, 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (pat, 1), 0, 1));
      break;

    case 785:  /* *fscalexf4_i387 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 1));
      recog_data.dup_loc[0] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 1);
      recog_data.dup_num[0] = 3;
      recog_data.dup_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 0);
      recog_data.dup_num[1] = 2;
      break;

    case 780:  /* fyl2xp1_extenddfxf3_i387 */
    case 779:  /* fyl2xp1_extendsfxf3_i387 */
    case 777:  /* fyl2x_extenddfxf3_i387 */
    case 776:  /* fyl2x_extendsfxf3_i387 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0), 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 1));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (pat, 0, 1), 0));
      break;

    case 774:  /* fpatan_extenddfxf3_i387 */
    case 773:  /* fpatan_extendsfxf3_i387 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 1), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (pat, 0, 1), 0));
      break;

    case 922:  /* stack_tls_protect_test_di */
    case 921:  /* stack_tls_protect_test_si */
    case 920:  /* stack_protect_test_di */
    case 919:  /* stack_protect_test_si */
    case 778:  /* fyl2xp1xf3_i387 */
    case 775:  /* fyl2xxf3_i387 */
    case 772:  /* *fpatanxf3_i387 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 1));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (pat, 0, 1), 0));
      break;

    case 771:  /* fptan_extenddfxf4_i387 */
    case 770:  /* fptan_extendsfxf4_i387 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 0), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (pat, 0, 0), 1));
      break;

    case 769:  /* fptanxf4_i387 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 0));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (pat, 0, 0), 1));
      break;

    case 783:  /* fxtract_extenddfxf3_i387 */
    case 782:  /* fxtract_extendsfxf3_i387 */
    case 768:  /* sincos_extenddfxf3_i387 */
    case 767:  /* sincos_extendsfxf3_i387 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0), 0));
      recog_data.dup_loc[0] = &XEXP (XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 0), 0);
      recog_data.dup_num[0] = 2;
      break;

    case 781:  /* fxtractxf3_i387 */
    case 766:  /* sincosxf3 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      recog_data.dup_loc[0] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 0);
      recog_data.dup_num[0] = 2;
      break;

    case 759:  /* fprem1xf4_i387 */
    case 758:  /* fpremxf4_i387 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 1));
      recog_data.dup_loc[0] = &XVECEXP (XEXP (XVECEXP (pat, 0, 2), 1), 0, 1);
      recog_data.dup_num[0] = 3;
      recog_data.dup_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 2), 1), 0, 0);
      recog_data.dup_num[1] = 2;
      recog_data.dup_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 1);
      recog_data.dup_num[2] = 3;
      recog_data.dup_loc[3] = &XVECEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0, 0);
      recog_data.dup_num[3] = 2;
      break;

    case 749:  /* *fop_xf_6_i387 */
    case 748:  /* *fop_xf_6_i387 */
    case 737:  /* *fop_df_6_i387 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (pat, 1), 1), 0));
      ro[3] = *(ro_loc[3] = &XEXP (pat, 1));
      break;

    case 747:  /* *fop_xf_5_i387 */
    case 746:  /* *fop_xf_5_i387 */
    case 743:  /* *fop_xf_3_i387 */
    case 742:  /* *fop_xf_3_i387 */
    case 736:  /* *fop_df_5_i387 */
    case 734:  /* *fop_df_3_i387 */
    case 733:  /* *fop_sf_3_i387 */
    case 732:  /* *fop_df_3_i387 */
    case 731:  /* *fop_sf_3_i387 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (pat, 1), 1), 0));
      ro[3] = *(ro_loc[3] = &XEXP (pat, 1));
      break;

    case 745:  /* *fop_xf_4_i387 */
    case 744:  /* *fop_xf_4_i387 */
    case 741:  /* *fop_xf_2_i387 */
    case 740:  /* *fop_xf_2_i387 */
    case 735:  /* *fop_df_4_i387 */
    case 730:  /* *fop_df_2_i387 */
    case 729:  /* *fop_sf_2_i387 */
    case 728:  /* *fop_df_2_i387 */
    case 727:  /* *fop_sf_2_i387 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (pat, 1), 1));
      ro[3] = *(ro_loc[3] = &XEXP (pat, 1));
      break;

    case 1240:  /* sse2_maskcmpv2df3 */
    case 1239:  /* sse_maskcmpv4sf3 */
    case 1238:  /* sse2_maskcmpdf3 */
    case 1237:  /* sse_maskcmpsf3 */
    case 1236:  /* *avx_maskcmpv4df3 */
    case 1235:  /* *avx_maskcmpv8sf3 */
    case 1234:  /* *avx_maskcmpv2df3 */
    case 1233:  /* *avx_maskcmpv4sf3 */
    case 739:  /* *fop_xf_1_i387 */
    case 738:  /* *fop_xf_comm_i387 */
    case 726:  /* *fop_df_1_i387 */
    case 725:  /* *fop_sf_1_i387 */
    case 724:  /* *fop_df_1_sse */
    case 723:  /* *fop_sf_1_sse */
    case 722:  /* *fop_df_1_avx */
    case 721:  /* *fop_sf_1_avx */
    case 719:  /* *fop_df_1_mixed */
    case 718:  /* *fop_sf_1_mixed */
    case 717:  /* *fop_df_1_mixed_avx */
    case 716:  /* *fop_sf_1_mixed_avx */
    case 715:  /* *fop_df_comm_i387 */
    case 714:  /* *fop_sf_comm_i387 */
    case 713:  /* *fop_df_comm_sse */
    case 712:  /* *fop_sf_comm_sse */
    case 711:  /* *fop_df_comm_avx */
    case 710:  /* *fop_sf_comm_avx */
    case 709:  /* *fop_df_comm_mixed */
    case 708:  /* *fop_sf_comm_mixed */
    case 707:  /* *fop_df_comm_mixed_avx */
    case 706:  /* *fop_sf_comm_mixed_avx */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (pat, 1), 1));
      ro[3] = *(ro_loc[3] = &XEXP (pat, 1));
      break;

    case 705:  /* *tls_dynamic_gnu2_combine_64 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1), 0), 0, 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0, 0));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0, 1));
      break;

    case 704:  /* *tls_dynamic_call_64 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 1));
      break;

    case 2038:  /* avx_pd_pd256 */
    case 2037:  /* avx_ps_ps256 */
    case 2036:  /* avx_si_si256 */
    case 2035:  /* avx_pd256_pd */
    case 2034:  /* avx_ps256_ps */
    case 2033:  /* avx_si256_si */
    case 1980:  /* aesimc */
    case 1951:  /* xop_frczv4df2256 */
    case 1950:  /* xop_frczv8sf2256 */
    case 1947:  /* xop_frczv2df2 */
    case 1946:  /* xop_frczv4sf2 */
    case 1846:  /* sse4_1_phminposuw */
    case 1837:  /* sse4_1_movntdqa */
    case 1812:  /* sse4a_movntdf */
    case 1811:  /* sse4a_movntsf */
    case 1754:  /* sse2_pmovmskb */
    case 1753:  /* sse2_movmskpd */
    case 1752:  /* sse_movmskps */
    case 1751:  /* avx_movmskpd256 */
    case 1750:  /* avx_movmskps256 */
    case 1398:  /* avx_cvtpd2dq256 */
    case 1393:  /* sse2_cvtsd2siq_2 */
    case 1391:  /* sse2_cvtsd2si_2 */
    case 1384:  /* sse2_cvtpd2pi */
    case 1379:  /* sse2_cvtps2dq */
    case 1378:  /* avx_cvtps2dq256 */
    case 1377:  /* avx_cvtps2dq */
    case 1371:  /* sse_cvtss2siq_2 */
    case 1369:  /* sse_cvtss2si_2 */
    case 1162:  /* sse_rsqrtv4sf2 */
    case 1161:  /* avx_rsqrtv8sf2 */
    case 1150:  /* sse_rcpv4sf2 */
    case 1149:  /* avx_rcpv8sf2 */
    case 1106:  /* sse3_lddqu */
    case 1105:  /* avx_lddqu */
    case 1104:  /* avx_lddqu256 */
    case 1103:  /* sse2_movntsi */
    case 1102:  /* sse2_movntv2di */
    case 1101:  /* avx_movntv2di */
    case 1100:  /* avx_movntv4di */
    case 1099:  /* sse2_movntv2df */
    case 1098:  /* sse_movntv4sf */
    case 1097:  /* avx_movntv4df */
    case 1096:  /* avx_movntv8sf */
    case 1095:  /* avx_movntv2df */
    case 1094:  /* avx_movntv4sf */
    case 1093:  /* *sse2_movdqu */
    case 1092:  /* *avx_movdqu */
    case 1091:  /* *avx_movdqu256 */
    case 1090:  /* *sse2_movupd */
    case 1089:  /* *sse_movups */
    case 1087:  /* *avx_movupd256 */
    case 1086:  /* *avx_movups256 */
    case 1085:  /* *avx_movupd */
    case 1084:  /* *avx_movups */
    case 1058:  /* mmx_pmovmskb */
    case 968:  /* mmx_rsqrtv2sf2 */
    case 965:  /* mmx_rcpv2sf2 */
    case 957:  /* sse_movntdi */
    case 927:  /* *rdpmc */
    case 828:  /* fxamdf2_i387_with_temp */
    case 827:  /* fxamsf2_i387_with_temp */
    case 826:  /* fxamxf2_i387 */
    case 825:  /* fxamdf2_i387 */
    case 824:  /* fxamsf2_i387 */
    case 795:  /* fistsi2 */
    case 794:  /* fisthi2 */
    case 793:  /* *fistsi2_1 */
    case 792:  /* *fisthi2_1 */
    case 789:  /* *fistdi2_1 */
    case 788:  /* rintxf2 */
    case 784:  /* *f2xm1xf2_i387 */
    case 763:  /* *cosxf2_i387 */
    case 760:  /* *sinxf2_i387 */
    case 755:  /* *rsqrtsf2_sse */
    case 751:  /* truncxfdf2_i387_noop_unspec */
    case 750:  /* truncxfsf2_i387_noop_unspec */
    case 720:  /* *rcpsf2_sse */
    case 703:  /* *tls_dynamic_lea_64 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (pat, 1), 0, 0));
      break;

    case 702:  /* *tls_dynamic_gnu2_combine_32 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1), 0), 0, 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0, 2));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0, 0));
      ro[4] = *(ro_loc[4] = &XVECEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0, 1));
      break;

    case 701:  /* *tls_dynamic_call_32 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 1));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 2));
      break;

    case 700:  /* *tls_dynamic_lea_32 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XEXP (XEXP (pat, 1), 1), 0), 0, 0));
      break;

    case 699:  /* *add_tp_di */
    case 697:  /* *add_tp_si */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1));
      break;

    case 695:  /* *tls_local_dynamic_32_once */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0, 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0, 1));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1), 0), 0, 0));
      ro[4] = *(ro_loc[4] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[5] = *(ro_loc[5] = &XEXP (XVECEXP (pat, 0, 2), 0));
      break;

    case 693:  /* *tls_local_dynamic_base_32_gnu */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 1));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[4] = *(ro_loc[4] = &XEXP (XVECEXP (pat, 0, 2), 0));
      break;

    case 692:  /* *tls_global_dynamic_64 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XVECEXP (pat, 0, 1), 0, 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1));
      break;

    case 691:  /* *tls_global_dynamic_32_gnu */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 1));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 2));
      ro[4] = *(ro_loc[4] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[5] = *(ro_loc[5] = &XEXP (XVECEXP (pat, 0, 2), 0));
      break;

    case 690:  /* *parityhi2_cmp */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      break;

    case 689:  /* paritysi2_cmp */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 2), 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      break;

    case 688:  /* paritydi2_cmp */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 2), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 3), 0));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      break;

    case 681:  /* bswaphi_lowpart */
    case 680:  /* *bswaphi_lowpart_1 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 0), 0));
      recog_data.dup_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0);
      recog_data.dup_num[0] = 0;
      break;

    case 687:  /* *bsrhi */
    case 685:  /* bsr_rex64 */
    case 670:  /* bsr */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1), 0));
      break;

    case 666:  /* *ffsdi_1 */
    case 665:  /* *ffssi_1 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      recog_data.dup_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0);
      recog_data.dup_num[0] = 1;
      break;

    case 1814:  /* sse4a_vmmovntv2df */
    case 1813:  /* sse4a_vmmovntv4sf */
    case 1392:  /* sse2_cvtsd2siq */
    case 1390:  /* sse2_cvtsd2si */
    case 1370:  /* sse_cvtss2siq */
    case 1368:  /* sse_cvtss2si */
    case 765:  /* *cos_extenddfxf2_i387 */
    case 764:  /* *cos_extendsfxf2_i387 */
    case 762:  /* *sin_extenddfxf2_i387 */
    case 761:  /* *sin_extendsfxf2_i387 */
    case 660:  /* set_got_offset_rex64 */
    case 659:  /* set_rip_rex64 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 0));
      break;

    case 657:  /* set_got_labelled */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0), 0));
      break;

    case 652:  /* return_indirect_internal */
    case 651:  /* return_pop_internal */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      break;

    case 1760:  /* sse2_clflush */
    case 1757:  /* sse_ldmxcsr */
    case 934:  /* *lwp_llwpcbdi1 */
    case 933:  /* *lwp_llwpcbsi1 */
    case 655:  /* pad */
    case 648:  /* prologue_use */
      ro[0] = *(ro_loc[0] = &XVECEXP (pat, 0, 0));
      break;

    case 1762:  /* *sse2_lfence */
    case 1761:  /* *sse2_mfence */
    case 1759:  /* *sse_sfence */
    case 647:  /* *memory_blockage */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      recog_data.dup_loc[0] = &XVECEXP (XEXP (pat, 1), 0, 0);
      recog_data.dup_num[0] = 0;
      break;

    case 909:  /* trap */
    case 829:  /* cld */
    case 663:  /* leave_rex64 */
    case 662:  /* leave */
    case 661:  /* eh_return_internal */
    case 653:  /* nop */
    case 650:  /* return_internal_long */
    case 649:  /* return_internal */
    case 646:  /* blockage */
      break;

    case 638:  /* *sibcall_pop_1 */
    case 637:  /* *call_pop_1 */
    case 636:  /* *call_pop_0 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 0), 1));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 1));
      break;

    case 635:  /* *tablejump_1 */
    case 634:  /* *tablejump_1 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 1));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 0), 0));
      break;

    case 633:  /* *indirect_jump */
    case 632:  /* *indirect_jump */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 1));
      break;

    case 631:  /* jump */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (pat, 1), 0));
      break;

    case 630:  /* *fp_jcc_4_si_387 */
    case 629:  /* *fp_jcc_4_hi_387 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 1));
      ro[4] = *(ro_loc[4] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1), 0));
      ro[5] = *(ro_loc[5] = &XEXP (XVECEXP (pat, 0, 3), 0));
      break;

    case 627:  /* *fp_jcc_2r_387 */
    case 625:  /* *fp_jcc_1r_387 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 2), 0));
      ro[4] = *(ro_loc[4] = &XEXP (XVECEXP (pat, 0, 3), 0));
      break;

    case 628:  /* *fp_jcc_3_387 */
    case 626:  /* *fp_jcc_2_387 */
    case 624:  /* *fp_jcc_1_387 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1), 0));
      ro[4] = *(ro_loc[4] = &XEXP (XVECEXP (pat, 0, 3), 0));
      break;

    case 623:  /* *jcc_btsi_mask_1 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0), 0), 1), 0), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0), 0), 1), 0), 1));
      ro[4] = *(ro_loc[4] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1), 0));
      break;

    case 622:  /* *jcc_btsi_1 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0), 0), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1), 0));
      break;

    case 621:  /* *jcc_btsi_mask */
    case 619:  /* *jcc_btdi_mask_rex64 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0), 2), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0), 2), 1));
      ro[4] = *(ro_loc[4] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1), 0));
      break;

    case 620:  /* *jcc_btsi */
    case 618:  /* *jcc_btdi_rex64 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0), 2), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1), 0));
      break;

    case 617:  /* *jcc_2 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XEXP (pat, 1), 2), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 0));
      break;

    case 616:  /* *jcc_1 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XEXP (pat, 1), 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 0));
      break;

    case 1959:  /* xop_maskcmp_unsv2di3 */
    case 1958:  /* xop_maskcmp_unsv4si3 */
    case 1957:  /* xop_maskcmp_unsv8hi3 */
    case 1956:  /* xop_maskcmp_unsv16qi3 */
    case 1955:  /* xop_maskcmpv2di3 */
    case 1954:  /* xop_maskcmpv4si3 */
    case 1953:  /* xop_maskcmpv8hi3 */
    case 1952:  /* xop_maskcmpv16qi3 */
    case 615:  /* *sse_setccdf */
    case 614:  /* *sse_setccsf */
    case 613:  /* *avx_setccdf */
    case 612:  /* *avx_setccsf */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (pat, 1));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (pat, 1), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (pat, 1), 1));
      break;

    case 606:  /* *btsi */
    case 605:  /* *btdi_rex64 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 0), 2));
      break;

    case 604:  /* *btcq */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 0), 2));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0);
      recog_data.dup_num[0] = 0;
      recog_data.dup_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 2);
      recog_data.dup_num[1] = 1;
      break;

    case 603:  /* *btrq */
    case 602:  /* *btsq */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 0), 2));
      break;

    case 589:  /* ix86_rotrdi3 */
    case 576:  /* ix86_rotldi3 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (pat, 0, 2), 0));
      break;

    case 517:  /* x86_shrd */
    case 508:  /* x86_64_shrd */
    case 493:  /* x86_shld */
    case 488:  /* x86_64_shld */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0);
      recog_data.dup_num[0] = 0;
      recog_data.dup_loc[1] = &XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1), 1), 1);
      recog_data.dup_num[1] = 2;
      break;

    case 677:  /* *popcountsi2_cmp_zext */
    case 486:  /* *one_cmplsi2_2_zext */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0), 0);
      recog_data.dup_num[0] = 1;
      break;

    case 476:  /* copysigntf3_var */
    case 475:  /* copysigndf3_var */
    case 474:  /* copysignsf3_var */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 1));
      ro[4] = *(ro_loc[4] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 2));
      ro[5] = *(ro_loc[5] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 3));
      break;

    case 2009:  /* *avx_vperm2f128v4df_full */
    case 2008:  /* *avx_vperm2f128v8sf_full */
    case 2007:  /* *avx_vperm2f128v8si_full */
    case 1983:  /* pclmulqdq */
    case 1982:  /* *vpclmulqdq */
    case 1967:  /* xop_pcom_tfv2di3 */
    case 1966:  /* xop_pcom_tfv4si3 */
    case 1965:  /* xop_pcom_tfv8hi3 */
    case 1964:  /* xop_pcom_tfv16qi3 */
    case 1922:  /* xop_pperm */
    case 1843:  /* sse4_1_pblendvb */
    case 1842:  /* *avx_pblendvb */
    case 1839:  /* sse4_1_mpsadbw */
    case 1838:  /* *avx_mpsadbw */
    case 1836:  /* sse4_1_dppd */
    case 1835:  /* sse4_1_dpps */
    case 1834:  /* avx_dppd256 */
    case 1833:  /* avx_dpps256 */
    case 1832:  /* avx_dppd */
    case 1831:  /* avx_dpps */
    case 1830:  /* sse4_1_blendvpd */
    case 1829:  /* sse4_1_blendvps */
    case 1826:  /* avx_blendvpd256 */
    case 1825:  /* avx_blendvps256 */
    case 1824:  /* avx_blendvpd */
    case 1823:  /* avx_blendvps */
    case 1815:  /* sse4a_extrqi */
    case 1804:  /* ssse3_palignrdi */
    case 1803:  /* ssse3_palignrti */
    case 1802:  /* *avx_palignrti */
    case 1230:  /* avx_cmppdv4df3 */
    case 1229:  /* avx_cmppsv8sf3 */
    case 1228:  /* avx_cmppdv2df3 */
    case 1227:  /* avx_cmppsv4sf3 */
    case 473:  /* copysigntf3_const */
    case 472:  /* copysigndf3_const */
    case 471:  /* copysignsf3_const */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (pat, 1), 0, 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (pat, 1), 0, 1));
      ro[3] = *(ro_loc[3] = &XVECEXP (XEXP (pat, 1), 0, 2));
      break;

    case 458:  /* *absnegtf2_sse */
    case 457:  /* *absnegxf2_i387 */
    case 456:  /* *absnegdf2_i387 */
    case 455:  /* *absnegsf2_i387 */
    case 454:  /* *absnegdf2_sse */
    case 453:  /* *absnegsf2_sse */
    case 452:  /* *absnegdf2_mixed */
    case 451:  /* *absnegsf2_mixed */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (pat, 0, 0), 1));
      break;

    case 450:  /* *negsi2_cmpz_zext */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0), 0), 0));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0), 0), 0);
      recog_data.dup_num[0] = 1;
      break;

    case 676:  /* *popcountdi2_cmp */
    case 675:  /* *popcountsi2_cmp */
    case 674:  /* *popcounthi2_cmp */
    case 485:  /* *one_cmpldi2_2 */
    case 484:  /* *one_cmplsi2_2 */
    case 483:  /* *one_cmplhi2_2 */
    case 482:  /* *one_cmplqi2_2 */
    case 449:  /* *negdi2_cmpz */
    case 448:  /* *negsi2_cmpz */
    case 447:  /* *neghi2_cmpz */
    case 446:  /* *negqi2_cmpz */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      recog_data.dup_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0);
      recog_data.dup_num[0] = 1;
      break;

    case 445:  /* *negsi2_1_zext */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0), 0));
      break;

    case 561:  /* *lshrsi3_cmp_zext */
    case 558:  /* *lshrsi3_cmp_one_bit_zext */
    case 418:  /* *xorsi_2_zext_imm */
    case 417:  /* *iorsi_2_zext_imm */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 1);
      recog_data.dup_num[1] = 2;
      break;

    case 906:  /* *call_value_1_rex64_ms_sysv */
    case 902:  /* *call_value_0_rex64_ms_sysv */
    case 694:  /* *tls_local_dynamic_base_64 */
    case 553:  /* *lshrsi3_1_one_bit_zext */
    case 404:  /* *xorsi_1_zext_imm */
    case 403:  /* *iorsi_1_zext_imm */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1));
      break;

    case 438:  /* *xorqi_cc_ext_1 */
    case 437:  /* *xorqi_cc_ext_1_rex64 */
    case 389:  /* *andqi_ext_0_cc */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 1);
      recog_data.dup_num[1] = 2;
      break;

    case 420:  /* *xorqi_2_slp */
    case 419:  /* *iorqi_2_slp */
    case 387:  /* *andqi_2_slp */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 0), 0);
      recog_data.dup_num[0] = 0;
      recog_data.dup_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0);
      recog_data.dup_num[1] = 0;
      recog_data.dup_loc[2] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 1);
      recog_data.dup_num[2] = 1;
      break;

    case 374:  /* *testqi_ext_3 */
    case 373:  /* *testqi_ext_3_rex64 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 0), 1));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (pat, 1), 0), 2));
      break;

    case 372:  /* *testqi_ext_2 */
    case 371:  /* *testqi_ext_1 */
    case 370:  /* *testqi_ext_1_rex64 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 1), 0));
      break;

    case 369:  /* *testqi_ext_0 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 0), 1));
      break;

    case 363:  /* *udivmoddi4_noext */
    case 362:  /* *udivmodsi4_noext */
    case 361:  /* *udivmodhi4_noext */
    case 357:  /* *divmoddi4_noext */
    case 356:  /* *divmodsi4_noext */
    case 355:  /* *divmodhi4_noext */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1));
      ro[4] = *(ro_loc[4] = &XEXP (XVECEXP (pat, 0, 2), 0));
      recog_data.dup_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0);
      recog_data.dup_num[0] = 2;
      recog_data.dup_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 1);
      recog_data.dup_num[1] = 3;
      break;

    case 360:  /* *udivmoddi4 */
    case 359:  /* *udivmodsi4 */
    case 358:  /* *udivmodhi4 */
    case 354:  /* *divmoddi4 */
    case 353:  /* *divmodsi4 */
    case 352:  /* *divmodhi4 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0);
      recog_data.dup_num[0] = 2;
      recog_data.dup_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 1);
      recog_data.dup_num[1] = 3;
      break;

    case 349:  /* *umulsi3_highpart_zext */
    case 348:  /* *smulsi3_highpart_zext */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0), 0), 1), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (pat, 0, 1), 0));
      break;

    case 347:  /* *umulsi3_highpart_1 */
    case 346:  /* *smulsi3_highpart_1 */
    case 345:  /* *umuldi3_highpart_1 */
    case 344:  /* *smuldi3_highpart_1 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0), 1), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (pat, 0, 1), 0));
      break;

    case 343:  /* *umulqihi3_1 */
    case 342:  /* *mulqihi3_1 */
    case 341:  /* *umulditi3_1 */
    case 340:  /* *mulditi3_1 */
    case 339:  /* *umulsidi3_1 */
    case 338:  /* *mulsidi3_1 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1), 0));
      break;

    case 332:  /* *subsi3_zext_cc_overflow */
    case 331:  /* *addsi3_zext_cc_overflow */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0), 1);
      recog_data.dup_num[1] = 2;
      recog_data.dup_loc[2] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1);
      recog_data.dup_num[2] = 1;
      break;

    case 330:  /* *subdi3_cc_overflow */
    case 329:  /* *adddi3_cc_overflow */
    case 328:  /* *subsi3_cc_overflow */
    case 327:  /* *addsi3_cc_overflow */
    case 326:  /* *subhi3_cc_overflow */
    case 325:  /* *addhi3_cc_overflow */
    case 324:  /* *subqi3_cc_overflow */
    case 323:  /* *addqi3_cc_overflow */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 1);
      recog_data.dup_num[1] = 2;
      recog_data.dup_loc[2] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1);
      recog_data.dup_num[2] = 1;
      break;

    case 322:  /* *subdi3_cconly_overflow */
    case 321:  /* *subsi3_cconly_overflow */
    case 320:  /* *subhi3_cconly_overflow */
    case 319:  /* *subqi3_cconly_overflow */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 0), 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (pat, 1), 1);
      recog_data.dup_num[0] = 0;
      break;

    case 318:  /* *adddi3_cconly_overflow */
    case 317:  /* *addsi3_cconly_overflow */
    case 316:  /* *addhi3_cconly_overflow */
    case 315:  /* *addqi3_cconly_overflow */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1);
      recog_data.dup_num[0] = 1;
      break;

    case 314:  /* *subsi3_carry_zext */
    case 313:  /* *addsi3_carry_zext */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 1), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 1), 0));
      break;

    case 312:  /* *subdi3_carry */
    case 311:  /* *adddi3_carry */
    case 310:  /* *subsi3_carry */
    case 309:  /* *addsi3_carry */
    case 308:  /* *subhi3_carry */
    case 307:  /* *addhi3_carry */
    case 306:  /* *subqi3_carry */
    case 305:  /* *addqi3_carry */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1), 0));
      break;

    case 304:  /* *subsi_3_zext */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0), 1);
      recog_data.dup_num[1] = 2;
      break;

    case 303:  /* *subdi_3 */
    case 302:  /* *subsi_3 */
    case 301:  /* *subhi_3 */
    case 300:  /* *subqi_3 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 1);
      recog_data.dup_num[1] = 2;
      break;

    case 286:  /* *lea_general_3_zext */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 1));
      ro[4] = *(ro_loc[4] = &XEXP (XEXP (XEXP (pat, 1), 0), 1));
      break;

    case 285:  /* *lea_general_3 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XEXP (pat, 1), 0), 1));
      ro[4] = *(ro_loc[4] = &XEXP (XEXP (pat, 1), 1));
      break;

    case 284:  /* *lea_general_2_zext */
    case 282:  /* *lea_general_1_zext */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XEXP (pat, 1), 0), 1));
      break;

    case 1884:  /* xop_pmacssdd */
    case 1883:  /* xop_pmacsdd */
    case 1882:  /* xop_pmacssww */
    case 1881:  /* xop_pmacsww */
    case 1306:  /* fma4_fmsubv2df4 */
    case 1305:  /* fma4_fmsubv4sf4 */
    case 1304:  /* fma4_fmsubdf4 */
    case 1303:  /* fma4_fmsubsf4 */
    case 1300:  /* fma4_fmaddv2df4 */
    case 1299:  /* fma4_fmaddv4sf4 */
    case 1298:  /* fma4_fmadddf4 */
    case 1297:  /* fma4_fmaddsf4 */
    case 1292:  /* fma4_fmsubv4df4256 */
    case 1291:  /* fma4_fmsubv8sf4256 */
    case 1290:  /* fma4_fmaddv4df4256 */
    case 1289:  /* fma4_fmaddv8sf4256 */
    case 283:  /* *lea_general_2 */
    case 281:  /* *lea_general_1 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (pat, 1), 0), 1));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (pat, 1), 1));
      break;

    case 436:  /* *xorqi_ext_2 */
    case 435:  /* *iorqi_ext_2 */
    case 434:  /* *xorqi_ext_1 */
    case 433:  /* *iorqi_ext_1 */
    case 432:  /* *xorqi_ext_1_rex64 */
    case 431:  /* *iorqi_ext_1_rex64 */
    case 392:  /* *andqi_ext_2 */
    case 391:  /* *andqi_ext_1 */
    case 390:  /* *andqi_ext_1_rex64 */
    case 280:  /* *addqi_ext_2 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1), 0));
      break;

    case 430:  /* *xorqi_ext_0 */
    case 429:  /* *iorqi_ext_0 */
    case 388:  /* andqi_ext_0 */
    case 279:  /* addqi_ext_1 */
    case 278:  /* *addqi_ext_1_rex64 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1));
      break;

    case 575:  /* *lshrqi2_cconly */
    case 573:  /* *lshrqi2_one_bit_cconly */
    case 567:  /* *lshrhi3_cconly */
    case 565:  /* *lshrhi3_one_bit_cconly */
    case 560:  /* *lshrsi3_cconly */
    case 557:  /* *lshrsi3_one_bit_cconly */
    case 550:  /* *lshrdi3_cconly_rex64 */
    case 548:  /* *lshrdi3_cconly_one_bit_rex64 */
    case 543:  /* *ashrqi3_cconly */
    case 541:  /* *ashrqi3_one_bit_cconly */
    case 535:  /* *ashrhi3_cconly */
    case 533:  /* *ashrhi3_one_bit_cconly */
    case 528:  /* *ashrsi3_cconly */
    case 525:  /* *ashrsi3_one_bit_cconly */
    case 515:  /* *ashrdi3_cconly_rex64 */
    case 513:  /* *ashrdi3_one_bit_cconly_rex64 */
    case 506:  /* *ashlqi3_cconly */
    case 502:  /* *ashlhi3_cconly */
    case 497:  /* *ashlsi3_cconly */
    case 491:  /* *ashldi3_cconly_rex64 */
    case 428:  /* *xordi_3 */
    case 427:  /* *iordi_3 */
    case 426:  /* *xorsi_3 */
    case 425:  /* *iorsi_3 */
    case 424:  /* *xorhi_3 */
    case 423:  /* *iorhi_3 */
    case 422:  /* *xorqi_3 */
    case 421:  /* *iorqi_3 */
    case 277:  /* *addqi_5 */
    case 276:  /* *addhi_5 */
    case 275:  /* *adddi_5 */
    case 274:  /* *addsi_5 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 1));
      break;

    case 267:  /* *addsi_3_zext */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0), 1);
      recog_data.dup_num[1] = 2;
      break;

    case 269:  /* *addqi_3 */
    case 268:  /* *addhi_3 */
    case 266:  /* *adddi_3 */
    case 265:  /* *addsi_3 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      break;

    case 529:  /* *ashrsi3_cmp_zext */
    case 526:  /* *ashrsi3_one_bit_cmp_zext */
    case 498:  /* *ashlsi3_cmp_zext */
    case 416:  /* *xorsi_2_zext */
    case 415:  /* *iorsi_2_zext */
    case 386:  /* *andsi_2_zext */
    case 299:  /* *subsi_2_zext */
    case 262:  /* *addsi_2_zext */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0), 1);
      recog_data.dup_num[1] = 2;
      break;

    case 574:  /* *lshrqi2_cmp */
    case 572:  /* *lshrqi2_one_bit_cmp */
    case 566:  /* *lshrhi3_cmp */
    case 564:  /* *lshrhi3_one_bit_cmp */
    case 559:  /* *lshrsi3_cmp */
    case 556:  /* *lshrsi3_one_bit_cmp */
    case 549:  /* *lshrdi3_cmp_rex64 */
    case 547:  /* *lshrdi3_cmp_one_bit_rex64 */
    case 542:  /* *ashrqi3_cmp */
    case 540:  /* *ashrqi3_one_bit_cmp */
    case 534:  /* *ashrhi3_cmp */
    case 532:  /* *ashrhi3_one_bit_cmp */
    case 527:  /* *ashrsi3_cmp */
    case 524:  /* *ashrsi3_one_bit_cmp */
    case 514:  /* *ashrdi3_cmp_rex64 */
    case 512:  /* *ashrdi3_one_bit_cmp_rex64 */
    case 505:  /* *ashlqi3_cmp */
    case 501:  /* *ashlhi3_cmp */
    case 496:  /* *ashlsi3_cmp */
    case 490:  /* *ashldi3_cmp_rex64 */
    case 414:  /* *xordi_2 */
    case 413:  /* *iordi_2 */
    case 412:  /* *xorsi_2 */
    case 411:  /* *iorsi_2 */
    case 410:  /* *xorhi_2 */
    case 409:  /* *iorhi_2 */
    case 408:  /* *xorqi_2 */
    case 407:  /* *iorqi_2 */
    case 385:  /* *andsi_2 */
    case 384:  /* *andhi_2 */
    case 383:  /* *andqi_2 */
    case 382:  /* *andqi_2_maybe_si */
    case 381:  /* *anddi_2 */
    case 298:  /* *subdi_2 */
    case 297:  /* *subsi_2 */
    case 296:  /* *subhi_2 */
    case 295:  /* *subqi_2 */
    case 264:  /* *addqi_2 */
    case 263:  /* *addhi_2 */
    case 261:  /* *adddi_2 */
    case 260:  /* *addsi_2 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 1);
      recog_data.dup_num[1] = 2;
      break;

    case 601:  /* *rotrqi3_1_slp */
    case 599:  /* *rotrqi3_1_one_bit_slp */
    case 587:  /* *rotlqi3_1_slp */
    case 585:  /* *rotlqi3_1_one_bit_slp */
    case 571:  /* *lshrqi3_1_slp */
    case 569:  /* *lshrqi3_1_one_bit_slp */
    case 539:  /* *ashrqi3_1_slp */
    case 537:  /* *ashrqi3_1_one_bit_slp */
    case 406:  /* *xorqi_1_slp */
    case 405:  /* *iorqi_1_slp */
    case 380:  /* *andqi_1_slp */
    case 294:  /* *subqi_1_slp */
    case 259:  /* *addqi_1_slp */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0);
      recog_data.dup_num[0] = 0;
      break;

    case 595:  /* *rotrsi3_1_zext */
    case 593:  /* *rotrsi3_1_one_bit_zext */
    case 582:  /* *rotlsi3_1_zext */
    case 580:  /* *rotlsi3_1_one_bit_zext */
    case 555:  /* *lshrsi3_1_zext */
    case 523:  /* *ashrsi3_1_zext */
    case 521:  /* *ashrsi3_1_one_bit_zext */
    case 519:  /* *ashrsi3_31_zext */
    case 495:  /* *ashlsi3_1_zext */
    case 402:  /* *xorsi_1_zext */
    case 401:  /* *iorsi_1_zext */
    case 377:  /* *andsi_1_zext */
    case 335:  /* *mulsi3_1_zext */
    case 293:  /* *subsi_1_zext */
    case 254:  /* *addsi_1_zext */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0), 1));
      break;

    case 247:  /* addqi3_cc */
    case 246:  /* *adddi3_cc */
    case 245:  /* *addsi3_cc */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 0));
      ro[2] = *(ro_loc[2] = &XVECEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0, 1));
      recog_data.dup_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 1), 1), 1);
      recog_data.dup_num[1] = 2;
      break;

    case 893:  /* pro_epilogue_adjust_stack_rex64 */
    case 892:  /* pro_epilogue_adjust_stack_1 */
    case 600:  /* *rotrqi3_1 */
    case 598:  /* *rotrqi3_1_one_bit */
    case 597:  /* *rotrhi3_1 */
    case 596:  /* *rotrhi3_one_bit */
    case 594:  /* *rotrsi3_1 */
    case 592:  /* *rotrsi3_1_one_bit */
    case 591:  /* *rotrdi3_1_rex64 */
    case 590:  /* *rotrdi3_1_one_bit_rex64 */
    case 588:  /* *rotlqi3_1 */
    case 586:  /* *rotlqi3_1_one_bit */
    case 584:  /* *rotlhi3_1 */
    case 583:  /* *rotlhi3_1_one_bit */
    case 581:  /* *rotlsi3_1 */
    case 579:  /* *rotlsi3_1_one_bit */
    case 578:  /* *rotldi3_1_rex64 */
    case 577:  /* *rotlsi3_1_one_bit_rex64 */
    case 570:  /* *lshrqi3_1 */
    case 568:  /* *lshrqi3_1_one_bit */
    case 563:  /* *lshrhi3_1 */
    case 562:  /* *lshrhi3_1_one_bit */
    case 554:  /* *lshrsi3_1 */
    case 552:  /* *lshrsi3_1_one_bit */
    case 551:  /* *lshrdi3_1 */
    case 546:  /* *lshrdi3_1_rex64 */
    case 545:  /* *lshrdi3_1_one_bit_rex64 */
    case 544:  /* *lshrti3_1 */
    case 538:  /* *ashrqi3_1 */
    case 536:  /* *ashrqi3_1_one_bit */
    case 531:  /* *ashrhi3_1 */
    case 530:  /* *ashrhi3_1_one_bit */
    case 522:  /* *ashrsi3_1 */
    case 520:  /* *ashrsi3_1_one_bit */
    case 518:  /* *ashrsi3_31 */
    case 516:  /* *ashrdi3_1 */
    case 511:  /* *ashrdi3_1_rex64 */
    case 510:  /* *ashrdi3_1_one_bit_rex64 */
    case 509:  /* ashrdi3_63_rex64 */
    case 507:  /* *ashrti3_1 */
    case 504:  /* *ashlqi3_1 */
    case 503:  /* *ashlqi3_1_lea */
    case 500:  /* *ashlhi3_1 */
    case 499:  /* *ashlhi3_1_lea */
    case 494:  /* *ashlsi3_1 */
    case 492:  /* *ashldi3_1 */
    case 489:  /* *ashldi3_1_rex64 */
    case 487:  /* *ashlti3_1 */
    case 400:  /* *xorqi_1 */
    case 399:  /* *iorqi_1 */
    case 398:  /* *xordi_1 */
    case 397:  /* *iordi_1 */
    case 396:  /* *xorsi_1 */
    case 395:  /* *iorsi_1 */
    case 394:  /* *xorhi_1 */
    case 393:  /* *iorhi_1 */
    case 379:  /* *andqi_1 */
    case 378:  /* *andhi_1 */
    case 376:  /* *andsi_1 */
    case 375:  /* *anddi_1 */
    case 351:  /* udivqi3 */
    case 350:  /* divqi3 */
    case 337:  /* *mulqi3_1 */
    case 336:  /* *mulhi3_1 */
    case 334:  /* *muldi3_1 */
    case 333:  /* *mulsi3_1 */
    case 292:  /* *subdi_1 */
    case 291:  /* *subsi_1 */
    case 290:  /* *subhi_1 */
    case 289:  /* *subqi_1 */
    case 288:  /* *subti3_doubleword */
    case 287:  /* *subdi3_doubleword */
    case 258:  /* *addqi_1_lea */
    case 257:  /* *addqi_1 */
    case 256:  /* *addhi_1_lea */
    case 255:  /* *addhi_1 */
    case 253:  /* *adddi_1 */
    case 252:  /* *addsi_1 */
    case 244:  /* *addti3_doubleword */
    case 243:  /* *adddi3_doubleword */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1));
      break;

    case 239:  /* floatdixf2_i387_with_xmm */
    case 238:  /* floatdidf2_i387_with_xmm */
    case 237:  /* floatdisf2_i387_with_xmm */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 3), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[4] = *(ro_loc[4] = &XEXP (XVECEXP (pat, 0, 2), 0));
      break;

    case 171:  /* fix_truncdi_i387_with_temp */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (pat, 0, 2), 0));
      ro[4] = *(ro_loc[4] = &XEXP (XVECEXP (pat, 0, 3), 0));
      ro[5] = *(ro_loc[5] = &XEXP (XVECEXP (pat, 0, 4), 0));
      break;

    case 175:  /* fix_truncsi_i387_with_temp */
    case 174:  /* fix_trunchi_i387_with_temp */
    case 170:  /* fix_truncdi_i387 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (pat, 0, 2), 0));
      ro[4] = *(ro_loc[4] = &XEXP (XVECEXP (pat, 0, 3), 0));
      break;

    case 242:  /* *floatunssixf2_1 */
    case 241:  /* *floatunssidf2_1 */
    case 240:  /* *floatunssisf2_1 */
    case 173:  /* fix_truncsi_i387 */
    case 172:  /* fix_trunchi_i387 */
    case 166:  /* fix_truncdi_i387_fisttp_with_temp */
    case 165:  /* fix_truncsi_i387_fisttp_with_temp */
    case 164:  /* fix_trunchi_i387_fisttp_with_temp */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (pat, 0, 2), 0));
      break;

    case 153:  /* *fixuns_truncdf_1 */
    case 152:  /* *fixuns_truncsf_1 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 2), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 3), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[4] = *(ro_loc[4] = &XEXP (XVECEXP (pat, 0, 1), 0));
      break;

    case 1083:  /* movdi_to_sse */
    case 664:  /* *ffs_no_cmove */
    case 230:  /* *floatdixf2_i387_with_temp */
    case 229:  /* *floatsixf2_i387_with_temp */
    case 228:  /* *floatdidf2_i387_with_temp */
    case 227:  /* *floatsidf2_i387_with_temp */
    case 226:  /* *floatdisf2_i387_with_temp */
    case 225:  /* *floatsisf2_i387_with_temp */
    case 216:  /* *floatdidf2_sse_with_temp */
    case 215:  /* *floatdisf2_sse_with_temp */
    case 214:  /* *floatsidf2_sse_with_temp */
    case 213:  /* *floatsisf2_sse_with_temp */
    case 210:  /* *floatsidf2_vector_sse_with_temp */
    case 209:  /* *floatsisf2_vector_sse_with_temp */
    case 200:  /* *floatdidf2_mixed_with_temp */
    case 199:  /* *floatdisf2_mixed_with_temp */
    case 198:  /* *floatsidf2_mixed_with_temp */
    case 197:  /* *floatsisf2_mixed_with_temp */
    case 194:  /* *floatsidf2_vector_mixed_with_temp */
    case 193:  /* *floatsisf2_vector_mixed_with_temp */
    case 183:  /* *floathixf2_i387_with_temp */
    case 182:  /* *floathidf2_i387_with_temp */
    case 181:  /* *floathisf2_i387_with_temp */
    case 163:  /* fix_truncdi_i387_fisttp */
    case 162:  /* fix_truncsi_i387_fisttp */
    case 161:  /* fix_trunchi_i387_fisttp */
    case 147:  /* *truncxfdf2_mixed */
    case 146:  /* *truncxfsf2_mixed */
    case 144:  /* *truncdfsf_i387 */
    case 143:  /* *truncdfsf_mixed */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 1), 0));
      break;

    case 126:  /* *extendsidi2_1 */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (pat, 0, 2), 0));
      break;

    case 863:  /* *x86_movdicc_0_m1_se */
    case 862:  /* *x86_movsicc_0_m1_se */
    case 861:  /* *x86_movdicc_0_m1 */
    case 860:  /* *x86_movsicc_0_m1 */
    case 686:  /* clzhi2_abm */
    case 684:  /* clzdi2_abm */
    case 673:  /* popcountdi2 */
    case 672:  /* popcountsi2 */
    case 671:  /* popcounthi2 */
    case 669:  /* clzsi2_abm */
    case 668:  /* ctzdi2 */
    case 667:  /* ctzsi2 */
    case 444:  /* *negdi2_1 */
    case 443:  /* *negsi2_1 */
    case 442:  /* *neghi2_1 */
    case 441:  /* *negqi2_1 */
    case 440:  /* *negti2_doubleword */
    case 439:  /* *negdi2_doubleword */
    case 169:  /* *fix_truncdi_i387_1 */
    case 168:  /* *fix_truncsi_i387_1 */
    case 167:  /* *fix_trunchi_i387_1 */
    case 122:  /* zero_extendsidi2_32 */
    case 120:  /* *zero_extendqisi2_movzbl_and */
    case 119:  /* *zero_extendqisi2_and */
    case 117:  /* *zero_extendqihi2_movzbw_and */
    case 116:  /* *zero_extendqihi2_and */
    case 114:  /* zero_extendhisi2_and */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      break;

    case 80:  /* *movqi_insv_2 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (pat, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 0));
      break;

    case 1858:  /* sse4_1_zero_extendv2siv2di2 */
    case 1857:  /* sse4_1_zero_extendv2hiv2di2 */
    case 1856:  /* sse4_1_zero_extendv4hiv4si2 */
    case 1855:  /* sse4_1_zero_extendv2qiv2di2 */
    case 1854:  /* sse4_1_zero_extendv4qiv4si2 */
    case 1853:  /* sse4_1_zero_extendv8qiv8hi2 */
    case 1852:  /* sse4_1_extendv2siv2di2 */
    case 1851:  /* sse4_1_extendv2hiv2di2 */
    case 1850:  /* sse4_1_extendv4hiv4si2 */
    case 1849:  /* sse4_1_extendv2qiv2di2 */
    case 1848:  /* sse4_1_extendv4qiv4si2 */
    case 1847:  /* sse4_1_extendv8qiv8hi2 */
    case 1409:  /* sse2_cvtps2pd */
    case 1397:  /* sse2_cvtdq2pd */
    case 1395:  /* sse2_cvttsd2siq */
    case 1394:  /* sse2_cvttsd2si */
    case 1373:  /* sse_cvttss2siq */
    case 1372:  /* sse_cvttss2si */
    case 1363:  /* sse_cvttps2pi */
    case 1088:  /* sse2_movq128 */
    case 1050:  /* *vec_dupv4hi */
    case 754:  /* sqrt_extenddfxf2_i387 */
    case 753:  /* sqrt_extendsfxf2_i387 */
    case 481:  /* *one_cmplsi2_1_zext */
    case 470:  /* *negextenddfxf2 */
    case 469:  /* *absextenddfxf2 */
    case 468:  /* *negextendsfxf2 */
    case 467:  /* *absextendsfxf2 */
    case 466:  /* *negextendsfdf2 */
    case 465:  /* *absextendsfdf2 */
    case 251:  /* *lea_2_zext */
    case 134:  /* *extendqisi2_zext */
    case 131:  /* *extendhisi2_zext */
    case 76:  /* *movqi_extzv_2_rex64 */
    case 75:  /* *movqi_extzv_2 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      break;

    case 643:  /* *call_1_rex64_ms_sysv */
    case 66:  /* *movstrictqi_xor */
    case 59:  /* *movstricthi_xor */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 0), 1));
      break;

    case 113:  /* swapxf */
    case 106:  /* *swapdf */
    case 100:  /* *swapsf */
    case 92:  /* *swapdi_rex64 */
    case 64:  /* *swapqi_2 */
    case 63:  /* *swapqi_1 */
    case 57:  /* *swaphi_2 */
    case 56:  /* *swaphi_1 */
    case 50:  /* *swapsi */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 0), 1));
      recog_data.dup_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0);
      recog_data.dup_num[0] = 1;
      recog_data.dup_loc[1] = &XEXP (XVECEXP (pat, 0, 1), 1);
      recog_data.dup_num[1] = 0;
      break;

    case 1989:  /* vec_dupv4df */
    case 1988:  /* vec_dupv4di */
    case 1987:  /* vec_dupv8sf */
    case 1986:  /* vec_dupv8si */
    case 1810:  /* absv2si2 */
    case 1809:  /* absv4hi2 */
    case 1808:  /* absv8qi2 */
    case 1807:  /* absv4si2 */
    case 1806:  /* absv8hi2 */
    case 1805:  /* absv16qi2 */
    case 1732:  /* *vec_dupv2di */
    case 1731:  /* *vec_dupv2di_sse3 */
    case 1730:  /* *vec_dupv2di_avx */
    case 1729:  /* *vec_dupv4si */
    case 1728:  /* *vec_dupv4si_avx */
    case 1727:  /* *vec_extractv2di_1_sse */
    case 1726:  /* *vec_extractv2di_1_sse2 */
    case 1725:  /* *vec_extractv2di_1_avx */
    case 1724:  /* *vec_extractv2di_1_rex64 */
    case 1723:  /* *vec_extractv2di_1_rex64_avx */
    case 1722:  /* *sse2_storeq */
    case 1721:  /* *sse2_storeq_rex64 */
    case 1719:  /* sse2_stored */
    case 1501:  /* vec_dupv2df */
    case 1500:  /* *vec_dupv2df_sse3 */
    case 1497:  /* *vec_extractv2df_0_sse */
    case 1496:  /* *vec_extractv2df_1_sse */
    case 1491:  /* sse2_storelpd */
    case 1490:  /* sse2_storehpd */
    case 1489:  /* *avx_storehpd */
    case 1469:  /* vec_extract_hi_v32qi */
    case 1468:  /* vec_extract_lo_v32qi */
    case 1467:  /* vec_extract_hi_v16hi */
    case 1466:  /* vec_extract_lo_v16hi */
    case 1465:  /* vec_extract_hi_v8sf */
    case 1464:  /* vec_extract_hi_v8si */
    case 1463:  /* vec_extract_lo_v8sf */
    case 1462:  /* vec_extract_lo_v8si */
    case 1461:  /* vec_extract_hi_v4df */
    case 1460:  /* vec_extract_hi_v4di */
    case 1459:  /* vec_extract_lo_v4df */
    case 1458:  /* vec_extract_lo_v4di */
    case 1457:  /* *vec_extractv4sf_0 */
    case 1439:  /* *vec_dupv4sf */
    case 1438:  /* *vec_dupv4sf_avx */
    case 1433:  /* sse_storelps */
    case 1432:  /* *avx_storelps */
    case 1429:  /* sse_storehps */
    case 1408:  /* avx_cvtps2pd256 */
    case 1406:  /* avx_cvtpd2ps256 */
    case 1400:  /* avx_cvttpd2dq256 */
    case 1396:  /* avx_cvtdq2pd256 */
    case 1385:  /* sse2_cvttpd2pi */
    case 1383:  /* sse2_cvtpi2pd */
    case 1382:  /* sse2_cvttps2dq */
    case 1381:  /* avx_cvttps2dq256 */
    case 1380:  /* avx_cvttps2dq */
    case 1376:  /* sse2_cvtdq2ps */
    case 1375:  /* avx_cvtdq2ps256 */
    case 1374:  /* avx_cvtdq2ps */
    case 1156:  /* sqrtv2df2 */
    case 1155:  /* sqrtv4df2 */
    case 1154:  /* sse_sqrtv4sf2 */
    case 1153:  /* avx_sqrtv8sf2 */
    case 1054:  /* *vec_extractv2si_1 */
    case 1053:  /* *vec_extractv2si_0 */
    case 1051:  /* *vec_dupv2si */
    case 1049:  /* mmx_pswapdv2si2 */
    case 984:  /* *vec_extractv2sf_1 */
    case 983:  /* *vec_extractv2sf_0 */
    case 981:  /* *vec_dupv2sf */
    case 980:  /* mmx_pswapdv2sf2 */
    case 979:  /* mmx_floatv2si2 */
    case 976:  /* mmx_pf2id */
    case 865:  /* *x86_movdicc_0_m1_neg */
    case 864:  /* *x86_movsicc_0_m1_neg */
    case 757:  /* *sqrtdf2_sse */
    case 756:  /* *sqrtsf2_sse */
    case 752:  /* sqrtxf2 */
    case 683:  /* *bswapdi_1 */
    case 682:  /* *bswapdi_movbe */
    case 679:  /* *bswapsi_1 */
    case 678:  /* *bswapsi_movbe */
    case 480:  /* *one_cmplqi2_1 */
    case 479:  /* *one_cmpldi2_1 */
    case 478:  /* *one_cmplsi2_1 */
    case 477:  /* *one_cmplhi2_1 */
    case 464:  /* *negxf2_1 */
    case 463:  /* *absxf2_1 */
    case 462:  /* *negdf2_1 */
    case 461:  /* *absdf2_1 */
    case 460:  /* *negsf2_1 */
    case 459:  /* *abssf2_1 */
    case 250:  /* *lea_2 */
    case 236:  /* *floatdixf2_i387 */
    case 235:  /* *floatsixf2_i387 */
    case 234:  /* *floatdidf2_i387 */
    case 233:  /* *floatsidf2_i387 */
    case 232:  /* *floatdisf2_i387 */
    case 231:  /* *floatsisf2_i387 */
    case 224:  /* *floatdidf2_sse_nointerunit */
    case 223:  /* *floatdisf2_sse_nointerunit */
    case 222:  /* *floatsidf2_sse_nointerunit */
    case 221:  /* *floatsisf2_sse_nointerunit */
    case 220:  /* *floatdidf2_sse_interunit */
    case 219:  /* *floatdisf2_sse_interunit */
    case 218:  /* *floatsidf2_sse_interunit */
    case 217:  /* *floatsisf2_sse_interunit */
    case 212:  /* *floatsidf2_vector_sse */
    case 211:  /* *floatsisf2_vector_sse */
    case 208:  /* *floatdidf2_mixed_nointerunit */
    case 207:  /* *floatdisf2_mixed_nointerunit */
    case 206:  /* *floatsidf2_mixed_nointerunit */
    case 205:  /* *floatsisf2_mixed_nointerunit */
    case 204:  /* *floatdidf2_mixed_interunit */
    case 203:  /* *floatdisf2_mixed_interunit */
    case 202:  /* *floatsidf2_mixed_interunit */
    case 201:  /* *floatsisf2_mixed_interunit */
    case 196:  /* *floatsidf2_vector_mixed */
    case 195:  /* *floatsisf2_vector_mixed */
    case 192:  /* *floatdixf2_1 */
    case 191:  /* *floatsixf2_1 */
    case 190:  /* *floatdidf2_1 */
    case 189:  /* *floatsidf2_1 */
    case 188:  /* *floatdisf2_1 */
    case 187:  /* *floatsisf2_1 */
    case 186:  /* *floathixf2_i387 */
    case 185:  /* *floathidf2_i387 */
    case 184:  /* *floathisf2_i387 */
    case 180:  /* *floathixf2_1 */
    case 179:  /* *floathidf2_1 */
    case 178:  /* *floathisf2_1 */
    case 160:  /* fix_truncdi_fisttp_i387_1 */
    case 159:  /* fix_truncsi_fisttp_i387_1 */
    case 158:  /* fix_trunchi_fisttp_i387_1 */
    case 157:  /* fix_truncdfsi_sse */
    case 156:  /* fix_truncsfsi_sse */
    case 155:  /* fix_truncdfdi_sse */
    case 154:  /* fix_truncsfdi_sse */
    case 151:  /* *truncxfdf2_i387 */
    case 150:  /* *truncxfsf2_i387 */
    case 149:  /* truncxfdf2_i387_noop */
    case 148:  /* truncxfsf2_i387_noop */
    case 145:  /* *truncdfsf2_i387_1 */
    case 142:  /* *truncdfsf_fast_i387 */
    case 141:  /* *truncdfsf_fast_sse */
    case 140:  /* *truncdfsf_fast_mixed */
    case 139:  /* *extenddfxf2_i387 */
    case 138:  /* *extendsfxf2_i387 */
    case 137:  /* *extendsfdf2_i387 */
    case 136:  /* *extendsfdf2_sse */
    case 135:  /* *extendsfdf2_mixed */
    case 133:  /* extendqisi2 */
    case 132:  /* extendqihi2 */
    case 130:  /* extendhisi2 */
    case 129:  /* extendqidi2 */
    case 128:  /* extendhidi2 */
    case 127:  /* extendsidi2_rex64 */
    case 125:  /* zero_extendqidi2 */
    case 124:  /* zero_extendhidi2 */
    case 123:  /* zero_extendsidi2_rex64 */
    case 121:  /* *zero_extendqisi2_movzbl */
    case 118:  /* *zero_extendqihi2_movzbl */
    case 115:  /* *zero_extendhisi2_movzwl */
    case 91:  /* *movabsdi_2_rex64 */
    case 74:  /* *movsi_extzv_1 */
    case 73:  /* *movdi_extzv_1 */
    case 72:  /* *movabsqi_2_rex64 */
    case 70:  /* *movqi_extv_1_rex64 */
    case 69:  /* *movqi_extv_1 */
    case 68:  /* *movhi_extv_1 */
    case 67:  /* *movsi_extv_1 */
    case 55:  /* *movabshi_2_rex64 */
    case 49:  /* *movabssi_2_rex64 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 0));
      break;

    case 645:  /* *sibcall_1_rex64 */
    case 644:  /* *call_1_rex64_large */
    case 642:  /* *call_1_rex64 */
    case 641:  /* *sibcall_1 */
    case 640:  /* *call_1 */
    case 639:  /* *call_0 */
    case 611:  /* *setcc_qi_slp */
    case 90:  /* *movabsdi_1_rex64 */
    case 79:  /* movdi_insv_1_rex64 */
    case 78:  /* *movsi_insv_1_rex64 */
    case 77:  /* movsi_insv_1 */
    case 71:  /* *movabsqi_1_rex64 */
    case 65:  /* *movstrictqi_1 */
    case 58:  /* *movstricthi_1 */
    case 54:  /* *movabshi_1_rex64 */
    case 48:  /* *movabssi_1_rex64 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (pat, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (pat, 1));
      break;

    case 656:  /* set_got */
    case 85:  /* popdi1 */
    case 84:  /* *popdi1_epilogue_rex64 */
    case 44:  /* popsi1 */
    case 43:  /* *popsi1_epilogue */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      break;

    case 654:  /* vswapmov */
    case 608:  /* *setcc_si_1_and */
    case 87:  /* *movdi_or_rex64 */
    case 86:  /* *movdi_xor_rex64 */
    case 83:  /* *pushdi2_prologue_rex64 */
    case 46:  /* *movsi_or */
    case 45:  /* *movsi_xor */
    case 42:  /* *pushsi2_prologue */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (pat, 0, 0), 1));
      break;

    case 1082:  /* *movv2df_internal */
    case 1081:  /* *movv4sf_internal */
    case 1080:  /* *movv1ti_internal */
    case 1079:  /* *movv2di_internal */
    case 1078:  /* *movv4si_internal */
    case 1077:  /* *movv8hi_internal */
    case 1076:  /* *movv16qi_internal */
    case 1075:  /* *avx_movv4df_internal */
    case 1074:  /* *avx_movv8sf_internal */
    case 1073:  /* *avx_movv4di_internal */
    case 1072:  /* *avx_movv8si_internal */
    case 1071:  /* *avx_movv16hi_internal */
    case 1070:  /* *avx_movv32qi_internal */
    case 1069:  /* *avx_movv2df_internal */
    case 1068:  /* *avx_movv4sf_internal */
    case 1067:  /* *avx_movv1ti_internal */
    case 1066:  /* *avx_movv2di_internal */
    case 1065:  /* *avx_movv4si_internal */
    case 1064:  /* *avx_movv8hi_internal */
    case 1063:  /* *avx_movv16qi_internal */
    case 956:  /* *movv2sf_internal */
    case 955:  /* *movv2sf_internal_avx */
    case 954:  /* *movv2sf_internal_rex64 */
    case 953:  /* *movv2sf_internal_rex64_avx */
    case 952:  /* *movv1di_internal */
    case 951:  /* *movv2si_internal */
    case 950:  /* *movv4hi_internal */
    case 949:  /* *movv8qi_internal */
    case 948:  /* *movv1di_internal_avx */
    case 947:  /* *movv2si_internal_avx */
    case 946:  /* *movv4hi_internal_avx */
    case 945:  /* *movv8qi_internal_avx */
    case 944:  /* *movv1di_internal_rex64 */
    case 943:  /* *movv2si_internal_rex64 */
    case 942:  /* *movv4hi_internal_rex64 */
    case 941:  /* *movv8qi_internal_rex64 */
    case 914:  /* *prefetch_3dnow_rex */
    case 913:  /* *prefetch_3dnow */
    case 610:  /* *setcc_qi */
    case 609:  /* *setcc_si_1_movzbl */
    case 607:  /* *setcc_di_1 */
    case 249:  /* *lea_1 */
    case 248:  /* *lea_1 */
    case 112:  /* *pushtf_sse */
    case 111:  /* *movtf_internal */
    case 110:  /* *movxf_integer */
    case 109:  /* *movxf_nointeger */
    case 108:  /* *pushxf_integer */
    case 107:  /* *pushxf_nointeger */
    case 105:  /* *movdf_integer */
    case 104:  /* *movdf_integer_rex64 */
    case 103:  /* *movdf_nointeger */
    case 102:  /* *pushdf_integer */
    case 101:  /* *pushdf_nointeger */
    case 99:  /* *movsf_1 */
    case 98:  /* *pushsf_rex64 */
    case 97:  /* *pushsf */
    case 96:  /* *movti_rex64 */
    case 95:  /* *movti_internal */
    case 94:  /* *pushti */
    case 93:  /* *movoi_internal */
    case 89:  /* *movdi_1_rex64 */
    case 88:  /* *movdi_2 */
    case 82:  /* *pushdi2_rex64 */
    case 81:  /* *pushdi */
    case 62:  /* *movqi_1 */
    case 61:  /* *pushqi2_rex64 */
    case 60:  /* *pushqi2 */
    case 53:  /* *movhi_1 */
    case 52:  /* *pushhi2_rex64 */
    case 51:  /* *pushhi2 */
    case 47:  /* *movsi_1 */
    case 41:  /* *pushsi2_rex64 */
    case 40:  /* *pushsi2 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (pat, 1));
      break;

    case 177:  /* x86_fldcw_1 */
    case 33:  /* x86_sahf_1 */
      ro[0] = *(ro_loc[0] = &XVECEXP (XEXP (pat, 1), 0, 0));
      break;

    case 1758:  /* sse_stmxcsr */
    case 936:  /* lwp_slwpcbdi */
    case 935:  /* lwp_slwpcbsi */
    case 929:  /* *rdtsc */
    case 698:  /* *load_tp_di */
    case 696:  /* *load_tp_si */
    case 658:  /* set_got_rex64 */
    case 176:  /* x86_fnstcw_1 */
    case 32:  /* x86_fnstsw_1 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      break;

    case 31:  /* *cmpfp_si_cc */
    case 30:  /* *cmpfp_hi_cc */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1));
      break;

    case 29:  /* *cmpfp_si */
    case 28:  /* *cmpfp_hi */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 1), 0));
      ro[3] = *(ro_loc[3] = &XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 1));
      break;

    case 273:  /* *addqi_4 */
    case 272:  /* *addhi_4 */
    case 271:  /* *addsi_4 */
    case 270:  /* *adddi_4 */
    case 27:  /* *cmpfp_u_cc */
    case 25:  /* *cmpfp_df_cc */
    case 24:  /* *cmpfp_sf_cc */
    case 21:  /* *cmpfp_xf_cc */
    case 19:  /* *cmpfp_0_cc */
      ro[0] = *(ro_loc[0] = &XEXP (XVECEXP (pat, 0, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XEXP (XVECEXP (pat, 0, 0), 1), 1));
      break;

    case 26:  /* *cmpfp_u */
    case 23:  /* *cmpfp_df */
    case 22:  /* *cmpfp_sf */
    case 20:  /* *cmpfp_xf */
    case 18:  /* *cmpfp_0 */
      ro[0] = *(ro_loc[0] = &XEXP (pat, 0));
      ro[1] = *(ro_loc[1] = &XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 0));
      ro[2] = *(ro_loc[2] = &XEXP (XVECEXP (XEXP (pat, 1), 0, 0), 1));
      break;

    case 17:  /* *cmpqi_ext_4 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 0), 0));
      break;

    case 16:  /* *cmpqi_ext_3_insn_rex64 */
    case 15:  /* *cmpqi_ext_3_insn */
    case 14:  /* *cmpqi_ext_2 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 0), 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 1));
      break;

    case 13:  /* *cmpqi_ext_1_rex64 */
    case 12:  /* *cmpqi_ext_1 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (pat, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (XEXP (pat, 1), 1), 0), 0));
      break;

    case 368:  /* *testsi_1 */
    case 367:  /* *testhi_1 */
    case 366:  /* *testqi_1 */
    case 365:  /* *testqi_1_maybe_si */
    case 364:  /* *testdi_1 */
    case 11:  /* *cmpdi_minus_1 */
    case 10:  /* *cmpsi_minus_1 */
    case 9:  /* *cmphi_minus_1 */
    case 8:  /* *cmpqi_minus_1 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (XEXP (pat, 1), 0), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (XEXP (pat, 1), 0), 1));
      break;

    case 39:  /* *cmpfp_iu_387 */
    case 38:  /* *cmpfp_iu_sse */
    case 37:  /* *cmpfp_iu_mixed */
    case 36:  /* *cmpfp_i_i387 */
    case 35:  /* *cmpfp_i_sse */
    case 34:  /* *cmpfp_i_mixed */
    case 7:  /* *cmpdi_1 */
    case 6:  /* *cmpsi_1 */
    case 5:  /* *cmphi_1 */
    case 4:  /* *cmpqi_1 */
    case 3:  /* *cmpdi_ccno_1 */
    case 2:  /* *cmpsi_ccno_1 */
    case 1:  /* *cmphi_ccno_1 */
    case 0:  /* *cmpqi_ccno_1 */
      ro[0] = *(ro_loc[0] = &XEXP (XEXP (pat, 1), 0));
      ro[1] = *(ro_loc[1] = &XEXP (XEXP (pat, 1), 1));
      break;

    }
}
