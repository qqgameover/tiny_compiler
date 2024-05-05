#include <string_view>
#include <cctype>

enum class Kind {
    Number,
    Identifier,
    LeftParen,
    RightParen,
    LeftSquare,
    RightSquare,
    LeftCurly,
    RightCurly,
    LessThan,
    GreaterThan,
    Equal,
    Plus,
    Minus,
    Asterisk,
    Slash,
    Hash,
    Dot,
    Comma,
    Colon,
    Semicolon,
    SingleQuote,
    DoubleQuote,
    Comment,
    Pipe,
    End,
    Error
};

class Token {
    public:
        Token(Kind kind) noexcept : m_kind(kind) {}
        Token(Kind kind, const char* beg, std::size_t len) noexcept : m_kind{kind}, m_lexeme(beg, len) {};
        Token(Kind kind, const char* begin, const char* end) noexcept : m_kind{kind}, m_lexeme(begin, std::distance(begin, end)) {}
        void kind(Kind kind) noexcept { m_kind = kind; }
        Kind kind() const noexcept { return m_kind; }
        bool is(Kind kind) const noexcept { return m_kind == kind; }
        bool is_either(Kind kind1, Kind kind2) const noexcept { return m_kind == kind1 || m_kind == kind2; }

        template<typename... Ts>
        bool is_either(Kind k1, Kind k2,  Ts... kinds) const noexcept {
            return is(k1) || is_either(k1, kinds...);
        }

        std::string_view lexeme() const noexcept { return m_lexeme; }

        void lexeme(std::string_view lexeme) noexcept { m_lexeme = std::move(lexeme); }

    private:
        Kind             m_kind{};
        std::string_view m_lexeme{};
};

class Lexer_a {
    public:
        Lexer_a(const char* begin) noexcept : m_begin(begin){}
        Token next() noexcept;

    private:
        Token Identifier() noexcept;
        Token number() noexcept;
        Token comment_or_slash() noexcept;
        Token atom(Kind kind) noexcept;

        char peek() const noexcept { return *m_begin; }
        char get() noexcept { return *m_begin++; }

        const char *m_begin = nullptr;
};

bool is_space(char c) noexcept {
    return std::isspace(c);
}

bool is_digit(char c) noexcept {
    return std::isdigit(c);
}

bool is_identifier_char(char c) noexcept {
    return std::isalnum(c) || c == '_';
}

Token Lexer_a::atom(Kind kind) noexcept {
    return Token(kind, m_begin++, 1);
}

Token Lexer_a::next() noexcept {
    while (is_space(peek())) {
        get();
    } 

    switch(peek())
    {
        case '\0': return Token(Kind::End);
        case '(': return atom(Kind::LeftParen);
        case ')': return atom(Kind::RightParen);
        case '[': return atom(Kind::LeftSquare);
        case ']': return atom(Kind::RightSquare);
        case '{': return atom(Kind::LeftCurly);
        case '}': return atom(Kind::RightCurly);
        case '<': return atom(Kind::LessThan);
        case '>': return atom(Kind::GreaterThan);
        case '=': return atom(Kind::Equal);
        case '+': return atom(Kind::Plus);
        case '-': return atom(Kind::Minus);
        case '*': return atom(Kind::Asterisk);
        case '/': return atom(Kind::Slash);
        case '#': return atom(Kind::Hash);
        case '.': return atom(Kind::Dot);
        case ',': return atom(Kind::Comma);
        case ':': return atom(Kind::Colon);
        case ';': return atom(Kind::Semicolon);
        case '\'': return atom(Kind::SingleQuote);
        case '"': return atom(Kind::DoubleQuote);
        case '|': return atom(Kind::Pipe);
        default:
            if (is_digit(peek())) return number();
            if (is_identifier_char(peek())) return Identifier();
            return Token(Kind::Error, m_begin++, 1);
    }

}

Token Lexer_a::Identifier() noexcept {
    const char* begin = m_begin;
    get();
    while (is_identifier_char(peek())) {
        get();
    }
    return Token(Kind::Identifier, begin, m_begin);
}

Token Lexer_a::number() noexcept {
    const char* begin = m_begin;
    get();
    while (is_digit(peek())) {
        get();
    }
    return Token(Kind::Number, begin, m_begin);
}

Token Lexer_a::comment_or_slash() noexcept {
    const char* begin = m_begin;
    get();
    if (peek() == '/') {
        get();
        begin = m_begin;
        while (peek() != '\0') {
            if(get() == '\n') {
                return Token(Kind::Comment, begin, std::distance(begin, m_begin) - 1);
            }
        }
        return Token(Kind::Error, m_begin, 1);
    } else { return Token(Kind::Slash, begin, 1); }
}

//Terrible taste of Lexer_a

#include <iomanip>
#include <iostream>
std::ostream& operator<<(std::ostream& os, const Kind& kind) {
    static const char* const names[]{
            "Number",      "Identifier",  "LeftParen",  "RightParen", "LeftSquare",
            "RightSquare", "LeftCurly",   "RightCurly", "LessThan",   "GreaterThan",
            "Equal",       "Plus",        "Minus",      "Asterisk",   "Slash",
            "Hash",        "Dot",         "Comma",      "Colon",      "Semicolon",
            "SingleQuote", "DoubleQuote", "Comment",    "Pipe",       "End",
            "Unexpected",
    };
    return os << names[static_cast<int>(kind)];
}


int main(int argc, char **argv) {
    // Lexer test code
    // Should be more than good enough for tiny basic I hope xd
    auto code =
      "x = 2\n"
      "// This is a comment.\n"
      "var x\n"
      "var y\n"
      "var f = function(x, y) { sin(x) * sin(y) + x * y; }\n"
      "der(f, x)\n"
      "var g = function(x, y) { 2 * (x + der(f, y)); } // der(f, y) is a "
      "matrix\n"
      "var r{3}; // Vector of three elements\n"
      "var J{12, 12}; // Matrix of 12x12 elements\n"
      "var dot = function(u{:}, v{:}) -> scalar {\n"
      "          return u[i] * v[i]; // Einstein notation\n"
      "}\n"
      "var norm = function(u{:}) -> scalar { return sqrt(dot(u, u)); }\n"
      "<end>";

  Lexer_a lex(code);
  for (auto token = lex.next();
       not token.is_either(Kind::End, Kind::Error);
       token = lex.next()) {
    std::cout << std::setw(12) << token.kind() << " |" << token.lexeme()
              << "|\n";
  }
}
