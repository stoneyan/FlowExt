#include "lex.h"
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif
#include <time.h>

int g_tab_spaces = 4;
bool g_log_lex = false;
const std::string LEXER_PARAM_EMPTY = "_P1P1P_";
const std::string LEXER_PARAM_COMMA = "_P1P1P_COMMA";

//#define WRITE_LOGFILE
#define TRACE  if (g_log_lex) printf

uint64_t get_cur_tick()
{
	uint64_t sec, msec;
#ifdef _WIN32
	sec = time(NULL);
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	msec = systime.wMilliseconds;
#else
	struct timeval tval;
	gettimeofday(&tval, NULL);
	sec = tval.tv_sec;
	msec = tval.tv_usec / 1000;
#endif
	return sec * 1000 + msec;
}

char* cur_time()
{
	static char time_buf[100];
	uint64_t cur_tick = get_cur_tick();
	time_t tsec = (time_t)(cur_tick / 1000);
	tm *tp1 = localtime(&tsec);
	sprintf(time_buf, "%4d-%02d-%02d %02d:%02d:%02d.%03d", tp1->tm_year+1900, tp1->tm_mon+1, tp1->tm_mday, tp1->tm_hour, tp1->tm_min, tp1->tm_sec, (int)(cur_tick % 1000));
	return time_buf;
}

void fatal_error(const char* format, ...)
{
    printf("\r\n\r\n");

    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    printf("\r\n\r\n");

    MY_ASSERT(false);
    exit(0);
}

char* ltoa(long v)
{
    static char buf[30];
	if (v < 1024 * 1024 && v > -1024 * 1024)
	    sprintf(buf, "%lu", v);
	else
	    sprintf(buf, "0x%lx", v);
    return buf;
}

std::string ltrim(const std::string &s)
{
    unsigned i;
    for (i = 0; i < s.size(); i++)
    {
        if (!isspace(s[i]))
            break;
    }

    return s.substr(i);
}

// trim from end
std::string rtrim(const std::string &s)
{
    int i;
    for (i = (int)s.size() - 1; i >= 0; i--)
    {
        if (!isspace(s[i]))
            break;
    }

    return s.substr(0, i + 1);
}

// trim from both ends
std::string trim(const std::string &s)
{
    return ltrim(rtrim(s));
}

StringVector str_explode(const std::string& str, const std::string& delim)
{
    std::vector<std::string> ret_v;

    size_t pos = 0;
    while (pos < str.size())
    {
        size_t pos2 = str.find(delim, pos);
        if (pos2 == std::string::npos)
        {
            ret_v.push_back(str.substr(pos));
            break;
        }
        ret_v.push_back(str.substr(pos, pos2 - pos));
        pos = pos2 + delim.size();
    }
    return ret_v;
}

bool file_exists(const char* fname)
{
    FILE* fp = fopen(fname, "rb");
    if (fp != NULL)
    {
        fclose(fp);
        return true;
    }

    return false;
}

const StringVector& get_sys_include_path()
{
	static StringVector s_sys_paths;
#ifdef _WIN32
	if (s_sys_paths.empty())
	{
		char buf[4096];
		DWORD ret = GetEnvironmentVariable("INCLUDE", buf, sizeof(buf));
		if (ret == 0)
			return s_sys_paths;
		std::string tmp_s = buf;
		s_sys_paths = str_explode(tmp_s, ";");
	}
#endif
	return s_sys_paths;
}

FILE* g_fp = NULL;

void write_log(const char* format, ...)
{
#ifdef WRITE_LOGFILE
	if (!g_fp)
		g_fp = fopen("./tmp.log", "wt");

    va_list argList;
    va_start(argList, format);
    vfprintf(g_fp, format, argList);
    vprintf(format, argList);

	va_end(argList);
	//fflush(g_fp);
#else
    va_list argList;
    va_start(argList, format);
    vprintf(format, argList);
#endif
}

CLexer::CLexer()
{
    m_bFileMode = true;
    m_file_line_no = 0;
    m_callback = NULL;
    m_callback_context = NULL;
	m_token_offset = 0;
}

CLexer::~CLexer()
{

}

void CLexer::init()
{
    m_include_path.clear();

    m_include_path.push_back("/usr/include");
#ifdef __linux__
	m_include_path.push_back("/usr/include/x86_64-linux-gnu");
    m_include_path.push_back("/usr/lib/gcc/x86_64-linux-gnu/4.6/include");
    m_include_path.push_back("/usr/include/c++/4.6");
    m_include_path.push_back("/usr/include/c++/4.6/x86_64-linux-gnu");
#else
    m_include_path.push_back("/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1/");
    m_include_path.push_back("/usr/include/c++/4.2.1");
    m_include_path.push_back("/usr/include/c++/4.2.1/tr1");
    m_include_path.push_back("/usr/include/machine");
#endif
    std::string name;
    std::vector<std::string> params;
    std::vector<std::string> keywords;

#define PREDEFINE(n) { name=n; handleDefine(name, false, params, keywords); }
#define PREDEFINE_2(n, v) { name=n; keywords.push_back(v); handleDefine(name, false, params, keywords); keywords.clear(); }
    PREDEFINE_2("__LINE__", "");
    PREDEFINE_2("__FUNCTION__", "");

    PREDEFINE("UNIX");
    PREDEFINE_2("__unix", "1");
    PREDEFINE_2("__unix__", "1");

#ifdef __linux__
    PREDEFINE_2("__gnu_linux__", "1");
    PREDEFINE_2("__linux", "1");
    PREDEFINE_2("__linux__", "1");
#else // macos
    PREDEFINE("__clang__");
#endif
    PREDEFINE("_GNU_SOURCE");
    PREDEFINE_2("NULL", "__null");

    PREDEFINE_2("__PTRDIFF_TYPE__", "long int");

    PREDEFINE_2("__STDC__", "1");
    PREDEFINE_2("__cplusplus", "1");
    PREDEFINE_2("__STDC_HOSTED__", "1");
    PREDEFINE_2("__GNUC__", "4");
    PREDEFINE_2("__GNUC_MINOR__", "6");
    PREDEFINE_2("__GNUC_PATCHLEVEL__", "3");
    PREDEFINE_2("__VERSION__", "\"4.6.3\"");
    PREDEFINE_2("__FINITE_MATH_ONLY__", "0");
    PREDEFINE_2("_LP64", "1");
    PREDEFINE_2("__LP64__", "1");
    PREDEFINE_2("__SIZEOF_INT__", "4");
    PREDEFINE_2("__SIZEOF_LONG__", "8");
    PREDEFINE_2("__SIZEOF_LONG_LONG__", "8");
    PREDEFINE_2("__SIZEOF_SHORT__", "2");
    PREDEFINE_2("__SIZEOF_FLOAT__", "4");
    PREDEFINE_2("__SIZEOF_DOUBLE__", "8");
    PREDEFINE_2("__SIZEOF_LONG_DOUBLE__", "16");
    PREDEFINE_2("__SIZEOF_SIZE_T__", "8");
    PREDEFINE_2("__CHAR_BIT__", "8");
    PREDEFINE_2("__BIGGEST_ALIGNMENT__", "16");
    PREDEFINE_2("__ORDER_LITTLE_ENDIAN__", "1234");
    PREDEFINE_2("__ORDER_BIG_ENDIAN__", "4321");
    PREDEFINE_2("__ORDER_PDP_ENDIAN__", "3412");
    PREDEFINE_2("__BYTE_ORDER__", "__ORDER_LITTLE_ENDIAN__");
    PREDEFINE_2("__FLOAT_WORD_ORDER__", "__ORDER_LITTLE_ENDIAN__");
    PREDEFINE_2("__SIZEOF_POINTER__", "8");
    PREDEFINE_2("__GNUG__", "4");
    PREDEFINE_2("__SIZE_TYPE__", "long unsigned int");
    PREDEFINE_2("__PTRDIFF_TYPE__", "long int");
    PREDEFINE_2("__WCHAR_TYPE__", "int");
    PREDEFINE_2("__WINT_TYPE__", "unsigned int");
    PREDEFINE_2("__INTMAX_TYPE__", "long int");
    PREDEFINE_2("__UINTMAX_TYPE__", "long unsigned int");
    PREDEFINE_2("__CHAR16_TYPE__", "short unsigned int");
    PREDEFINE_2("__CHAR32_TYPE__", "unsigned int");
    PREDEFINE_2("__SIG_ATOMIC_TYPE__", "int");
    PREDEFINE_2("__INT8_TYPE__", "signed char");
    PREDEFINE_2("__INT16_TYPE__", "short int");
    PREDEFINE_2("__INT32_TYPE__", "int");
    PREDEFINE_2("__INT64_TYPE__", "long int");
    PREDEFINE_2("__UINT8_TYPE__", "unsigned char");
    PREDEFINE_2("__UINT16_TYPE__", "short unsigned int");
    PREDEFINE_2("__UINT32_TYPE__", "unsigned int");
    PREDEFINE_2("__UINT64_TYPE__", "long unsigned int");
    PREDEFINE_2("__INT_LEAST8_TYPE__", "signed char");
    PREDEFINE_2("__INT_LEAST16_TYPE__", "short int");
    PREDEFINE_2("__INT_LEAST32_TYPE__", "int");
    PREDEFINE_2("__INT_LEAST64_TYPE__", "long int");
    PREDEFINE_2("__UINT_LEAST8_TYPE__", "unsigned char");
    PREDEFINE_2("__UINT_LEAST16_TYPE__", "short unsigned int");
    PREDEFINE_2("__UINT_LEAST32_TYPE__", "unsigned int");
    PREDEFINE_2("__UINT_LEAST64_TYPE__", "long unsigned int");
    PREDEFINE_2("__INT_FAST8_TYPE__", "signed char");
    PREDEFINE_2("__INT_FAST16_TYPE__", "long int");
    PREDEFINE_2("__INT_FAST32_TYPE__", "long int");
    PREDEFINE_2("__INT_FAST64_TYPE__", "long int");
    PREDEFINE_2("__UINT_FAST8_TYPE__", "unsigned char");
    PREDEFINE_2("__UINT_FAST16_TYPE__", "long unsigned int");
    PREDEFINE_2("__UINT_FAST32_TYPE__", "long unsigned int");
    PREDEFINE_2("__UINT_FAST64_TYPE__", "long unsigned int");
    PREDEFINE_2("__INTPTR_TYPE__", "long int");
    PREDEFINE_2("__UINTPTR_TYPE__", "long unsigned int");
    PREDEFINE_2("__GXX_WEAK__", "1");
    PREDEFINE_2("__DEPRECATED", "1");
    PREDEFINE_2("__GXX_RTTI", "1");
    PREDEFINE_2("__EXCEPTIONS", "1");
    PREDEFINE_2("__GXX_ABI_VERSION", "1002");
    PREDEFINE_2("__SCHAR_MAX__", "127");
    PREDEFINE_2("__SHRT_MAX__", "32767");
    PREDEFINE_2("__INT_MAX__", "2147483647");
    PREDEFINE_2("__LONG_MAX__", "9223372036854775807L");
    PREDEFINE_2("__LONG_LONG_MAX__", "9223372036854775807LL");
    PREDEFINE_2("__WCHAR_MAX__", "2147483647");
    PREDEFINE_2("__WCHAR_MIN__", "(-__WCHAR_MAX__ - 1)");
    PREDEFINE_2("__WINT_MAX__", "4294967295U");
    PREDEFINE_2("__WINT_MIN__", "0U");
    PREDEFINE_2("__PTRDIFF_MAX__", "9223372036854775807L");
    PREDEFINE_2("__SIZE_MAX__", "18446744073709551615UL");
    PREDEFINE_2("__INTMAX_MAX__", "9223372036854775807L");
    PREDEFINE_2("__INTMAX_C(c)", "c ## L");
    PREDEFINE_2("__UINTMAX_MAX__", "18446744073709551615UL");
    PREDEFINE_2("__UINTMAX_C(c)", "c ## UL");
    PREDEFINE_2("__SIG_ATOMIC_MAX__", "2147483647");
    PREDEFINE_2("__SIG_ATOMIC_MIN__", "(-__SIG_ATOMIC_MAX__ - 1)");
    PREDEFINE_2("__INT8_MAX__", "127");
    PREDEFINE_2("__INT16_MAX__", "32767");
    PREDEFINE_2("__INT32_MAX__", "2147483647");
    PREDEFINE_2("__INT64_MAX__", "9223372036854775807L");
    PREDEFINE_2("__UINT8_MAX__", "255");
    PREDEFINE_2("__UINT16_MAX__", "65535");
    PREDEFINE_2("__UINT32_MAX__", "4294967295U");
    PREDEFINE_2("__UINT64_MAX__", "18446744073709551615UL");
    PREDEFINE_2("__INT_LEAST8_MAX__", "127");
    PREDEFINE_2("__INT8_C(c)", "c");
    PREDEFINE_2("__INT_LEAST16_MAX__", "32767");
    PREDEFINE_2("__INT16_C(c)", "c");
    PREDEFINE_2("__INT_LEAST32_MAX__", "2147483647");
    PREDEFINE_2("__INT32_C(c)", "c");
    PREDEFINE_2("__INT_LEAST64_MAX__", "9223372036854775807L");
    PREDEFINE_2("__INT64_C(c)", "c ## L");
    PREDEFINE_2("__UINT_LEAST8_MAX__", "255");
    PREDEFINE_2("__UINT8_C(c)", "c");
    PREDEFINE_2("__UINT_LEAST16_MAX__", "65535");
    PREDEFINE_2("__UINT16_C(c)", "c");
    PREDEFINE_2("__UINT_LEAST32_MAX__", "4294967295U");
    PREDEFINE_2("__UINT32_C(c)", "c ## U");
    PREDEFINE_2("__UINT_LEAST64_MAX__", "18446744073709551615UL");
    PREDEFINE_2("__UINT64_C(c)", "c ## UL");
    PREDEFINE_2("__INT_FAST8_MAX__", "127");
    PREDEFINE_2("__INT_FAST16_MAX__", "9223372036854775807L");
    PREDEFINE_2("__INT_FAST32_MAX__", "9223372036854775807L");
    PREDEFINE_2("__INT_FAST64_MAX__", "9223372036854775807L");
    PREDEFINE_2("__UINT_FAST8_MAX__", "255");
    PREDEFINE_2("__UINT_FAST16_MAX__", "18446744073709551615UL");
    PREDEFINE_2("__UINT_FAST32_MAX__", "18446744073709551615UL");
    PREDEFINE_2("__UINT_FAST64_MAX__", "18446744073709551615UL");
    PREDEFINE_2("__INTPTR_MAX__", "9223372036854775807L");
    PREDEFINE_2("__UINTPTR_MAX__", "18446744073709551615UL");
    PREDEFINE_2("__FLT_EVAL_METHOD__", "0");
    PREDEFINE_2("__DEC_EVAL_METHOD__", "2");
    PREDEFINE_2("__FLT_RADIX__", "2");
    PREDEFINE_2("__FLT_MANT_DIG__", "24");
    PREDEFINE_2("__FLT_DIG__", "6");
    PREDEFINE_2("__FLT_MIN_EXP__", "(-125)");
    PREDEFINE_2("__FLT_MIN_10_EXP__", "(-37)");
    PREDEFINE_2("__FLT_MAX_EXP__", "128");
    PREDEFINE_2("__FLT_MAX_10_EXP__", "38");
    PREDEFINE_2("__FLT_DECIMAL_DIG__", "9");
    PREDEFINE_2("__FLT_MAX__", "3.40282346638528859812e+38F");
    PREDEFINE_2("__FLT_MIN__", "1.17549435082228750797e-38F");
    PREDEFINE_2("__FLT_EPSILON__", "1.19209289550781250000e-7F");
    PREDEFINE_2("__FLT_DENORM_MIN__", "1.40129846432481707092e-45F");
    PREDEFINE_2("__FLT_HAS_DENORM__", "1");
    PREDEFINE_2("__FLT_HAS_INFINITY__", "1");
    PREDEFINE_2("__FLT_HAS_QUIET_NAN__", "1");
    PREDEFINE_2("__DBL_MANT_DIG__", "53");
    PREDEFINE_2("__DBL_DIG__", "15");
    PREDEFINE_2("__DBL_MIN_EXP__", "(-1021)");
    PREDEFINE_2("__DBL_MIN_10_EXP__", "(-307)");
    PREDEFINE_2("__DBL_MAX_EXP__", "1024");
    PREDEFINE_2("__DBL_MAX_10_EXP__", "308");
    PREDEFINE_2("__DBL_DECIMAL_DIG__", "17");
    PREDEFINE_2("__DBL_MAX__", "double(1.79769313486231570815e+308L)");
    PREDEFINE_2("__DBL_MIN__", "double(2.22507385850720138309e-308L)");
    PREDEFINE_2("__DBL_EPSILON__", "double(2.22044604925031308085e-16L)");
    PREDEFINE_2("__DBL_DENORM_MIN__", "double(4.94065645841246544177e-324L)");
    PREDEFINE_2("__DBL_HAS_DENORM__", "1");
    PREDEFINE_2("__DBL_HAS_INFINITY__", "1");
    PREDEFINE_2("__DBL_HAS_QUIET_NAN__", "1");
    PREDEFINE_2("__LDBL_MANT_DIG__", "64");
    PREDEFINE_2("__LDBL_DIG__", "18");
    PREDEFINE_2("__LDBL_MIN_EXP__", "(-16381)");
    PREDEFINE_2("__LDBL_MIN_10_EXP__", "(-4931)");
    PREDEFINE_2("__LDBL_MAX_EXP__", "16384");
    PREDEFINE_2("__LDBL_MAX_10_EXP__", "4932");
    PREDEFINE_2("__DECIMAL_DIG__", "21");
    PREDEFINE_2("__LDBL_MAX__", "1.18973149535723176502e+4932L");
    PREDEFINE_2("__LDBL_MIN__", "3.36210314311209350626e-4932L");
    PREDEFINE_2("__LDBL_EPSILON__", "1.08420217248550443401e-19L");
    PREDEFINE_2("__LDBL_DENORM_MIN__", "3.64519953188247460253e-4951L");
    PREDEFINE_2("__LDBL_HAS_DENORM__", "1");
    PREDEFINE_2("__LDBL_HAS_INFINITY__", "1");
    PREDEFINE_2("__LDBL_HAS_QUIET_NAN__", "1");
    PREDEFINE_2("__DEC32_MANT_DIG__", "7");
    PREDEFINE_2("__DEC32_MIN_EXP__", "(-94)");
    PREDEFINE_2("__DEC32_MAX_EXP__", "97");
    PREDEFINE_2("__DEC32_MIN__", "1E-95DF");
    PREDEFINE_2("__DEC32_MAX__", "9.999999E96DF");
    PREDEFINE_2("__DEC32_EPSILON__", "1E-6DF");
    PREDEFINE_2("__DEC32_SUBNORMAL_MIN__", "0.000001E-95DF");
    PREDEFINE_2("__DEC64_MANT_DIG__", "16");
    PREDEFINE_2("__DEC64_MIN_EXP__", "(-382)");
    PREDEFINE_2("__DEC64_MAX_EXP__", "385");
    PREDEFINE_2("__DEC64_MIN__", "1E-383DD");
    PREDEFINE_2("__DEC64_MAX__", "9.999999999999999E384DD");
    PREDEFINE_2("__DEC64_EPSILON__", "1E-15DD");
    PREDEFINE_2("__DEC64_SUBNORMAL_MIN__", "0.000000000000001E-383DD");
    PREDEFINE_2("__DEC128_MANT_DIG__", "34");
    PREDEFINE_2("__DEC128_MIN_EXP__", "(-6142)");
    PREDEFINE_2("__DEC128_MAX_EXP__", "6145");
    PREDEFINE_2("__DEC128_MIN__", "1E-6143DL");
    PREDEFINE_2("__DEC128_MAX__", "9.999999999999999999999999999999999E6144DL");
    PREDEFINE_2("__DEC128_EPSILON__", "1E-33DL");
    PREDEFINE_2("__DEC128_SUBNORMAL_MIN__", "0.000000000000000000000000000000001E-6143DL");
    PREDEFINE("__REGISTER_PREFIX__");
    PREDEFINE("__USER_LABEL_PREFIX__");
    PREDEFINE_2("_FORTIFY_SOURCE", "2");
    PREDEFINE_2("__GNUC_GNU_INLINE__", "1");
    PREDEFINE_2("__NO_INLINE__", "1");
    PREDEFINE_2("__STRICT_ANSI__", "1");
    PREDEFINE_2("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1", "1");
    PREDEFINE_2("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2", "1");
    PREDEFINE_2("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4", "1");
    PREDEFINE_2("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8", "1");
    PREDEFINE_2("__GCC_HAVE_DWARF2_CFI_ASM", "1");
    PREDEFINE_2("__PRAGMA_REDEFINE_EXTNAME", "1");
    PREDEFINE_2("__SSP__", "1");
    PREDEFINE_2("__SIZEOF_INT128__", "16");
    PREDEFINE_2("__SIZEOF_WCHAR_T__", "4");
    PREDEFINE_2("__SIZEOF_WINT_T__", "4");
    PREDEFINE_2("__SIZEOF_PTRDIFF_T__", "8");
    PREDEFINE_2("__amd64", "1");
    PREDEFINE_2("__amd64__", "1");
    PREDEFINE_2("__x86_64", "1");
    PREDEFINE_2("__x86_64__", "1");
    PREDEFINE_2("__k8", "1");
    PREDEFINE_2("__k8__", "1");
    PREDEFINE_2("__MMX__", "1");
    PREDEFINE_2("__SSE__", "1");
    PREDEFINE_2("__SSE2__", "1");
    PREDEFINE_2("__SSE_MATH__", "1");
    PREDEFINE_2("__SSE2_MATH__", "1");
    PREDEFINE_2("__ELF__", "1");
    PREDEFINE_2("__DECIMAL_BID_FORMAT__", "1");

#undef PREDEFINE
}

char* CLexer::readContentFromFile(const char* file_name)
{
    FILE* fp = fopen(file_name, "rb");
    if (fp == NULL)
        return NULL;

    fseek(fp, 0, SEEK_END);
    int fsize = ftell(fp);
    char* file_content = (char*)malloc(fsize + 1);
    fseek(fp, 0, SEEK_SET);
    fread(file_content, 1, fsize, fp);
    fclose(fp);

    file_content[fsize] = 0;
    return file_content;
}

long CLexer::calcValue(const char* str)
{
    int n = 0;
    int base = 10;

    if (str[0] == '\'')
	{
		if (str[1] == '\\')
		{
			switch (str[2])
			{
			case '0':
				return '\0';
			case 't':
				return '\t';
			case 'r':
				return '\r';
			case 'n':
				return '\n';
			case '\'':
				return '\'';
			case '\\':
				return '\\';
			default:
				MY_ASSERT(false);
			}
		}
		MY_ASSERT(str[2] == '\'');
		return str[1];
	}

    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        str += 2;
        base = 16;
    }
    while (*str)
    {
        int m = 0;
        if (base == 10)
        {
            if (*str >= '0' && *str <= '9')
                m = *str - '0';
            else if (*str == 'L' || *str == 'U')
            {
                if (str[1] != 0)
                    fatal_error("no value expected after L in %s", str);
                break;
            }
            else
                fatal_error("Cannot calculate '%s'", str);
        }
        else
        {
            if (*str >= '0' && *str <= '9')
                m = *str - '0';
            else if (*str >= 'A' && *str <= 'F')
                m = *str - 'A' + 10;
            else if (*str >= 'a' && *str <= 'f')
                m = *str - 'a' + 10;
            else if (*str == 'U' || *str == 'L')
            {
            }
            else
                fatal_error("Unrecognized value string %s", str);
        }
        n = n * base + m;
        str++;
    }

    return n;
}

std::string CLexer::combine_tokens(StringVector::iterator begin, StringVector::iterator end)
{
    std::string ret_s;
    int last_type = 0;

    for (StringVector::iterator it = begin; it != end; it++)
    {
        char c = (*it)[0];
        int type = (isalnum(c) ? 1 : 2);
        if (last_type == 1 && type == 1)
            ret_s += " ";
        ret_s += *it;
        last_type = type;
    }

    return ret_s;
}

bool CLexer::is_empty()
{
    if (m_bFileMode)
    {
        return (m_file_list.size() == 1 && m_file_list[0].extra_data.empty() && !*m_file_list[0].content_cur);
    }

    return m_token_offset >= (int)m_tokens.size();
}

bool CLexer::isSymbol(char c)
{
    return (!isalnum(c) && !isspace(c));
}

bool CLexer::isChar(const std::string& str)
{
	return str[0] == '\'';
}

bool CLexer::isNumber(const std::string& str)
{
    char c = str.at(0);
    if (isdigit(c))
        return true;

    if (c == '-' && str.size() > 1 && isdigit(str.at(1)))
        return true;

    return false;
}

bool CLexer::isString(const std::string& str) // "..." or L"..."
{
	return str.at(0) == '"' || str.at(0) == 'L' && str.at(1) == '"';
}

bool CLexer::isIdentifier(const std::string& str)
{
    char c = str.at(0);
    return (c == '_' || isalpha(c));
}

bool CLexer::isCommentWord(const std::string& s)
{
    return s.size() > 2 && s.substr(0, 2) == "//";
}

std::string CLexer::get_cur_filename()
{
    if (m_bFileMode)
    {
        MY_ASSERT(m_file_list.size() > 0);
		MY_ASSERT(m_file_list.back().file_name.size() > 2);
        return m_file_list.back().file_name;
    }

    MY_ASSERT(m_file_stack.size() > 0);
	MY_ASSERT(m_file_stack.back().size() > 2);
    return m_file_stack.back();
}

int CLexer::get_cur_line_no()
{
    if (m_bFileMode)
    {
        MY_ASSERT(m_file_list.size() > 0);
        return m_file_list.back().row;
    }

    return m_file_line_no;
}

StringVector CLexer::get_file_stack()
{
	StringVector ret_v;
	if (m_bFileMode)
	{
		for (std::vector<SourceFile>::iterator it = m_file_list.begin(); it != m_file_list.end(); it++)
		{
			//size_t pos = it->file_name.find('-');
			//MY_ASSERT(pos == std::string::npos);
			//ret_v.push_back(it->file_name.substr(0, pos));
			ret_v.push_back(it->file_name);
		}
	}
	else
	{
		for (StringVector::iterator it = m_file_stack.begin(); it != m_file_stack.end(); it++)
		{
			//size_t pos = it->find('-');
			//MY_ASSERT(pos == std::string::npos);
			//ret_v.push_back(it->substr(0, pos));
			ret_v.push_back(*it);
		}
	}
	return ret_v;
}

std::string CLexer::file_stack_2_string(const StringVector& file_stack, int line_no)
{
    std::string s = "//";
    for (int i = 0; i < (int)file_stack.size(); i++)
    {
        if (i > 0)
            s += "|";

        s += file_stack[i];
    }
    return s + std::string("@") + ltoa(line_no);
}

void CLexer::string_2_file_stack(const std::string& s, StringVector& file_stack, int& line_no)
{
    std::string s2 = s.substr(2);
    size_t pos = s2.find('@');
    MY_ASSERT(pos != std::string::npos);
    line_no = atoi(s2.c_str() + pos + 1);
    s2.resize(pos);
    file_stack.clear();
    while (true)
    {
        pos = s2.find('|');
        if (pos == std::string::npos)
        {
			MY_ASSERT(s2.size() > 2);
            file_stack.push_back(s2);
            break;
        }
		MY_ASSERT(pos > 2);
        file_stack.push_back(s2.substr(0, pos));
        s2 = s2.substr(pos + 1);
    }
}

char CLexer::peek_char(int offset)
{
    SourceFile& sf = m_file_list.back();

    if (!sf.extra_data.empty())
        return sf.extra_data[offset];

    return sf.content_cur[offset];
}

char CLexer::get_char()
{
    SourceFile& sf = m_file_list.back();

    char c;

    if (!sf.extra_data.empty())
    {
        c = sf.extra_data[0];
        sf.extra_data = sf.extra_data.substr(1);
    }
    else
    {
        c = *sf.content_cur;
        if (c)
            sf.content_cur++;
    }
    return c;
}

bool CLexer::read_char(char& c, bool bCrossNewLine)
{
    SourceFile& sf = m_file_list.back();

    while (true)
    {
        c = peek_char();
        if (c == 0)
            return false;

        if (c == '\t')
        {
            get_char();
            sf.col += g_tab_spaces - 1;
            sf.col -= sf.col % g_tab_spaces;
        }
        else if (c == '\r')
        {
            get_char();
        }
        else if (c == '\n')
        {
            if (!bCrossNewLine)
                return false;
            get_char();
            sf.row++;
            sf.col = 1;
        }
        else if (c == '\\')
        {
            get_char();
            sf.col++;
            char c2 = peek_char();
            if (c2 == ' ' || c2 == '\t' || c2 == '\r' || c2 == '\n')
            {
                c2 = get_char();
                while (c2 == ' ' || c2 == '\t' || c2 == '\r')
                {
                    c2 = get_char();
                }
                if (c2 == 0)
                    fatal_error("unexpected EOF after \\ at %s:%d", sf.file_name.c_str(), sf.row);
                if (c2 != '\n')
                    fatal_error("unexpected character after \\ at %s:%d", sf.file_name.c_str(), sf.row);
                sf.row++;
                sf.col = 1;
                continue;
            }
        }
        else
        {
            get_char();
            sf.col++;
        }

        return true;
    }
}

// read a word, handles "", '', /*..*/, //
std::string CLexer::read_word2(bool bCrossNewLine)
{
    std::string ret_s;
    char c;
    while (true)
    {
        do
        {
            if (!read_char(c, bCrossNewLine))
                return ret_s;
        } while (isspace(c));

		if (c == 'L')
        {
            if (peek_char() == '"' || peek_char() == '\'')
			{
				ret_s += c;
                read_char(c);
			}
		}
        if (c == '"' || c == '\'')
        {
            int start_line = get_cur_line_no();
            ret_s += c;
            while (true)
            {
                char c2;
                if (!read_char(c2))
                    fatal_error("an open string is found at %s:%d", get_cur_filename().c_str(), start_line);
                ret_s += c2;
                if (c2 == c)
                    break;
                if (c2 == '\\')
                {
                    if (!read_char(c2))
                        fatal_error("EOF found in the middle of a string after \\ at %s", get_cur_filename().c_str());
                    ret_s += c2;
                }
            }
            break;
        }
        if (c == '/')
        {
            if (peek_char() == '/')
            {
                read_char(c);
                while (true)
                {
                    c = peek_char();
                    if (c == 0)
                        return "";
                    if (c == '\n')
                    {
                        if (!bCrossNewLine)
                            return "";
                        read_char(c);
                        break;
                    }
                    read_char(c);
                }
                continue;
            }
            if (peek_char() == '*')
            {
                read_char(c);
                while (true)
                {
                    if (!read_char(c))
                        fatal_error("EOF found in the middle of a comment in file <%s>", get_cur_filename().c_str());
                    if (c == '*' && !peek_char())
                        fatal_error("EOF found in the middle of a comment in file <%s>", get_cur_filename().c_str());
                    if (c == '*' && peek_char() == '/')
                    {
                        read_char(c);
                        break;
                    }
                }
                continue;
            }
        }
        if (isalpha(c) || c == '_')
        {
            ret_s += c;
            while (isalnum(peek_char()) || peek_char() == '_' || peek_char() == '$')
            {
                read_char(c);
                ret_s += c;
            }
            break;
        }
        if (isdigit(c))
        {
            ret_s += c;
            while (isdigit(peek_char()) || peek_char() == '.' || isalpha(peek_char())) // x o E l f
            {
                read_char(c);
                ret_s += c;
            }
            break;
        }
        // ++ += -- -= -> << >> <= >= <<= >>= == |= %= || &= && != *= /= ^= ... :: ##
        ret_s += c;
        if (c == '+')
        {
            c = peek_char();
            if (c == '+' || c == '=')
            {
                read_char(c);
                ret_s += c;
            }
            break;
        }
        if (c == '-')
        {
            c = peek_char();
            if (c == '-' || c == '=' || c == '>')
            {
                read_char(c);
                ret_s += c;
            }
            break;
        }
        if (c == '<')
        {
            c = peek_char();
            if (c == '<' || c == '=')
            {
                read_char(c);
                ret_s += c;
                if (c == '<' && peek_char() == '=')
                {
                    read_char(c);
                    ret_s += c;
                }
            }
            break;
        }
        if (c == '>')
        {
            c = peek_char();
            if (c == '>' || c == '=')
            {
                read_char(c);
                ret_s += c;
                if (c == '>' && peek_char() == '=')
                {
                    read_char(c);
                    ret_s += c;
                }
            }
            break;
        }
        if (c == '|')
        {
            c = peek_char();
            if (c == '|' || c == '=')
            {
                read_char(c);
                ret_s += c;
            }
            break;
        }
        if (c == '&')
        {
            c = peek_char();
            if (c == '&' || c == '=')
            {
                read_char(c);
                ret_s += c;
            }
            break;
        }
        if (c == '=')
        {
            if (peek_char() == '=')
            {
                read_char(c);
                ret_s += c;
            }
            break;
        }
        if (c == '!')
        {
            if (peek_char() == '=')
            {
                read_char(c);
                ret_s += c;
            }
            break;
        }
        if (c == '*')
        {
            if (peek_char() == '=')
            {
                read_char(c);
                ret_s += c;
            }
            break;
        }
        if (c == '/')
        {
            if (peek_char() == '=')
            {
                read_char(c);
                ret_s += c;
            }
            break;
        }
        if (c == '%')
        {
            if (peek_char() == '=')
            {
                read_char(c);
                ret_s += c;
            }
            break;
        }
        if (c == '^')
        {
            if (peek_char() == '=')
            {
                read_char(c);
                ret_s += c;
            }
            break;
        }
        if (c == ':')
        {
            if (peek_char() == ':')
            {
                read_char(c);
                ret_s += c;
            }
            break;
        }
        if (c == '.' && peek_char() == '.' && peek_char(1) == '.')
        {
            read_char(c);
            ret_s += c;
            read_char(c);
            ret_s += c;
            break;
        }
        if (c == '#')
        {
            if (peek_char() == '#')
            {
                read_char(c);
                ret_s += c;
            }
            break;
        }
        break;
    }

//printf("read_word2 returns %s\n", ret_s.c_str());
    return ret_s;
}

bool CLexer::isDefined(const std::string& s)
{
    return m_define_map.find(s) != m_define_map.end();
}

bool CLexer::getDefineItem(const std::string& s, DefineItem& item)
{
    std::map<std::string, DefineItem>::iterator it = m_define_map.find(s);
    if (it == m_define_map.end())
        return false;

    item = it->second;
    return true;
}

bool CLexer::isDefineNumber(DefineItem& item, std::string& n)
{
    if (item.params.size() != 0 || item.value_keywords.size() != 1)
    {
        //printf("\nps=%ld, vs=%ld\n", item.params.size(), item.value_keywords.size());
        return false;
    }
    std::string& s = item.value_keywords[0];
    if (!isNumber(s))
    {
        //printf("\n%s is not a number\n", s.c_str());
        return false;
    }
    n = s;
    return true;
}

// return error. empty means succeed
std::string CLexer::handleDefine(const std::string& name, bool bHasParentheses, const StringVector& params, const StringVector& keywords)
{
    std::string err_s;
    DefineItem item;
    if (getDefineItem(name, item))
    {
        if (item.bHasParentheses == bHasParentheses && item.params == params && item.value_keywords == keywords)
        {
            //TRACE("Macro %s is already defined with the same value\n", name.c_str());
        }
        else
        {
            err_s = name + " is already defined with different value. ";
            if (item.bHasParentheses != bHasParentheses)
            {
                err_s += std::string("prev bHasParentheses=") + (item.bHasParentheses ? "true" : "false") + ", new value=" + (bHasParentheses ? "true" : "false") + ", ";
            }
            if (item.params != params)
            {
                err_s += "prev params={";
                for (size_t i = 0; i < item.params.size(); i++)
                    err_s += item.params[i] + ",";
                err_s += "}, new params={";
                for (size_t i = 0; i < params.size(); i++)
                    err_s += params[i] + ",";
                err_s += "} ";
            }
            if (item.value_keywords != keywords)
            {
                err_s += "prev body={";
                for (size_t i = 0; i < item.value_keywords.size(); i++)
                    err_s += item.value_keywords[i] + " ";
                err_s += "}, new body={";
                for (size_t i = 0; i < keywords.size(); i++)
                    err_s += keywords[i] + " ";
                err_s += "} ";
            }
        }
        return err_s;
    }

    item.bHasParentheses = bHasParentheses;
    item.params = params;
    item.value_keywords = keywords;
    if (item.value_keywords.size() == 1)
    {
        if (item.value_keywords[0].empty())
            item.value_keywords[0] = LEXER_PARAM_EMPTY;
        else if (item.value_keywords[0] == ",")
            item.value_keywords[0] = LEXER_PARAM_COMMA;
    }

    if (g_log_lex)
    {
        printf("#defining %s", name.c_str());
        if (item.params.size() > 0)
        {
            printf("(");
            for (size_t i = 0; i < item.params.size(); i++)
            {
                if (i > 0)
                    printf(",");
                printf("%s", item.params[i].c_str());
            }
            printf(")");
        }
        if (item.value_keywords.size() > 0)
        {
            printf("=");
            for (size_t i = 0; i < item.value_keywords.size(); i++)
            {
                if (i > 0)
                    printf(" ");
                printf("%s", item.value_keywords[i].c_str());
            }
        }
        printf("\n");
    }
    m_define_map[name] = item;
    return err_s;
}

void CLexer::handleUndefine(const std::string& name)
{
    m_define_map.erase(name);
}

std::string CLexer::printDirectiveTree(DirectiveTreeNode* root)
{
    if (root == NULL)
        return "";

    std::string s = "{" + printDirectiveTree(root->left) + " " + root->value + " " + printDirectiveTree(root->right) + "}";
    return s;
}

void CLexer::deleteDirectiveTree(DirectiveTreeNode* root)
{
    if (root == NULL)
        return;

    deleteDirectiveTree(root->left);
    deleteDirectiveTree(root->right);
    deleteDirectiveTree(root->third);
}

void CLexer::transformDirectiveListToTree2(std::vector<DirectiveTreeNode*>& l)
{
    // check parenthesis first
    for (unsigned i = 0; i < l.size(); i++)
    {
        if (l[i]->value == ")")
            throw("unpaired ')' found in directive");

        if (l[i]->value != "(")
            continue;

        std::vector<DirectiveTreeNode*> l2;
        int depth = 0;
        while (true)
        {
            if (i + 1 >= l.size())
                throw("cannot find matching ')' in this directive");
            if (l[i + 1]->value == "(")
                depth++;
            else if (l[i + 1]->value == ")")
            {
                if (depth == 0)
                {
                    l.erase(l.begin() + i + 1);
                    break;
                }
                depth--;
            }
            l2.push_back(l[i + 1]);
            l.erase(l.begin() + i + 1);
        }
        transformDirectiveListToTree2(l2);
        MY_ASSERT(l2.size() == 1);
        l[i]->right = l2[0];
        l[i]->bOperator = false;
    }

    //priority:  1: ! ~, 2: * / %, 3: + - 4: << >>, 5: >, >=, <, <=, 6: ==, !=, 7: &, 8: ^, 9: |, 10: &&, 11: ||, 12: ?:
    int priority = 1;
    while (l.size() > 1)
    {
        bool bProgress = false;
        for (size_t i = 0; i < l.size(); i++)
        {
            if (!l[i]->bOperator)
                continue;

            std::string s = l[i]->value;
            if (s == "(")
            {
                MY_ASSERT(false);
            }
            if (s == "L") // L'\0'
            {
                if (i + 1 >= l.size())
                    throw("expecting identifier after 'L'");
                if (l[i + 1]->value[0] != '\'')
                    throw("expecting a char value after 'L'");

                s = l[i + 1]->value;
                s = s.substr(1, s.size() - 2);
                char c;
                if (sscanf(s.c_str(), "%c", &c) != 1)
                    fatal_error("cannot resovle %s as a number\n", s.c_str());
                l[i]->value = ltoa(c);
                l[i]->bOperator = false;
                l.erase(l.begin() + i + 1);
                bProgress = true;
                continue;
            }
            if (priority >= 1 && (s == "!" || s == "~"))
            {
                if (i + 1 >= l.size())
                    throw("Identifier expected after " + s);
                if (l[i + 1]->bOperator)
                    continue;
                l[i]->right = l[i + 1];
                l[i]->bOperator = false;
                l.erase(l.begin() + i + 1);
                bProgress = true;
                continue;
            }
            if (priority >= 2 && (s == "*" || s == "/" || s == "%"))
            {
                if (i == 0)
                    throw("Identifier expected before " + s);
                if (i + 1 >= l.size())
                    throw("Identifier expected after " + s);

                if (l[i - 1]->bOperator || l[i + 1]->bOperator)
                    continue;
                l[i]->left = l[i - 1];
                l[i]->right = l[i + 1];
                l[i]->bOperator = false;
                l[i - 1] = l[i];
                l.erase(l.begin() + i + 1);
                l.erase(l.begin() + i);
                i--;
                bProgress = true;
                continue;
            }
            if (priority >= 3 && (s == "+" || s == "-"))
            {
                if (i + 1 >= l.size())
                    throw("Identifier expected after " + s);
                if (l[i + 1]->bOperator)
                    continue;
                if (i == 0 || l[i - 1]->bOperator)
                {
                    l[i]->right = l[i + 1];
                    l.erase(l.begin() + i + 1);
                }
                else
                {
                    l[i]->left = l[i - 1];
                    l[i]->right = l[i + 1];
                    l[i - 1] = l[i];
                    l.erase(l.begin() + i + 1);
                    l.erase(l.begin() + i);
                    i--;
                }
                l[i]->bOperator = false;
                bProgress = true;
                continue;
            }
            if (priority >= 4 && (s == "<<" || s == ">>"))
            {
                if (i == 0)
                    throw("Identifier expected before " + s);
                if (i + 1 >= l.size())
                    throw("Identifier expected after " + s);

                if (l[i - 1]->bOperator || l[i + 1]->bOperator)
                    continue;
                l[i]->left = l[i - 1];
                l[i]->right = l[i + 1];
                l[i]->bOperator = false;
                l[i - 1] = l[i];
                l.erase(l.begin() + i + 1);
                l.erase(l.begin() + i);
                i--;
                bProgress = true;
                continue;
            }
            if (priority >= 5 && (s == "<" || s == "<=" || s == ">" || s == ">=" || s == "==" || s == "!="))
            {
                if (i == 0)
                    throw("Identifier expected before " + s);
                if (i + 1 >= l.size())
                    throw("Identifier expected after " + s);

                if (l[i - 1]->bOperator || l[i + 1]->bOperator)
                    continue;
                l[i]->left = l[i - 1];
                l[i]->right = l[i + 1];
                l[i]->bOperator = false;
                l[i - 1] = l[i];
                l.erase(l.begin() + i + 1);
                l.erase(l.begin() + i);
                i--;
                bProgress = true;
                continue;
            }
            if (priority >= 6 && (s == "==" || s == "!="))
            {
                if (i == 0)
                    throw("Identifier expected before " + s);
                if (i + 1 >= l.size())
                    throw("Identifier expected after " + s);
                if (l[i - 1]->bOperator || l[i + 1]->bOperator)
                    continue;
                l[i]->left = l[i - 1];
                l[i]->right = l[i + 1];
                l[i]->bOperator = false;
                l[i - 1] = l[i];
                l.erase(l.begin() + i + 1);
                l.erase(l.begin() + i);
                i--;
                bProgress = true;
                continue;
            }
            if (priority >= 7 && (s == "&"))
            {
                if (i == 0)
                    throw("Identifier expected before " + s);
                if (i + 1 >= l.size())
                    throw("Identifier expected after " + s);
                if (l[i - 1]->bOperator || l[i + 1]->bOperator)
                    continue;
                l[i]->left = l[i - 1];
                l[i]->right = l[i + 1];
                l[i]->bOperator = false;
                l[i - 1] = l[i];
                l.erase(l.begin() + i + 1);
                l.erase(l.begin() + i);
                i--;
                bProgress = true;
                continue;
            }
            if (priority >= 8 && (s == "^"))
            {
                if (i == 0)
                    throw("Identifier expected before " + s);
                if (i + 1 >= l.size())
                    throw("Identifier expected after " + s);
                if (l[i - 1]->bOperator || l[i + 1]->bOperator)
                    continue;
                l[i]->left = l[i - 1];
                l[i]->right = l[i + 1];
                l[i]->bOperator = false;
                l[i - 1] = l[i];
                l.erase(l.begin() + i + 1);
                l.erase(l.begin() + i);
                i--;
                bProgress = true;
                continue;
            }
            if (priority >= 9 && (s == "|"))
            {
                if (i == 0)
                    throw("Identifier expected before " + s);
                if (i + 1 >= l.size())
                    throw("Identifier expected after " + s);
                if (l[i - 1]->bOperator || l[i + 1]->bOperator)
                    continue;
                l[i]->left = l[i - 1];
                l[i]->right = l[i + 1];
                l[i]->bOperator = false;
                l[i - 1] = l[i];
                l.erase(l.begin() + i + 1);
                l.erase(l.begin() + i);
                i--;
                bProgress = true;
                continue;
            }
            if (priority >= 10 && (s == "&&"))
            {
                if (i == 0)
                    throw("Identifier expected before " + s);
                if (i + 1 >= l.size())
                    throw("Identifier expected after " + s);
                if (l[i - 1]->bOperator || l[i + 1]->bOperator)
                    continue;
                l[i]->left = l[i - 1];
                l[i]->right = l[i + 1];
                l[i]->bOperator = false;
                l[i - 1] = l[i];
                l.erase(l.begin() + i + 1);
                l.erase(l.begin() + i);
                i--;
                bProgress = true;
                continue;
            }
            if (priority >= 11 && (s == "||"))
            {
                if (i == 0)
                    throw("Identifier expected before " + s);
                if (i + 1 >= l.size())
                    throw("Identifier expected after " + s);
                if (l[i - 1]->bOperator || l[i + 1]->bOperator)
                    continue;
                l[i]->left = l[i - 1];
                l[i]->right = l[i + 1];
                l[i]->bOperator = false;
                l[i - 1] = l[i];
                l.erase(l.begin() + i + 1);
                l.erase(l.begin() + i);
                i--;
                bProgress = true;
                continue;
            }
            if (priority >= 12 && (s == "?"))
            {
                if (i == 0)
                    throw("Identifier expected before " + s);
                if (i + 3 >= l.size())
                    throw("Identifier expected after " + s);
                if (l[i + 2]->value != ":")
                    throw("':' is expected after " + s);

                l[i]->left = l[i - 1];
                l[i]->right = l[i + 1];
                l[i]->third = l[i + 3];
                l[i]->bOperator = false;
                l[i - 1] = l[i];
                l.erase(l.begin() + i + 3);
                l.erase(l.begin() + i + 2);
                l.erase(l.begin() + i + 1);
                l.erase(l.begin() + i);
                i--;
                bProgress = true;
                continue;
            }
        }
        if (!bProgress)
        {
            priority++;
            if (priority > 13)
                throw(std::string("Cannot resolve any deeper"));
        }
    }

    if (l[0]->bOperator)
        throw(std::string("only have one operator in this directive"));
}

// return error string,
std::string CLexer::transformDirectiveListToTree(const std::vector<std::string>& keywords_param, DirectiveTreeNode*& root)
{
    std::string err_s;

    if (keywords_param.empty())
    {
        root = NULL;
        return "";
    }

    // expands keywords
    std::vector<std::string> keywords = keywords_param;
    for (size_t i = 0; i < keywords.size(); i++)
    {
        std::string s = keywords[i];

        if (i + 2 < keywords.size() && keywords[i + 1] == "##")
        {
            std::string s1 = keywords[i];
            std::string s2 = keywords[i + 2];
            MY_ASSERT(isIdentifier(keywords[i]));
            MY_ASSERT(isIdentifier(keywords[i + 2]));
            keywords[i] += keywords[i + 2];
            keywords.erase(keywords.begin() + i + 2);
            keywords.erase(keywords.begin() + i + 1);
            i--; // need to check expand macro of the new keyword
            continue;
        }

        if (s == "defined" || s == "__has_feature")
        {
            if (i + 1 >= keywords.size())
                return "Insufficient parameters after defined";

            s = keywords[i + 1];
            if (s == "(")
            {
                if (i + 3 >= keywords.size())
                    return "Insufficient parameters after defined(";

                if (!isIdentifier(keywords[i + 2]))
                {
                    err_s = keywords[i + 2] + " needs to be an identifier";
                    return err_s;
                }
                if (keywords[i + 3] != ")")
                {
                    err_s = keywords[i + 3] + " needs to be )";
                    return err_s;
                }
                keywords[i] = (isDefined(keywords[i + 2]) ? "1" : "0");
                keywords.erase(keywords.begin() + i + 3);
                keywords.erase(keywords.begin() + i + 2);
                keywords.erase(keywords.begin() + i + 1);
                continue;
            }
            std::string ret_s;
            if (s == "defined")
            {
				if (!isIdentifier(s))
				{
					err_s = s + " needs to be an identifier";
					return err_s;
				}
	            ret_s = (isDefined(s) ? "1" : "0");
            }
            else
            {
            	ret_s = "0";
            }
            keywords[i] = ret_s;
            keywords.erase(keywords.begin() + i + 1);
            continue;
        }

        DefineItem item;
        if (!isIdentifier(s) || !getDefineItem(s, item))
            continue;

        size_t j = i + 1;
        std::string name = s;
        std::vector<std::vector<std::string> > values;
        if (item.params.size() > 0)
        {
            // read params
            if (keywords[j++] != "(")
                fatal_error("'(' is expected after macro %s", name.c_str());
            for (size_t k = 0; k < item.params.size(); k++)
            {
                std::vector<std::string> v;
                int nest = 0;
                while (true)
                {
                    s = keywords[j++];
                    if (s.empty())
                        fatal_error("EOF found when resolving macro <%s> in a directive", name.c_str());
                    if ((s == "," || s == ")") && nest == 0)
                        break;
                    if (s == "(")
                        nest++;
                    if (s == ")")
                        nest--;
                    v.push_back(s);
                }
                values.push_back(v);
                if (s == ",")
                {
                    if (k == item.params.size() - 1)
                        fatal_error("Only %d params are expected in macro %s", item.params.size(), name.c_str());
                    continue;
                }
                if (s == ")")
                {
                    if (k < item.params.size() - 1)
                        fatal_error("%d params are required in macro %s", item.params.size(), name.c_str());
                    break;
                }
            }
        }
        else if (j < keywords.size())
        {
            if (keywords[j] == "(")
            {
                if (j + 1 >= keywords.size() || keywords[j + 1] != ")")
                    fatal_error("macro '%s' doesn't take any parameter", s.c_str());
                keywords.erase(keywords.begin() + j + 1);
                keywords.erase(keywords.begin() + j);
            }
        }
        // expand a macro
        keywords.erase(keywords.begin() + i, keywords.begin() + j);
        j = i;

        for (size_t m = 0; m < item.value_keywords.size(); m++)
        {
			const std::string& s = item.value_keywords[m];

            bool bFound = false;
            for (size_t k = 0; k < item.params.size(); k++)
            {
                if (item.params[k] == s)
                {
                    keywords.insert(keywords.begin() + j, values[k].begin(), values[k].end());
                    j += values[k].size();
                    bFound = true;
                    break;
                }
            }
            if (!bFound)
            {
                keywords.insert(keywords.begin() + j, s);
                j++;
            }
        }
        i--;
    }

    //TRACE(", Expand to: ");
    std::vector<DirectiveTreeNode*> l;
    for (unsigned i = 0; i < keywords.size(); i++)
    {
        const std::string s = keywords[i];

        //TRACE("%s ", s.c_str());
        DirectiveTreeNode* ptr = new DirectiveTreeNode;
        bool bOperator = true;
        if (isNumber(s))
            bOperator = false;
        else if (isIdentifier(s))
        {
            if (s != "defined" && s != "L")
                bOperator = false;
        }
        ptr->bOperator = bOperator;
        ptr->value = s;
        ptr->left = ptr->right = ptr->third = NULL;
        l.push_back(ptr);
    }
    TRACE("\n");

    try {
        transformDirectiveListToTree2(l);
    }
    catch (std::string& err_s)
    {
        err_s += ", expr=";
        for (size_t i = 0; i < l.size(); i++)
            err_s += printDirectiveTree(l[i]) + ",";
        return err_s;
    }

    root = l[0];
    return "";
}

std::string CLexer::calcDirectiveTree(DirectiveTreeNode* root, long& value, std::vector<long>& return_list)
{
    std::string err_s;

    if (root->value == "defined")
    {
        value = isDefined(root->right->value);
        return err_s;
    }
    if (root->value == "(")
    {
        if (root->right == NULL)
            value = 0;
        else
            err_s = calcDirectiveTree(root->right, value, return_list);
        return err_s;
    }
    if (root->value == "!" || root->value == "~")
    {
        err_s = calcDirectiveTree(root->right, value, return_list);
        if (!err_s.empty())
            return err_s;
        //TRACE("CLexer::calcDirectiveTree, %s%ld=", root->value.c_str(), value);
        if (root->value == "!")
            value = !value;
        else
            value = ~value;
        //TRACE("%ld\n", value);
        return err_s;
    }
    if (root->value == "*" || root->value == "/" || root->value == "%")
    {
        long l_value, r_value;
        err_s = calcDirectiveTree(root->left, l_value, return_list);
        if (!err_s.empty())
            return err_s;
        err_s = calcDirectiveTree(root->right, r_value, return_list);
        if (!err_s.empty())
            return err_s;
        //TRACE("CLexer::calcDirectiveTree, %ld%s%ld=", l_value, root->value.c_str(), r_value);
        if (root->value == "*")
            value = l_value * r_value;
        else if (root->value == "/")
            value = l_value / r_value;
        else
            value = l_value % r_value;
        //TRACE("%ld\n", value);
        return err_s;
    }
    if (root->value == "+" || root->value == "-")
    {
        long l_value = 0, r_value;
        if (root->left)
        {
            err_s = calcDirectiveTree(root->left, l_value, return_list);
            if (!err_s.empty())
                return err_s;
        }
        err_s = calcDirectiveTree(root->right, r_value, return_list);
        if (!err_s.empty())
            return err_s;
        //TRACE("CLexer::calcDirectiveTree, %ld%s%ld=", l_value, root->value.c_str(), r_value);
        value = (root->value == "+" ? l_value + r_value : l_value - r_value);
        //TRACE("%ld\n", value);
        return err_s;
    }
    if (root->value == "<<" || root->value == ">>")
    {
        long l_value, r_value;
        err_s = calcDirectiveTree(root->left, l_value, return_list);
        if (!err_s.empty())
            return err_s;
        err_s = calcDirectiveTree(root->right, r_value, return_list);
        if (!err_s.empty())
            return err_s;
        //TRACE("CLexer::calcDirectiveTree, %ld%s%ld=", l_value, root->value.c_str(), r_value);
        value = (root->value == "<<" ? (l_value << r_value) : (l_value >> r_value));
        //TRACE("%ld\n", value);
        return err_s;
    }
    if (root->value == ">" || root->value == ">=" || root->value == "<" || root->value == "<=" || root->value == "==" || root->value == "!=")
    {
        long l_value, r_value;
        err_s = calcDirectiveTree(root->left, l_value, return_list);
        if (!err_s.empty())
            return err_s;
        err_s = calcDirectiveTree(root->right, r_value, return_list);
        if (!err_s.empty())
            return err_s;
        //TRACE("CLexer::calcDirectiveTree, %ld%s%ld=", l_value, root->value.c_str(), r_value);
        if (root->value == ">")
            value = (l_value > r_value);
        else if (root->value == ">=")
            value = (l_value >= r_value);
        else if (root->value == "<")
            value = (l_value < r_value);
        else if (root->value == "<=")
            value = (l_value <= r_value);
        else if (root->value == "==")
            value = (l_value == r_value);
        else if (root->value == "!=")
            value = (l_value != r_value);
        //TRACE("%ld\n", value);
        return err_s;
    }
    if (root->value == "&" || root->value == "^" || root->value == "|")
    {
        long l_value, r_value;
        err_s = calcDirectiveTree(root->left, l_value, return_list);
        if (!err_s.empty())
            return err_s;
        err_s = calcDirectiveTree(root->right, r_value, return_list);
        if (!err_s.empty())
            return err_s;
        //TRACE("CLexer::calcDirectiveTree, %ld%s%ld=", l_value, root->value.c_str(), r_value);
        if (root->value == "&")
            value = (l_value & r_value);
        else if (root->value == "^")
            value = (l_value ^ r_value);
        else if (root->value == "|")
            value = (l_value | r_value);
        //TRACE("%ld\n", value);
        return err_s;
    }
    if (root->value == "&&")
    {
        err_s = calcDirectiveTree(root->left, value, return_list);
        if (!err_s.empty())
            return err_s;
        if (!value)
        {
            //TRACE("CLexer::calcDirectiveTree, left value of && is 0, return 0\n");
            return err_s;
        }
        err_s = calcDirectiveTree(root->right, value, return_list);
        //TRACE("CLexer::calcDirectiveTree, &&, right value=%ld\n", value);
        return err_s;
    }
    if (root->value == "||")
    {
        err_s = calcDirectiveTree(root->left, value, return_list);
        if (!err_s.empty())
            return err_s;
        if (value)
        {
            //TRACE("CLexer::calcDirectiveTree, left value of || is 1, return 1\n");
            return err_s;
        }
        err_s = calcDirectiveTree(root->right, value, return_list);
        //TRACE("CLexer::calcDirectiveTree, ||, right value=%ld\n", value);
        return err_s;
    }
    if (root->value == "?")
    {
        err_s = calcDirectiveTree(root->left, value, return_list);
        if (!err_s.empty())
            return err_s;
        if (value)
            err_s = calcDirectiveTree(root->right, value, return_list);
        else
            err_s = calcDirectiveTree(root->third, value, return_list);
        return err_s;
    }
    if (isNumber(root->value))
    {
        value = calcValue(root->value.c_str());
        //TRACE("CLexer::calcDirectiveTree, number %s=%ld\n", root->value.c_str(), value);
        return err_s;
    }
    DefineItem item;
    if (!getDefineItem(root->value, item))
    {
        //TRACE("%s is not defined, use 0 instead", root->value.c_str());
        value = 0;
    }
    else
    {
        std::string n;
        if (!isDefineNumber(item, n))
            return root->value + " is not defined as a number";
        value = atol(n.c_str());
    }

    return err_s;
}

std::string CLexer::checkDirective(const std::vector<std::string> keywords, bool& bTrue)
{
    if (g_log_lex)
    {
        //printf("Checking directive: ");
        //BOOST_FOREACH(const std::string& kw, keywords)
        //    printf("%s ", kw.c_str());
    }

    DirectiveTreeNode* root;
    std::string err_s = transformDirectiveListToTree(keywords, root);
    if (!err_s.empty())
        return err_s;

    std::string s = printDirectiveTree(root);

    long value;
    std::vector<long> ll;
    err_s = calcDirectiveTree(root, value, ll);
    if (!err_s.empty())
    {
        return err_s;
    }
    deleteDirectiveTree(root);
    bTrue = (value != 0);

    return err_s;
}

std::string CLexer::simple_expand_macro(const std::string str)
{
    std::string ret_s;

    StringVector v = str_explode(str, " ");
    if (v.empty())
        return "";

    for (unsigned i = 0; i < v.size(); i++)
    {
        std::string s = v[i];
        DefineItem item;
        if (!isIdentifier(s) || !getDefineItem(s, item))
        {
            if (!ret_s.empty())
                ret_s += " ";
            ret_s += s;
        }
        else
        {
            MY_ASSERT(!item.bHasParentheses);
            StringVector v2 = item.value_keywords;
            for (unsigned j = 0; j < v2.size(); j++)
            {
                s = simple_expand_macro(v2[j]);
                if (s.empty())
                    continue;
                if (!ret_s.empty())
                    ret_s += " ";
                ret_s += s;
            }
        }
    }

    return ret_s;
}

/*
 * if A is defined with parameters but not used with (), then return with original tokens (unresolved)
 * __VA_ARGS__ is definition body means to replace "..." in the parameter
 */
std::string CLexer::expand_with_macro(const DefineItem& item, const std::string& name)
{
    SourceFile& sf = m_file_list.back();
    std::vector<std::string> values, ret_v;
    std::string v, s, orig_s = name;

    if (name == "__LINE__")
        return ltoa(get_cur_line_no());
    if (name == "__FUNCTION__")
    {
        MY_ASSERT(m_callback);
        std::string s;
        m_callback(m_callback_context, LEXER_CALLBACK_MODE_GET_FUNCTION, s);
        return s;
    }

    if (item.bHasParentheses)
    {
        // read params
        s = read_word2();
        if (s != "(")
        {
            //fatal_error("'(' is expected after macro %s in file<%s>:%d", name.c_str(), sf.file_name.c_str(), sf.row);
            sf.extra_data = s + " " + sf.extra_data;
            return name;
        }
        orig_s += " (";
    }

    int nHasVArgs = 0;
    if (item.bHasParentheses)
    {
        //item.params.size()
        while (true)
        {
            v = "";
            int nest = 0;
            while (true)
            {
                s = read_word(false);
                if (isCommentWord(s))
                    continue;
                if (s.empty())
                    fatal_error("EOF found in the middle of a macro in file <%s>", sf.file_name.c_str());
                if ((s == "," || s == ")") && nest == 0)
                    break;
                if (s == "(")
                    nest++;
                if (s == ")")
                    nest--;
                if (!v.empty())
                    v += " ";
                v += s;
            }
            values.push_back(v);
            orig_s += " " + v + " " + (s == ")" ? s : ",");
            if (s == ")")
                break;
        }

        if (item.params.empty() || item.params[item.params.size() - 1] != "...")
        {
            while (values.size() > item.params.size() && values[values.size() - 1].empty())
                values.resize(values.size() - 1);

            if (item.params.size() != values.size())
            {
                // check whether the extra parameters are all empty
                if (values.size() > item.params.size())
                {
                    std::string s;
                    for (unsigned i = item.params.size() - 1; i < values.size(); i++)
                    {
                        if (i > item.params.size() - 1)
                            s += ",";
                        s += values[i];
                    }
                    values[item.params.size() - 1] = s;
                    values.resize(item.params.size());
                }
                if (item.params.size() != values.size())
                {
                    std::string s;
                    for (unsigned i = 0; i < values.size(); i++)
                    {
                        if (i > 0)
                            s += ", ";
                        s += values[i];
                    }
                    fatal_error("%d params are expected in macro %s while %d are found (%s), in file <%s>:%d", item.params.size(), name.c_str(), values.size(), s.c_str(), sf.file_name.c_str(), sf.row);
                }
            }
        }
        else
        {
            nHasVArgs = 1;
            if (values.size() < item.params.size() - 1)
                fatal_error("At least params are expected in macro %s while %d are found, in file <%s>:%d", item.params.size() - 1, name.c_str(), values.size(), sf.file_name.c_str(), sf.row);
            std::string s;
            for (size_t i = item.params.size() - 1; i < values.size(); i++)
            {
                if (!s.empty())
                    s += ", ";
                s += values[i];
            }
            values[item.params.size() - 1] = s;
            values.resize(item.params.size());
        }
    }
    // expand a macro
    bool bHasPound = false;
    for (size_t i = 0; i < item.value_keywords.size(); i++)
    {
		const std::string& s = item.value_keywords[i];
        std::string t = s;
        if (t == "#")
        {
            if (bHasPound)
                fatal_error("# cannot followed by another #");
            bHasPound = true;
            continue;
        }
        if (t == "__VA_ARGS__")
        {
            if (nHasVArgs == 0)
                fatal_error("No '...' defined in macro parameters");
            t = values[item.params.size() - 1];
        }
        else
        {
            for (size_t i = 0; i < item.params.size() - nHasVArgs; i++)
            {
                if (item.params[i] == s)
                {
                    t = values[i];
                    if (bHasPound)
                    {
                        t = "\"" + t + "\"";
                        bHasPound = false;
                    }
                    break;
                }
            }
        }
        if (bHasPound)
            fatal_error("# must be followed by a macro parameter");
        ret_v.push_back(t);
    }
    if (bHasPound)
        fatal_error("# is found at the end of a macro");

    for (unsigned i = 0; i < ret_v.size(); i++)
    {
        if (ret_v[i] == "##")
        {
            MY_ASSERT(i > 0 && i + 1 < ret_v.size());
            MY_ASSERT(ret_v[i + 1] != "##");
            MY_ASSERT(ret_v[i - 1] != LEXER_PARAM_COMMA);
            MY_ASSERT(ret_v[i + 1] != LEXER_PARAM_COMMA);
            if (ret_v[i - 1] == LEXER_PARAM_EMPTY)
                ret_v[i - 1] = "";
            if (ret_v[i + 1] != LEXER_PARAM_EMPTY)
                ret_v[i - 1] += ret_v[i + 1];
            ret_v.erase(ret_v.begin() + i + 1);
            ret_v.erase(ret_v.begin() + i);
            i--;
        }
    }

    std::string expanded_s;
    for (unsigned i = 0; i < ret_v.size(); i++)
    {
        if (i > 0)
            expanded_s += " ";
        expanded_s += ret_v[i];
    }

    //TRACE("Resolve macro '%s' to '%s'\n", orig_s.c_str(), expanded_s.c_str());
    if (orig_s == expanded_s)
    {
        MY_ASSERT(ret_v[0] == name);
        expanded_s = expanded_s.substr(name.size() + 1);
        sf.extra_data = expanded_s + " " + sf.extra_data;
        return name;
    }

    return expanded_s;
}

// read a word, handles #define, #include, #ifdef, #ifndef, #if
std::string CLexer::read_word(bool bFromExternal)
{
    if (!m_bFileMode)
    {
        if (m_token_offset >= m_tokens.size())
            return "";

        std::string s = m_tokens[m_token_offset++];
		if (m_token_offset >= m_tokens.size())
		{
			m_tokens.clear();
			m_token_offset = 0;
		}
        //printf("read word <%s>\n", s.c_str());

        if (isCommentWord(s))
        {
            if (s[2] == '*')
            {
                m_file_line_no += atoi(s.c_str() + 3);
            }
            else
            {
                std::string s2 = s.substr(2);
                size_t pos = s2.find('@');
                MY_ASSERT(pos != std::string::npos);
                m_file_line_no = atoi(s2.c_str() + pos + 1);
                s2.resize(pos);
                m_file_stack.clear();
                //printf("\nchange file_stack to ");
                while (true)
                {
                    pos = s2.find('|');
                    if (pos == std::string::npos)
                    {
                        //printf("|%s", s2.c_str());
						MY_ASSERT(s2.size() > 2);
                        m_file_stack.push_back(s2);
                        break;
                    }
                    //printf("|%s", s2.substr(0, pos).c_str());
					MY_ASSERT(s2.substr(0, pos).size() > 2);
                    m_file_stack.push_back(s2.substr(0, pos));
                    s2 = s2.substr(pos + 1);
                }
                //printf("\n");
            }
        }
        return s;
    }

	if (m_token_offset < m_tokens.size()) // only for token after '#'
	{
        std::string s = m_tokens[m_token_offset++];
		return s;
	}

    int file_stack_size = m_file_list.size();
    int last_row = (m_file_list.size() > 0 ? m_file_list.back().row : -1);
    while (m_file_list.size() > 0)
    {
        if (file_stack_size != m_file_list.size())
        {
            SourceFile& sf = m_file_list.back();
            std::string s = "//";
            for (int i = 0; i < m_file_list.size(); i++)
            {
                if (i > 0)
                    s += "|";

                s += m_file_list[i].file_name; // + "-" + ltoa(m_file_list[i].row);
            }
            last_row = m_file_list.back().row;
            s += std::string("@") + ltoa(last_row);
            TRACE("#Change file to %s\n", s.c_str());
            return s;
        }
        SourceFile& sf = m_file_list.back();

        std::string s = read_word2();
        if (s.empty())
        {
            if (m_file_list.size() > 1)
            {
				MY_ASSERT(sf.is_allocated);
				free(sf.content_start);
                m_file_list.pop_back();
                continue;
            }
            return s;
        }

        if (last_row != sf.row)
        {
            sf.extra_data = s + " " + sf.extra_data;
            return std::string("//*") + ltoa(sf.row - last_row);
        }

        if (s != "#")
        {
            if (bFromExternal)
            {
                if (s == LEXER_PARAM_EMPTY)
                    continue;
                if (s == LEXER_PARAM_COMMA)
                    return ",";
            }
            DefineItem item;
            if (!isIdentifier(s) || !getDefineItem(s, item))
                return s;

            //if (item.value_keywords.size() == 1 && item.params.size() == 0 && item.value_keywords[0] == s)
            //    return s;

            std::string s2 = expand_with_macro(item, s);
            if (s2 == s)
                return s;

            sf.extra_data = s2 + " " + sf.extra_data;
            continue;
        }

        s = read_word2(false);
        if (s.empty())
            continue;

        if (s == "define")
        {
            std::string name = read_word2(false);
            if (name.empty())
                fatal_error("define line is empty at %s:%d", sf.file_name.c_str(), sf.row);
            if (!isIdentifier(name))
                fatal_error("define name needs to be identifier at %s:%d", sf.file_name.c_str(), sf.row);
            bool bHasParentheses = false;
            std::vector<std::string> params;
            if (peek_char() == '(')
            {
                bHasParentheses = true;
                s = read_word2(false); // the '('
                s = read_word2(false);
                bool bHasVArgs = false;
                if (!s.empty() && s != ")")
                {
                    if (s == "...")
                    {
                        params.push_back(s);
                        bHasVArgs = true;
                    }
                    else if (isIdentifier(s))
                    {
                        params.push_back(s);
                    }
                    else
                        fatal_error("expecting an identifier of parameter at line:%d, col:%d in <%s>", sf.row, sf.col, sf.file_name.c_str());
                    s = read_word2(false);
                }
                while (s != ")")
                {
                    if (s.empty())
                        fatal_error("unexpected EOF with a define at %s", sf.file_name.c_str());
                    if (s != ",")
                        fatal_error("',' is expected at line:%d, col:%d in <%s>", sf.row, sf.col, sf.file_name.c_str());
                    s = read_word2(false);
                    if (s.empty())
                        fatal_error("unexpected EOF with a define at %s", sf.file_name.c_str());
                    if (bHasVArgs)
                        fatal_error("Unexpected parameter found after ... at line:%d, col:%d in <%s>", sf.row, sf.col, sf.file_name.c_str());
                    if (s == "...")
                    {
                        params.push_back(s);
                        bHasVArgs = true;
                    }
                    else if (isIdentifier(s))
                    {
                        params.push_back(s);
                    }
                    else
                        fatal_error("expecting an identifier of parameter at line:%d, col:%d in <%s>", sf.row, sf.col, sf.file_name.c_str());
                    s = read_word2(false);
                }
            }
            std::vector<std::string> l;
            while (true)
            {
                s = read_word2(false);
                if (s.empty())
                    break;
                l.push_back(s);
            }
            std::string err_s = handleDefine(name, bHasParentheses, params, l);
            if (!err_s.empty())
                fatal_error("At line:%d, col:%d in <%s>, %s", sf.row, sf.col, sf.file_name.c_str(), err_s.c_str());
        }
        else if (s == "include")
        {
            s = read_word2();
            std::string fname;

            DefineItem item;
            while (getDefineItem(s, item))
            {
                std::string s2 = expand_with_macro(item, s);
                StringVector ret_v = str_explode(s2, " ");
                s2 = combine_tokens(ret_v.begin(), ret_v.end());
                sf.extra_data = s2 + " " + sf.extra_data;
                s = read_word2();
            }
            if (s.at(0) == '"')
            {
                s = s.substr(1, s.size() - 2);
                fname = sf.file_name;
                size_t pos = fname.rfind('/');
                if (pos == std::string::npos)
                {
                    fname = s;
                }
                else
                {
                    fname.resize(pos + 1);
                    fname += s;
                }
                if (!file_exists(fname.c_str()))
                    fname = "";
            }
            else if (s == "<")
            {
                char c;
                s = "";
                while (true)
                {
                    if (!read_char(c) || c == '\n')
                        fatal_error("#include error in file %s:%d", sf.file_name.c_str(), sf.row);
                    if (c == '>')
                        break;
                    s += c;
                }
            }
            else
                fatal_error("#include error in file %s:%d, s=%s", sf.file_name.c_str(), sf.row, s.c_str());

            if (fname.empty())
            {
                fname = s;
                bool bFound = false;
                for (size_t i = 0; i < m_include_path.size(); i++)
                {
					const std::string& path = m_include_path[i];

                    std::string s = path + "/" + fname;
                    if (file_exists(s.c_str()))
                    {
                        bFound = true;
                        fname = s;
                        break;
                    }
                }
                if (!bFound)
                    fatal_error("include file <%s> not found in file %s:%d", fname.c_str(), sf.file_name.c_str(), sf.row);
            }
            if (m_pragma_once_files.find(fname) != m_pragma_once_files.end())
                continue;

            char* content = readContentFromFile(fname.c_str());
            if (content == NULL)
                fatal_error("include file '%s', not found in file %s:%d", fname.c_str(), sf.file_name.c_str(), sf.row);
            SourceFile sf2;
			sf2.file_name = fname;
			sf2.content_start = sf2.content_cur = content;
            m_file_list.push_back(sf2);
            continue;
        }
        else if (s == "ifdef" || s == "ifndef" || s == "if")
        {
            int start_line = sf.row;
            bool bTrue;
            if (s == "if")
            {
                std::vector<std::string> keywords;
                while (true)
                {
                    s = read_word2(false);
                    if (s.empty())
                        break;
                    keywords.push_back(s);
                }
                std::string err = checkDirective(keywords, bTrue);
                if (!err.empty())
                    fatal_error("Analyzing directive failed in file %s:%d, %s", sf.file_name.c_str(), sf.row, err.c_str());
            }
            else
            {
                bTrue = (s == "ifdef");
                s = read_word2(false);
                if (s.empty())
                    fatal_error("Expecting an identifier in file %s:%d", sf.file_name.c_str(), sf.row);

                bTrue = (bTrue == isDefined(s));
                s = read_word2(false);
                if (!s.empty())
                    fatal_error("Unexpected identifier '%s' at %s:%d", s.c_str(), sf.file_name.c_str(), sf.row);
            }
            int local_nest = 1;
            if (!bTrue) // skip false blocks
            {
                while (true)
                {
                    s = read_word2();
                    if (s.empty())
                        fatal_error("Unexpected EOF within an #if block in <%s>:%d", sf.file_name.c_str(), start_line);
                    if (s == "#")
                    {
                        s = read_word2(false);
                        if (s.empty())
                            continue;
                        if (s == "ifdef" || s == "ifndef" || s == "if")
                            local_nest++;
                        else if (s == "elif" || s == "else")
                        {
                            if (local_nest > 1)
                                continue;
                            if (s == "else")
                                break;
                            std::vector<std::string> keywords;
                            while (true)
                            {
                                s = read_word2(false);
                                if (s.empty())
                                    break;
                                keywords.push_back(s);
                            }
                            std::string err = checkDirective(keywords, bTrue);
                            if (!err.empty())
                                fatal_error("Analyzing directive in file %s:%d failed, %s", sf.file_name.c_str(), sf.row, err.c_str());
                            if (bTrue)
                                break;
                        }
                        else if (s == "endif")
                        {
                            local_nest--;
                            if (local_nest == 0)
                                break;
                        }
                    }
                }
            }
            if (local_nest == 0)
                continue;

            // finding the range of true block
            if (!sf.extra_data.empty())
                fatal_error("directive is in a macro, which is impossible");
            SourceFile sf2;
            sf2.file_name = sf.file_name;
            sf2.row = sf.row;
            sf2.col = sf.col;
            int start_offset = sf.content_cur - sf.content_start;
            char* content_end = NULL;
            while (true)
            {
                content_end = sf.content_cur;
                s = read_word2();
                if (s.empty())
                    fatal_error("Unexpected EOF within an #if block in <%s>:%d", sf.file_name.c_str(), sf2.row);
                if (s == "#")
                {
                    s = read_word2(false);
                    if (s.empty())
                        continue;
                    if (s == "ifdef" || s == "ifndef" || s == "if")
                    {
                        local_nest++;
                        //printf("got %s, local_nest=%d\n", s.c_str(), local_nest);
                    }
                    else if (s == "elif" || s == "else")
                    {
                        //printf("got %s, local_nest=%d\n", s.c_str(), local_nest);
                        if (local_nest == 1)
                            break;
                    }
                    else if (s == "endif")
                    {
                        local_nest--;
                        //printf("got %s, local_nest=%d\n", s.c_str(), local_nest);
                        if (local_nest == 0)
                            break;
                    }
                }
            }
            int end_offset = content_end - sf.content_start;
            sf2.content_start = (char*)malloc(end_offset - start_offset + 1);
            memcpy(sf2.content_start, sf.content_start + start_offset, end_offset - start_offset);
            sf2.content_start[end_offset - start_offset] = 0;
            sf2.content_cur = sf2.content_start;

            // skip false blocks
            int else_line_no = sf.row;
            if (local_nest > 0)
            {
                while (true)
                {
                    s = read_word2();
                    if (s.empty())
                        fatal_error("Unexpected EOF within an #if:%d/#else:%d block in <%s>", start_line, else_line_no, sf.file_name.c_str());
                    if (s == "#")
                    {
                        s = read_word2(false);
                        if (s.empty())
                            continue;
                        if (s == "ifdef" || s == "ifndef" || s == "if")
                            local_nest++;
                        else if (s == "elif" || s == "else")
                        {
                        }
                        else if (s == "endif")
                        {
                            local_nest--;
                            if (local_nest == 0)
                                break;
                        }
                    }
                }
            }
            m_file_list.push_back(sf2);
            TRACE("#Entering block %s:%d~%d\n", sf2.file_name.c_str(), sf2.row, sf.row);
            continue;
        }
        else if (s == "undef")
        {
            s = read_word2(false);
            if (s.empty())
                fatal_error("Expecting an identifier in file %s:%d", sf.file_name.c_str(), sf.row);

            handleUndefine(s);

            s = read_word2(false);
            if (!s.empty())
                fatal_error("Unexpected identifier at line:%d, col:%d in <%s>", sf.row, sf.col, sf.file_name.c_str());
        }
        else if (s == "error")
            fatal_error("Meet error directive in %s:%d", sf.file_name.c_str(), sf.row);
        else if (s == "pragma")
        {
            std::vector<std::string> keywords;
			std::string whole_line = s;
            while (true)
            {
                s = read_word2(false);
                if (s.empty())
                    break;
                keywords.push_back(s);
				whole_line += " " + s;
            }
            /*MY_ASSERT(keywords.size() > 0);
            s = keywords[0];
            if (s == "GCC")
            {
                if (keywords[1] == "system_header" || keywords[1] == "visibility")
                {
                    // do nothing
                }
                else
                    fatal_error("unknown pragma GCC directive <%s> in %s:%d", s.c_str(), sf.file_name.c_str(), sf.row);
            }
            else if (s == "once")
            {
                MY_ASSERT(!m_file_list.back().file_name.empty());
                m_pragma_once_files.insert(m_file_list.back().file_name);
            }
            else if (s == "pack" || s == "warning" || s == "comment")
			{
				while (true)
				{
					s = read_word2(false);
					if (s.empty())
						break;
				}
			}
            else
                fatal_error("unknown pragma directive <%s> in %s:%d", s.c_str(), sf.file_name.c_str(), sf.row);*/

			m_tokens.push_back(whole_line);
			return "#";
        }
        else
        {
        	if (s == "line")
			{
				s = read_word2(false);
				MY_ASSERT(!s.empty());
			}
			MY_ASSERT(isNumber(s));
			unsigned int line_no = atoi(s.c_str());
			s = read_word2(false);
			MY_ASSERT(!s.empty());
			MY_ASSERT(s[0] == '"');
			std::string file_path = s.substr(1, s.size() - 2);
			// converting "\\" to "\"
#ifdef _WIN32
			for (size_t kk = 0; kk < file_path.size(); kk++)
			{
				if (file_path[kk] == '\\' && kk + 1 < file_path.size() && file_path[kk + 1] == '\\')
					file_path.erase(kk + 1, 1);
			}
#endif
            while (true)
            {
                s = read_word2(false);
                if (s.empty())
                    break;
				//whole_line += " " + s;
            }
			//m_tokens.push_back(whole_line);
			//return "#";

			if (file_path == "<built-in>" || file_path == "<command-line>")
				continue;
			if (line_no == 1)
			{
				if (m_file_list[0].row == 1)
				{
					if (m_file_list[0].file_name != file_path)
						m_file_list[0].file_name = file_path;
				}
				else if (m_file_list[0].file_name != file_path) // usually the 4th line is the same as the first line
				{
					SourceFile newF;
					newF.content_start = newF.content_cur = sf.content_cur;
					newF.row = 0; // there's a '\n' needs to be skipped.
					newF.file_name = file_path;
					newF.is_allocated = true;
					m_file_list.push_back(newF);
				}
				//for (int i = 0; i < s_include_path_list.size(); i++)
				//	printf("    ");
				//s_include_path_list.push_back(file_path);
				//printf("%s\n", file_path.c_str());
			}
			else
			{
				int i;
				for (i = m_file_list.size() - 1; i >= 0; i--)
				{
					if (m_file_list[i].file_name == file_path)
					{
						m_file_list[i].content_cur = sf.content_cur;
						m_file_list[i].row = line_no - 1; // there's a '\n' needs to be skipped.
						m_file_list.resize(i + 1); // assume their is_allocated are all true
						break;
					}
				}
				MY_ASSERT(i >= 0);
			}
        }
		//fatal_error("unknown directive <%s> in %s:%d", s.c_str(), sf.file_name.c_str(), sf.row);
    }

    return "";
}

std::string CLexer::read_word_without_comment()
{
    std::string s;

    while (true)
    {
        s = read_word();
        if (!isCommentWord(s))
            break;
    }

    return s;
}

void CLexer::startWithFile(const char* file_name)
{
    m_bFileMode = true;
    init();

    char* content = readContentFromFile(file_name);
    if (content == NULL)
        fatal_error("File '%s' open failed", file_name);

    SourceFile sf;
	sf.file_name = file_name;
	sf.content_start = sf.content_cur = content;
    m_file_list.push_back(sf);
}

void CLexer::startWithBuffer(const char* file_name, const char* buf)
{
    m_bFileMode = true;
    init();

    char* content = (char*)malloc(strlen(buf) + 1);
    strcpy(content, buf);

    SourceFile sf;
	sf.file_name = file_name;
	sf.content_start = sf.content_cur = content;
    m_file_list.push_back(sf);
}

void CLexer::startWithTokens(const StringVector& tokens)
{
    m_bFileMode = false;

    MY_ASSERT(tokens.size() >= 1);
    MY_ASSERT(isCommentWord(tokens[0]));
    m_tokens = tokens;
	m_token_offset = 0;

    std::string s = m_tokens[m_token_offset++];
    string_2_file_stack(s, m_file_stack, m_file_line_no);

    MY_ASSERT(m_file_stack.size() > 0);
}

void CLexer::pushTokenFront(const std::string& s)
{
	m_tokens.insert(m_tokens.begin() + m_token_offset, s);
}

void CLexer::printDefineMap()
{
    FILE* fp = fopen("defines.map", "wt");

    for (DefineMap::const_iterator it = m_define_map.begin(); it != m_define_map.end(); it++)
    {
        std::string str = "#define " + it->first;
        if (!it->second.params.empty())
        {
            str += "(";
            int n = 0;
            for (size_t i = 0; i < it->second.params.size(); i++)
            {
				const std::string& param = it->second.params[i];
                if (n > 0)
                    str += ", ";
                str += param;
                n++;
            }
            str += ")";
        }
        str += "    ";
        for (size_t i = 0; i < it->second.value_keywords.size(); i++)
        {
			const std::string& s = it->second.value_keywords[i];
            str += s + " ";
        }
        fprintf(fp, "%s\n", str.c_str());
    }

    fclose(fp);
}
