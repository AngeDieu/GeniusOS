#ifndef POINCARE_PARSING_PARSER_H
#define POINCARE_PARSING_PARSER_H

/* A precedence-climbing parser is implemented hereafter.
 * It is a trade-off between
 *   a readable but less efficient recursive-descent parser
 * and
 *   an efficient but less readable shunting-yard parser. */

#include "tokenizer.h"
#include "parsing_context.h"

namespace Poincare {

class Parser {
public:
  enum class Status {
    Success,
    Progress,
    Error
  };

  /* Set the context to nullptr if the expression has already been parsed
   * Setting the context to nullptr remove some ambiguous cases like:
   * - f(x) will always be parsed as f(x) and not f*(x)
   * - u{n} will always be parsed as u_n and not u*{n}
   * - abc will always be parsed as abc and not a*b*c
   * The same is true if you set parseForAssignment = true
   * but the parser will set parseForAssignment = false when it encounters a "=".
   * (so that f(x)=xy is parsed as f(x)=x*y, and not as f*(x)=x*y or as f(x)=xy) */
  Parser(const char * text, Context * context, bool parseForAssignment = false) :
    m_parsingContext(context, parseForAssignment ? ParsingContext::ParsingMethod::Assignment : ParsingContext::ParsingMethod::Classic),
    m_status(Status::Progress),
    m_tokenizer(text, &m_parsingContext),
    m_currentToken(Token(Token::Undefined)),
    m_nextToken(m_tokenizer.popToken()),
    m_pendingImplicitMultiplication(false),
    m_waitingSlashForMixedFraction(false)
  {}

  Expression parse();
  Status getStatus() const { return m_status; }

  static bool IsReservedName(const char * name, size_t nameLength);

private:
  Expression parseUntil(Token::Type stoppingType);

  // Methods on Tokens
  void popToken();
  bool popTokenIfType(Token::Type type);
  bool nextTokenHasPrecedenceOver(Token::Type stoppingType);
  void isThereImplicitMultiplication();

  // Specific Token parsers
  void parseUnexpected(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parseNumber(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parseConstant(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parseUnit(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parseReservedFunction(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parseSpecialIdentifier(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parseCustomIdentifier(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parseEmpty(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parseMatrix(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parseLeftParenthesis(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parseLeftSystemParenthesis(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parseBang(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parsePercentAddition(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parsePercentSimple(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parsePlus(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parseMinus(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parseTimes(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parseSlash(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parseImplicitTimes(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parseCaret(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parseCaretWithParenthesis(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parseComparisonOperator(Token::Type tokenType, Expression & leftHandSide, Token::Type stoppingType);
  void parseEqual(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0) { parseComparisonOperator(Token::Equal, leftHandSide, stoppingType); }
  void parseSuperior(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0) { parseComparisonOperator(Token::Superior, leftHandSide, stoppingType); }
  void parseSuperiorEqual(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0) { parseComparisonOperator(Token::SuperiorEqual, leftHandSide, stoppingType); }
  void parseInferior(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0) { parseComparisonOperator(Token::Inferior, leftHandSide, stoppingType); }
  void parseInferiorEqual(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0) { parseComparisonOperator(Token::InferiorEqual, leftHandSide, stoppingType); }
  void parseRightwardsArrow(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parseLeftSuperscript(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);
  void parseList(Expression & leftHandSide, Token::Type stoppingType = (Token::Type)0);

  // Parsing helpers
  bool parseBinaryOperator(const Expression & leftHandSide, Expression & rightHandSide, Token::Type stoppingType);
  Expression parseVector();
  Expression parseFunctionParameters();
  Expression parseCommaSeparatedList();
  void privateParseReservedFunction(Expression & leftHandSide, const Expression::FunctionHelper * const * functionHelper);
  void privateParseCustomIdentifier(Expression & leftHandSide, const char * name, size_t length);
  void parseSequence(Expression & leftHandSide, const char * name, Token::Type rightDelimiter);
  void defaultParseLeftParenthesis(bool isSystemParenthesis, Expression & leftHandSide, Token::Type stoppingType);
  bool generateMixedFractionIfNeeded(Expression & leftHandSide);
  // Allows you to rewind to previous position
  void rememberCurrentParsingPosition(Token * storedCurrentToken, Token * storedNextToken, const char ** tokenizerPosition);
  void restorePreviousParsingPosition(Token storedCurrentToken, Token storedNextToken, const char * tokenizerPosition);
  // Data members
  ParsingContext m_parsingContext;
  Status m_status;
    /* m_status is initialized to Status::Progress,
     * is changed to Status::Error if the Parser encounters an error,
     * and is otherwise changed Status::Success. */
  Tokenizer m_tokenizer;
  Token m_currentToken;
  Token m_nextToken;
  bool m_pendingImplicitMultiplication;
  bool m_waitingSlashForMixedFraction;
};

}

#endif
