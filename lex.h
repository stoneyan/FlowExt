#ifndef __LEX__H_
#define __LEX__H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>
#include <list>
#include <vector>
#include <set>
#include <map>

typedef std::vector<std::string> StringVector;
typedef std::set<std::string> StringSet;

typedef unsigned long long uint64_t;

#ifdef _DEBUG
#define MY_ASSERT(x)	do { if (!(x)) abort(); } while (false)
#else
#define MY_ASSERT(x)	do {} while (false)
#endif
void fatal_error(const char* format, ...);
char* ltoa(long v);
StringVector str_explode(const std::string& str, const std::string& delim);
void write_log(const char* fmt, ...);
char* cur_time();
std::string trim(const std::string &s);
uint64_t get_cur_tick();
StringVector get_sys_include_path();

#define LEXER_CALLBACK_MODE_GET_FUNCTION    1

typedef bool (*LexerCallback)(void* context, int mode, std::string& s);

class CLexer
{
public:
    CLexer();
    ~CLexer();

    void setCallback(LexerCallback cb, void* context) { m_callback = cb; m_callback_context = context; }

    void startWithFile(const char* file_name);
    void startWithBuffer(const char* file_name, const char* buf);
    void startWithTokens(const StringVector& tokens);

	void pushTokenFront(const std::string& s);

    // if the return string prefix with "//", skip it
    std::string read_word(bool bFromExternal = true);
    std::string read_word_without_comment();

    int get_cur_line_no();
	std::string get_cur_filename();
    StringVector get_file_stack();

    bool is_empty();

    // for debug purpose
    void printDefineMap();

    static bool isNumber(const std::string& str);
    static bool isIdentifier(const std::string& str);
    static bool isCommentWord(const std::string& s);
    static std::string file_stack_2_string(const StringVector& file_stack, int line_no);
    static void string_2_file_stack(const std::string& s, StringVector& file_stack, int& line_no);

protected:
    void init();

    struct SourceFile {
        std::string file_name;
        char*       content_start;
        char*       content_cur;
        int         row;
        int         col;
        std::string extra_data;
		bool		is_allocated;

		SourceFile() : content_start(NULL), content_cur(NULL), row(1), col(1), is_allocated(false) {}
    };

    struct DefineItem {
        bool            bHasParentheses;
        StringVector    params;
        StringVector    value_keywords;
    };
    typedef std::map<std::string, DefineItem>   DefineMap;

    struct DirectiveTreeNode {
        bool bOperator;
        std::string value;
        DirectiveTreeNode* left;
        DirectiveTreeNode* right;
        DirectiveTreeNode* third;
    };

    bool					m_bFileMode; // true: file, false: token
    // read from file
    StringVector            m_include_path;
    std::vector<SourceFile> m_file_list;
    DefineMap               m_define_map;

    // read from tokens
    StringVector    		m_tokens;
	int						m_token_offset;
    StringVector			m_file_stack;
    int						m_file_line_no;

    std::set<std::string>   m_pragma_once_files;

    LexerCallback           m_callback;
    void*                   m_callback_context;

    // read all content in a file and returns it
    static char* readContentFromFile(const char* file_name);
    static long calcValue(const char* str);
    static bool isSymbol(char c);
    static std::string combine_tokens(StringVector::iterator begin, StringVector::iterator end);
    bool isDefined(const std::string& s);
    bool getDefineItem(const std::string& s, DefineItem& item);
    bool isDefineNumber(DefineItem& item, std::string& n);
    // analyze define statement and store it into m_define_map
    std::string handleDefine(const std::string& name, bool bHasParentheses, const StringVector& params, const StringVector& keywords);
    void handleUndefine(const std::string& name);
    // print the whole m_define_map to file "defines.map"
    std::string expand_with_macro(const DefineItem& item, const std::string& s);
    std::string simple_expand_macro(const std::string str);

    std::string printDirectiveTree(DirectiveTreeNode* root);
    void deleteDirectiveTree(DirectiveTreeNode* root);
    std::string transformDirectiveListToTree(const std::vector<std::string>& keywords_param, DirectiveTreeNode*& root);
    void transformDirectiveListToTree2(std::vector<DirectiveTreeNode*>& l);
    std::string calcDirectiveTree(DirectiveTreeNode* root, long& value, std::vector<long>& return_list);
    std::string checkDirective(const std::vector<std::string> keywords, bool& bTrue);

    bool read_char(char& c, bool bCrossNewLine = true);
    char peek_char(int offset = 0);
    char get_char();
    std::string read_word2(bool bCrossNewLine = true);
};

#endif // __LEX__H_
