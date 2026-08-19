#include <Spark/Serialization/Base64/Base64.h>


namespace spark::serialization
{
static const char* g_Codes = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
static const unsigned char g_Decodes[256] =
{
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
     52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255,
    255, 254, 255, 255, 255,   0,   1,   2,   3,   4,   5,   6,
      7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,
     19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
    255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,
     37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
     49,  50,  51, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255
};

void Base64Encode(const unsigned char* src, int srcLen, unsigned char* dest, int& destLen)
{
    unsigned char* p = dest;
    int remainder = srcLen % 3;
    for (auto i = 0; i < srcLen - remainder; i += 3)
    {
        *p++ = g_Codes[src[i] >> 2];
        *p++ = g_Codes[((src[i] & 3) << 4) + (src[i + 1] >> 4)];
        *p++ = g_Codes[((src[i + 1] & 0xF) << 2) + (src[i + 2] >> 6)];
        *p++ = g_Codes[src[i + 2] & 0x3F];
    }

    if (remainder > 0)
    {
        unsigned char a = 0, b = 0;
        a = src[srcLen - remainder];
        if (remainder > 1)
        {
            b = src[srcLen - remainder + 1];
        }
        *p++ = g_Codes[a >> 2];
        *p++ = g_Codes[((a & 3) << 4) + (b >> 4)];
        if (remainder > 1)
        {
            *p++ = g_Codes[(b & 0xf) << 2];
        }
        else
        {
            *p++ = '=';
        }
        *p++ = '=';
    }
    *p = 0;
    destLen = (int)(p - dest);
}
void Base64Decode(const unsigned char* src, int srcLen, unsigned char* dest, int& destLen)
{
    if (srcLen <= 0)
    {
        destLen = 0;
        return;
    }
    int remainder = srcLen % 4;
    unsigned char* p = dest;
    unsigned char value;
    for (auto i = 0; i < srcLen - remainder; i += 4)
    {
        value = g_Decodes[src[i]] << 2;
        value += g_Decodes[src[i + 1]] >> 4;
        *p++ = value;
        if (src[i + 2] != '=')
        {
            value = g_Decodes[src[i + 1]] << 4;
            value += g_Decodes[src[i + 2]] >> 2;
            *p++ = value;
        }
        if (src[i + 3] != '=')
        {
            value = g_Decodes[src[i + 2]] << 6;
            value += g_Decodes[src[i + 3]];
            *p++ = value;
        }
    }
    *p = '\0';
    destLen = (int)(p - dest);
}
}
