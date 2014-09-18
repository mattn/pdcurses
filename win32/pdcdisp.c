/* Public Domain Curses */

#include "pdcwin.h"

RCSID("$Id: pdcdisp.c,v 1.47 2008/07/14 04:24:52 wmcbrine Exp $")

#include <stdlib.h>
#include <string.h>

#ifdef CHTYPE_LONG

# define A(x) ((chtype)x | A_ALTCHARSET)

chtype acs_map[128] =
{
    A(0), A(1), A(2), A(3), A(4), A(5), A(6), A(7), A(8), A(9), A(10),
    A(11), A(12), A(13), A(14), A(15), A(16), A(17), A(18), A(19),
    A(20), A(21), A(22), A(23), A(24), A(25), A(26), A(27), A(28), 
    A(29), A(30), A(31), ' ', '!', '"', '#', '$', '%', '&', '\'', '(', 
    ')', '*',

# ifdef PDC_WIDE
    0x2192, 0x2190, 0x2191, 0x2193,
# else
    A(0x1a), A(0x1b), A(0x18), A(0x19),
# endif

    '/',

# ifdef PDC_WIDE
    0x2588,
# else
    0xdb,
# endif

    '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=',
    '>', '?', '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
    'X', 'Y', 'Z', '[', '\\', ']', '^', '_',

# ifdef PDC_WIDE
    0x2666, 0x2592,
# else
    A(0x04), 0xb1,
# endif

    'b', 'c', 'd', 'e',

# ifdef PDC_WIDE
    0x00b0, 0x00b1, 0x2591, 0x00a4, 0x2518, 0x2510, 0x250c, 0x2514,
    0x253c, 0x23ba, 0x23bb, 0x2500, 0x23bc, 0x23bd, 0x251c, 0x2524,
    0x2534, 0x252c, 0x2502, 0x2264, 0x2265, 0x03c0, 0x2260, 0x00a3,
    0x00b7,
# else
    0xf8, 0xf1, 0xb0, A(0x0f), 0xd9, 0xbf, 0xda, 0xc0, 0xc5, 0x2d, 0x2d,
    0xc4, 0x2d, 0x5f, 0xc3, 0xb4, 0xc1, 0xc2, 0xb3, 0xf3, 0xf2, 0xe3,
    0xd8, 0x9c, 0xf9,
# endif

    A(127)
};

# undef A

#endif

/* position hardware cursor at (y, x) */

void PDC_gotoyx(int row, int col)
{
    COORD coord;

    PDC_LOG(("PDC_gotoyx() - called: row %d col %d from row %d col %d\n",
             row, col, SP->cursrow, SP->curscol));

    coord.X = col;
    coord.Y = row;

    SetConsoleCursorPosition(pdc_con_out, coord);
}

#if defined(PDC_FORCE_UTF8) && !defined(PDC_WIDE)
static char utf8len_tab[256] =
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,0,0,
};

static int to_wchar(const chtype* p, int max_len, int *off)
{
#define CH(x) ((unsigned char) (p[x] & A_CHARTEXT))
    int len;

    *off = 1;
    if (CH(0) < 0x80)
        return CH(0);

    len = utf8len_tab[CH(0)];
    if (len > 1 && (CH(1) & 0xc0) == 0x80 && max_len > 0)
    {
        if (len == 2)
		{
            *off = 2;
            return ((CH(0) & 0x1f) << 6) + (CH(1) & 0x3f);
        }
        if ((CH(2) & 0xc0) == 0x80 && max_len > 1)
        {
            if (len == 3)
            {
                *off = 3;
                return ((CH(0) & 0x0f) << 12) + ((CH(1) & 0x3f) << 6)
                    + (CH(2) & 0x3f);
            }
            if ((CH(3) & 0xc0) == 0x80 && max_len > 2)
            {
                if (len == 4)
                {
                    *off = 4;
                    return ((CH(0) & 0x07) << 18) + ((CH(1) & 0x3f) << 12)
                        + ((CH(2) & 0x3f) << 6) + (CH(3) & 0x3f);
                }
                if ((CH(4) & 0xc0) == 0x80 && max_len > 3)
                {
                    if (len == 5)
                    {
                        *off = 5;
                        return ((CH(0) & 0x03) << 24) + ((CH(1) & 0x3f) << 18)
                            + ((CH(2) & 0x3f) << 12) + ((CH(3) & 0x3f) << 6)
                            + (CH(4) & 0x3f);
                    }
                    if ((CH(5) & 0xc0) == 0x80 && len == 6 && max_len > 4)
                    {
                        *off = 6;
                        return ((CH(0) & 0x01) << 30) + ((CH(1) & 0x3f) << 24)
                            + ((CH(2) & 0x3f) << 18) + ((CH(3) & 0x3f) << 12)
                            + ((CH(4) & 0x3f) << 6) + (CH(5) & 0x3f);
                    }
                }
            }
        }
    }
    return CH(0);
#undef CH
}

struct interval
{
    long first;
    long last;
};

static int bisearch(wchar_t ucs, const struct interval *table, int max)
{
    int min = 0;
    int mid;

    if (ucs < table[0].first || ucs > table[max].last)
        return 0;
    while (max >= min) {
        mid = (min + max) / 2;
        if (ucs > table[mid].last)
            min = mid + 1;
        else if (ucs < table[mid].first)
            max = mid - 1;
        else
            return 1;
    }

    return 0;
}

static int wcwidth(wchar_t ucs)
{
    /* sorted list of non-overlapping intervals of non-spacing characters */
    static const struct interval combining[] =
    {
        { 0x0300, 0x034E }, { 0x0360, 0x0362 }, { 0x0483, 0x0486 },
        { 0x0488, 0x0489 }, { 0x0591, 0x05A1 }, { 0x05A3, 0x05B9 },
        { 0x05BB, 0x05BD }, { 0x05BF, 0x05BF }, { 0x05C1, 0x05C2 },
        { 0x05C4, 0x05C4 }, { 0x064B, 0x0655 }, { 0x0670, 0x0670 },
        { 0x06D6, 0x06E4 }, { 0x06E7, 0x06E8 }, { 0x06EA, 0x06ED },
        { 0x070F, 0x070F }, { 0x0711, 0x0711 }, { 0x0730, 0x074A },
        { 0x07A6, 0x07B0 }, { 0x0901, 0x0902 }, { 0x093C, 0x093C },
        { 0x0941, 0x0948 }, { 0x094D, 0x094D }, { 0x0951, 0x0954 },
        { 0x0962, 0x0963 }, { 0x0981, 0x0981 }, { 0x09BC, 0x09BC },
        { 0x09C1, 0x09C4 }, { 0x09CD, 0x09CD }, { 0x09E2, 0x09E3 },
        { 0x0A02, 0x0A02 }, { 0x0A3C, 0x0A3C }, { 0x0A41, 0x0A42 },
        { 0x0A47, 0x0A48 }, { 0x0A4B, 0x0A4D }, { 0x0A70, 0x0A71 },
        { 0x0A81, 0x0A82 }, { 0x0ABC, 0x0ABC }, { 0x0AC1, 0x0AC5 },
        { 0x0AC7, 0x0AC8 }, { 0x0ACD, 0x0ACD }, { 0x0B01, 0x0B01 },
        { 0x0B3C, 0x0B3C }, { 0x0B3F, 0x0B3F }, { 0x0B41, 0x0B43 },
        { 0x0B4D, 0x0B4D }, { 0x0B56, 0x0B56 }, { 0x0B82, 0x0B82 },
        { 0x0BC0, 0x0BC0 }, { 0x0BCD, 0x0BCD }, { 0x0C3E, 0x0C40 },
        { 0x0C46, 0x0C48 }, { 0x0C4A, 0x0C4D }, { 0x0C55, 0x0C56 },
        { 0x0CBF, 0x0CBF }, { 0x0CC6, 0x0CC6 }, { 0x0CCC, 0x0CCD },
        { 0x0D41, 0x0D43 }, { 0x0D4D, 0x0D4D }, { 0x0DCA, 0x0DCA },
        { 0x0DD2, 0x0DD4 }, { 0x0DD6, 0x0DD6 }, { 0x0E31, 0x0E31 },
        { 0x0E34, 0x0E3A }, { 0x0E47, 0x0E4E }, { 0x0EB1, 0x0EB1 },
        { 0x0EB4, 0x0EB9 }, { 0x0EBB, 0x0EBC }, { 0x0EC8, 0x0ECD },
        { 0x0F18, 0x0F19 }, { 0x0F35, 0x0F35 }, { 0x0F37, 0x0F37 },
        { 0x0F39, 0x0F39 }, { 0x0F71, 0x0F7E }, { 0x0F80, 0x0F84 },
        { 0x0F86, 0x0F87 }, { 0x0F90, 0x0F97 }, { 0x0F99, 0x0FBC },
        { 0x0FC6, 0x0FC6 }, { 0x102D, 0x1030 }, { 0x1032, 0x1032 },
        { 0x1036, 0x1037 }, { 0x1039, 0x1039 }, { 0x1058, 0x1059 },
        { 0x1160, 0x11FF }, { 0x17B7, 0x17BD }, { 0x17C6, 0x17C6 },
        { 0x17C9, 0x17D3 }, { 0x180B, 0x180E }, { 0x18A9, 0x18A9 },
        { 0x200B, 0x200F }, { 0x202A, 0x202E }, { 0x206A, 0x206F },
        { 0x20D0, 0x20E3 }, { 0x302A, 0x302F }, { 0x3099, 0x309A },
        { 0xFB1E, 0xFB1E }, { 0xFE20, 0xFE23 }, { 0xFEFF, 0xFEFF },
        { 0xFFF9, 0xFFFB }
    };

    /* test for 8-bit control characters */
    if (ucs == 0)
        return 0;
    if (ucs < 32 || (ucs >= 0x7f && ucs < 0xa0))
        return -1;

    /* binary search in table of non-spacing characters */
    if (bisearch(ucs, combining,
                sizeof(combining) / sizeof(struct interval) - 1))
        return 0;

    /* if we arrive here, ucs is not a combining or C0/C1 control character */

    return 1 +
        (ucs >= 0x1100 &&
         (ucs <= 0x115f ||                    /* Hangul Jamo init. consonants */
          (ucs >= 0x2e80 && ucs <= 0xa4cf && (ucs & ~0x0011) != 0x300a &&
           ucs != 0x303f) ||                  /* CJK ... Yi */
          (ucs >= 0xac00 && ucs <= 0xd7a3) || /* Hangul Syllables */
          (ucs >= 0xf900 && ucs <= 0xfaff) || /* CJK Compatibility Ideographs */
          (ucs >= 0xfe30 && ucs <= 0xfe6f) || /* CJK Compatibility Forms */
          (ucs >= 0xff00 && ucs <= 0xff5f) || /* Fullwidth Forms */
          (ucs >= 0xffe0 && ucs <= 0xffe6) ||
#ifndef _WIN32
          (ucs >= 0x20000 && ucs <= 0x2ffff)
#else
          0
#endif
         ));
}

static int wcwidth_cjk(wchar_t ucs)
{
    /* sorted list of non-overlapping intervals of East Asian Ambiguous
     * characters */
    static const struct interval ambiguous[] =
    {
        { 0x00A1, 0x00A1 }, { 0x00A4, 0x00A4 }, { 0x00A7, 0x00A8 },
        { 0x00AA, 0x00AA }, { 0x00AD, 0x00AE }, { 0x00B0, 0x00B4 },
        { 0x00B6, 0x00BA }, { 0x00BC, 0x00BF }, { 0x00C6, 0x00C6 },
        { 0x00D0, 0x00D0 }, { 0x00D7, 0x00D8 }, { 0x00DE, 0x00E1 },
        { 0x00E6, 0x00E6 }, { 0x00E8, 0x00EA }, { 0x00EC, 0x00ED },
        { 0x00F0, 0x00F0 }, { 0x00F2, 0x00F3 }, { 0x00F7, 0x00FA },
        { 0x00FC, 0x00FC }, { 0x00FE, 0x00FE }, { 0x0101, 0x0101 },
        { 0x0111, 0x0111 }, { 0x0113, 0x0113 }, { 0x011B, 0x011B },
        { 0x0126, 0x0127 }, { 0x012B, 0x012B }, { 0x0131, 0x0133 },
        { 0x0138, 0x0138 }, { 0x013F, 0x0142 }, { 0x0144, 0x0144 },
        { 0x0148, 0x014B }, { 0x014D, 0x014D }, { 0x0152, 0x0153 },
        { 0x0166, 0x0167 }, { 0x016B, 0x016B }, { 0x01CE, 0x01CE },
        { 0x01D0, 0x01D0 }, { 0x01D2, 0x01D2 }, { 0x01D4, 0x01D4 },
        { 0x01D6, 0x01D6 }, { 0x01D8, 0x01D8 }, { 0x01DA, 0x01DA },
        { 0x01DC, 0x01DC }, { 0x0251, 0x0251 }, { 0x0261, 0x0261 },
        { 0x02C4, 0x02C4 }, { 0x02C7, 0x02C7 }, { 0x02C9, 0x02CB },
        { 0x02CD, 0x02CD }, { 0x02D0, 0x02D0 }, { 0x02D8, 0x02DB },
        { 0x02DD, 0x02DD }, { 0x02DF, 0x02DF }, { 0x0300, 0x034E },
        { 0x0360, 0x0362 }, { 0x0391, 0x03A1 }, { 0x03A3, 0x03A9 },
        { 0x03B1, 0x03C1 }, { 0x03C3, 0x03C9 }, { 0x0401, 0x0401 },
        { 0x0410, 0x044F }, { 0x0451, 0x0451 }, { 0x2010, 0x2010 },
        { 0x2013, 0x2016 }, { 0x2018, 0x2019 }, { 0x201C, 0x201D },
        { 0x2020, 0x2022 }, { 0x2024, 0x2027 }, { 0x2030, 0x2030 },
        { 0x2032, 0x2033 }, { 0x2035, 0x2035 }, { 0x203B, 0x203B },
        { 0x203E, 0x203E }, { 0x2074, 0x2074 }, { 0x207F, 0x207F },
        { 0x2081, 0x2084 }, { 0x20AC, 0x20AC }, { 0x2103, 0x2103 },
        { 0x2105, 0x2105 }, { 0x2109, 0x2109 }, { 0x2113, 0x2113 },
        { 0x2116, 0x2116 }, { 0x2121, 0x2122 }, { 0x2126, 0x2126 },
        { 0x212B, 0x212B }, { 0x2153, 0x2155 }, { 0x215B, 0x215E },
        { 0x2160, 0x216B }, { 0x2170, 0x2179 }, { 0x2190, 0x2199 },
        { 0x21B8, 0x21B9 }, { 0x21D2, 0x21D2 }, { 0x21D4, 0x21D4 },
        { 0x21E7, 0x21E7 }, { 0x2200, 0x2200 }, { 0x2202, 0x2203 },
        { 0x2207, 0x2208 }, { 0x220B, 0x220B }, { 0x220F, 0x220F },
        { 0x2211, 0x2211 }, { 0x2215, 0x2215 }, { 0x221A, 0x221A },
        { 0x221D, 0x2220 }, { 0x2223, 0x2223 }, { 0x2225, 0x2225 },
        { 0x2227, 0x222C }, { 0x222E, 0x222E }, { 0x2234, 0x2237 },
        { 0x223C, 0x223D }, { 0x2248, 0x2248 }, { 0x224C, 0x224C },
        { 0x2252, 0x2252 }, { 0x2260, 0x2261 }, { 0x2264, 0x2267 },
        { 0x226A, 0x226B }, { 0x226E, 0x226F }, { 0x2282, 0x2283 },
        { 0x2286, 0x2287 }, { 0x2295, 0x2295 }, { 0x2299, 0x2299 },
        { 0x22A5, 0x22A5 }, { 0x22BF, 0x22BF }, { 0x2312, 0x2312 },
        { 0x2329, 0x232A }, { 0x2460, 0x24BF }, { 0x24D0, 0x24E9 },
        { 0x2500, 0x254B }, { 0x2550, 0x2574 }, { 0x2580, 0x258F },
        { 0x2592, 0x2595 }, { 0x25A0, 0x25A1 }, { 0x25A3, 0x25A9 },
        { 0x25B2, 0x25B3 }, { 0x25B6, 0x25B7 }, { 0x25BC, 0x25BD },
        { 0x25C0, 0x25C1 }, { 0x25C6, 0x25C8 }, { 0x25CB, 0x25CB },
        { 0x25CE, 0x25D1 }, { 0x25E2, 0x25E5 }, { 0x25EF, 0x25EF },
        { 0x2605, 0x2606 }, { 0x2609, 0x2609 }, { 0x260E, 0x260F },
        { 0x261C, 0x261C }, { 0x261E, 0x261E }, { 0x2640, 0x2640 },
        { 0x2642, 0x2642 }, { 0x2660, 0x2661 }, { 0x2663, 0x2665 },
        { 0x2667, 0x266A }, { 0x266C, 0x266D }, { 0x266F, 0x266F },
        { 0x273D, 0x273D }, { 0x3008, 0x300B }, { 0x3014, 0x3015 },
        { 0x3018, 0x301B }, { 0xFFFD, 0xFFFD }
    };

    /* binary search in table of non-spacing characters */
    if (bisearch(ucs, ambiguous,
                sizeof(ambiguous) / sizeof(struct interval) - 1))
        return 2;

    return wcwidth(ucs);
}
#endif

/* update the given physical line to look like the corresponding line in
   curscr */

void PDC_transform_line(int lineno, int x, int len, const chtype *srcp)
{
    CHAR_INFO ci[512];
    int j;
    COORD bufSize, bufPos;
    SMALL_RECT sr;
#if defined(PDC_FORCE_UTF8) && !defined(PDC_WIDE)
    int off = 0, jx = 0, swidth = 0;
    static int (*f_wcwidth)(wchar_t) = NULL;
    if (f_wcwidth == NULL)
    {
        UINT cp = GetACP();
        if (cp == 932 || cp == 51932 || cp == 936 || cp == 949 || cp == 950)
            f_wcwidth = wcwidth_cjk;
        else
            f_wcwidth = wcwidth;
    }
#endif

    PDC_LOG(("PDC_transform_line() - called: lineno=%d\n", lineno));

    bufPos.X = bufPos.Y = 0;

    bufSize.X = len;
    bufSize.Y = 1;

    sr.Top = lineno;
    sr.Bottom = lineno;
    sr.Left = x;
    sr.Right = x + len - 1;

    for (j = 0; j < len; j++)
    {
        chtype ch = srcp[j];
#if defined(PDC_FORCE_UTF8) && !defined(PDC_WIDE)
        chtype uch = to_wchar(srcp + j, 200, &off);

        ci[jx].Attributes = pdc_atrtab[ch >> PDC_ATTR_SHIFT];

# ifdef CHTYPE_LONG
        if (ch & A_ALTCHARSET && !(ch & 0xff80))
            ch = acs_map[ch & 0x7f];
# endif
        ci[jx].Char.UnicodeChar = uch;
        j += off - 1;
		swidth += f_wcwidth(uch);
        jx++;
#else
        ci[j].Attributes = pdc_atrtab[ch >> PDC_ATTR_SHIFT];
# ifdef CHTYPE_LONG
        if (ch & A_ALTCHARSET && !(ch & 0xff80))
            ch = acs_map[ch & 0x7f];
# endif
        ci[j].Char.UnicodeChar = ch & A_CHARTEXT;
#endif
    }
#if defined(PDC_FORCE_UTF8) && !defined(PDC_WIDE)
    sr.Right = x + swidth - 1;
#endif

    WriteConsoleOutputW(pdc_con_out, ci, bufSize, bufPos, &sr);
}
