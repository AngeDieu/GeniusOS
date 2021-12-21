#include <poincare/init.h>
#include <poincare_expressions.h>
#include <poincare/exception_checkpoint.h>
#include <poincare/src/parsing/parser.h>
#include <apps/shared/global_context.h>
#include "tree/helpers.h"
#include "helper.h"

using namespace Poincare;

void assert_tokenizes_as(const Token::Type * tokenTypes, const char * string) {
  Tokenizer tokenizer(string, nullptr);
  while (true) {
    Token token = tokenizer.popToken();
    quiz_assert_print_if_failure(token.type() == *tokenTypes, string);
    if (token.type() == Token::EndOfStream) {
      return;
    }
    tokenTypes++;
  }
}

void assert_tokenizes_as_number(const char * string) {
  const Token::Type types[] = {Token::Number, Token::EndOfStream};
  assert_tokenizes_as(types, string);
}

void assert_tokenizes_as_unit(const char * string) {
  const Token::Type types[] = {Token::Unit, Token::EndOfStream};
  assert_tokenizes_as(types, string);
}

void assert_tokenizes_as_constant(const char * string) {
  const Token::Type types[] = {Token::Constant, Token::EndOfStream};
  assert_tokenizes_as(types, string);
}

void assert_tokenizes_as_undefined_token(const char * string) {
  Tokenizer tokenizer(string, nullptr);
  while (true) {
    Token token = tokenizer.popToken();
    if (token.type() == Token::Undefined) {
      return;
    }
    quiz_assert_print_if_failure(token.type() != Token::EndOfStream, string);
  }
}

void assert_text_not_parsable(const char * text) {
  Expression result = Expression::Parse(text, nullptr);
  quiz_assert_print_if_failure(result.isUninitialized(), text);
}

void assert_parsed_expression_is(const char * expression, Poincare::Expression r, bool addParentheses = false) {
  Shared::GlobalContext context;
  Expression e = parse_expression(expression, &context, addParentheses);
  quiz_assert_print_if_failure(e.isIdenticalTo(r), expression);
}

void assert_parsed_expression_with_user_parentheses_is(const char * expression, Poincare::Expression r) { return assert_parsed_expression_is(expression, r, true); }

QUIZ_CASE(poincare_parsing_tokenize_numbers) {
  assert_tokenizes_as_number("1");
  assert_tokenizes_as_number("12");
  assert_tokenizes_as_number("123");
  assert_tokenizes_as_number("1.3");
  assert_tokenizes_as_number(".3");
  assert_tokenizes_as_number("1.3ᴇ3");
  assert_tokenizes_as_number("12.34ᴇ56");
  assert_tokenizes_as_number(".3ᴇ-32");
  assert_tokenizes_as_number("12.");
  assert_tokenizes_as_number(".999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999");
  assert_tokenizes_as_number("0.");
  assert_tokenizes_as_number("1.ᴇ-4");
  assert_tokenizes_as_number("1.ᴇ9999");

  const Token::Type binaryType[] = {Token::BinaryNumber, Token::EndOfStream};
  assert_tokenizes_as(binaryType, "0b0011010101");
  const Token::Type hexadecimalType[] = {Token::HexadecimalNumber, Token::EndOfStream};
  assert_tokenizes_as(hexadecimalType, "0x1234567890ABCDEF");

  assert_tokenizes_as_undefined_token("1ᴇ");
  assert_tokenizes_as_undefined_token("1..");
  assert_tokenizes_as_undefined_token("..");
  assert_tokenizes_as_undefined_token("1.ᴇᴇ");
  assert_tokenizes_as_undefined_token("1.ᴇ");
  assert_tokenizes_as_undefined_token("1ᴇ--4");
  assert_tokenizes_as_undefined_token("1.ᴇᴇ4");
  assert_tokenizes_as_undefined_token("1ᴇ2ᴇ4");
}

QUIZ_CASE(poincare_parsing_memory_exhaustion) {
#if __EMSCRIPTEN__
  /* We skip the following test on the web simulator, as it depends on a rollback
   * using a longjump in ExceptionCheckpoint::Raise. This longjump was removed
   * from the web implementation, as it cannot be made to work reliably in the
   * version of emscripten we depend on.
   * Fortuitously, this test used to succeed on the web simulator, but we cannot
   * rely on this behaviour. */
#else
  int initialPoolSize = pool_size();
  assert_parsed_expression_is("2+3",Addition::Builder(BasedInteger::Builder(2), BasedInteger::Builder(3)));
  assert_pool_size(initialPoolSize);

  int memoryFailureHasBeenHandled = false;
  {
    Poincare::ExceptionCheckpoint ecp;
    if (ExceptionRun(ecp)) {
      Addition a = Addition::Builder();
      while (true) {
        Expression e = Expression::Parse("1+2+3+4+5+6+7+8+9+10", nullptr);
        a.addChildAtIndexInPlace(e, 0, a.numberOfChildren());
      }
    } else {
      memoryFailureHasBeenHandled = true;
    }
  }

  quiz_assert(memoryFailureHasBeenHandled);
  assert_pool_size(initialPoolSize);
  Expression e = Expression::Parse("1+1", nullptr);
  /* Stupid check to make sure the global variable generated by Bison is not
   * ruining everything */
#endif
}

QUIZ_CASE(poincare_parsing_parse_numbers) {
  // Parse decimal
  assert_parsed_expression_is("0", BasedInteger::Builder(0));
  assert_parsed_expression_is("0.1", Decimal::Builder(0.1));
  assert_parsed_expression_is("1.", BasedInteger::Builder(1));
  assert_parsed_expression_is(".1", Decimal::Builder(0.1));
  assert_parsed_expression_is("0ᴇ2", Decimal::Builder(0.0));
  assert_parsed_expression_is("0.1ᴇ2", Decimal::Builder(10.0));
  assert_parsed_expression_is("1.ᴇ2", Decimal::Builder(100.0));
  assert_parsed_expression_is(".1ᴇ2", Decimal::Builder(10.0));
  assert_parsed_expression_is("0ᴇ-2", Decimal::Builder(0.0));
  assert_parsed_expression_is("0.1ᴇ-2", Decimal::Builder(0.001));
  assert_parsed_expression_is("1.ᴇ-2", Decimal::Builder(0.01));
  assert_parsed_expression_is(".1ᴇ-2", Decimal::Builder(0.001));
  // Decimal with rounding when digits are above 14
  assert_parsed_expression_is("0.0000012345678901234", Decimal::Builder(Integer("12345678901234"), -6));
  assert_parsed_expression_is("0.00000123456789012345", Decimal::Builder(Integer("12345678901235"), -6));
  assert_parsed_expression_is("0.00000123456789012341", Decimal::Builder(Integer("12345678901234"), -6));
  assert_parsed_expression_is("1234567890123.4", Decimal::Builder(Integer("12345678901234"), 12));
  assert_parsed_expression_is("123456789012345.2", Decimal::Builder(Integer("12345678901235"), 14));
  assert_parsed_expression_is("123456789012341.2", Decimal::Builder(Integer("12345678901234"), 14));
  assert_parsed_expression_is("12.34567", Decimal::Builder(Integer("1234567"), 1));
  assert_parsed_expression_is(".999999999999990", Decimal::Builder(Integer("99999999999999"), -1));
  assert_parsed_expression_is("9.99999999999994", Decimal::Builder(Integer("99999999999999"), 0));
  assert_parsed_expression_is("99.9999999999995", Decimal::Builder(Integer("100000000000000"), 2));
  assert_parsed_expression_is("999.999999999999", Decimal::Builder(Integer("100000000000000"), 3));
  assert_parsed_expression_is("9999.99199999999", Decimal::Builder(Integer("99999920000000"), 3));
  assert_parsed_expression_is("99299.9999999999", Decimal::Builder(Integer("99300000000000"), 4));

  // Parse integer
  assert_parsed_expression_is("123456789012345678765434567", BasedInteger::Builder("123456789012345678765434567"));
  assert_parsed_expression_is(MaxParsedIntegerString(), BasedInteger::Builder(MaxParsedIntegerString()));

  // Parsed Based integer
  assert_parsed_expression_is("0b1011", BasedInteger::Builder("1011", 4, Integer::Base::Binary));
  assert_parsed_expression_is("0x12AC", BasedInteger::Builder("12AC", 4, Integer::Base::Hexadecimal));

  // Integer parsed in Decimal because they overflow Integer
  assert_parsed_expression_is(ApproximatedParsedIntegerString(), Float<double>::Builder(1.0e30));

  // Infinity
  assert_parsed_expression_is("23ᴇ1000", Infinity::Builder(false));
  assert_parsed_expression_is("2.3ᴇ1000", Decimal::Builder(Integer(23), 1000));

  // Zero
  assert_parsed_expression_is("0.23ᴇ-1000", Decimal::Builder(Integer(0), 0));
  assert_parsed_expression_is("0.23ᴇ-999", Decimal::Builder(Integer(23), -1000));
}

QUIZ_CASE(poincare_parsing_parse) {
  assert_parsed_expression_is("1", BasedInteger::Builder(1));
  assert_parsed_expression_is("(1)", Parenthesis::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("((1))", Parenthesis::Builder((Expression)Parenthesis::Builder(BasedInteger::Builder(1))));
  assert_parsed_expression_is("1+2", Addition::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2)));
  assert_parsed_expression_is("(1)+2", Addition::Builder(Parenthesis::Builder(BasedInteger::Builder(1)),BasedInteger::Builder(2)));
  assert_parsed_expression_is("(1+2)", Parenthesis::Builder(Addition::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2))));
  Expression::Tuple one_two_three = {BasedInteger::Builder(1),BasedInteger::Builder(2),BasedInteger::Builder(3)};
  assert_parsed_expression_is("1+2+3", Addition::Builder(one_two_three));
  assert_parsed_expression_is("1+2+(3+4)", Addition::Builder({BasedInteger::Builder(1), BasedInteger::Builder(2), Parenthesis::Builder(Addition::Builder(BasedInteger::Builder(3),BasedInteger::Builder(4)))}));
  assert_parsed_expression_is("1×2", Multiplication::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2)));
  assert_parsed_expression_is("1×2×3", Multiplication::Builder(one_two_three));
  assert_parsed_expression_is("1+2×3", Addition::Builder(BasedInteger::Builder(1), Multiplication::Builder(BasedInteger::Builder(2), BasedInteger::Builder(3))));
  assert_parsed_expression_is("1/2", Division::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2)));
  assert_parsed_expression_is("(1/2)", Parenthesis::Builder(Division::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2))));
  assert_parsed_expression_is("1/2/3", Division::Builder(Division::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2)),BasedInteger::Builder(3)));
  assert_parsed_expression_is("1/2×3", Multiplication::Builder(Division::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2)),BasedInteger::Builder(3)));
  assert_parsed_expression_is("(1/2×3)", Parenthesis::Builder(Multiplication::Builder(Division::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2)),BasedInteger::Builder(3))));
  assert_parsed_expression_is("1×2/3", Multiplication::Builder(BasedInteger::Builder(1),Division::Builder(BasedInteger::Builder(2),BasedInteger::Builder(3))));
  assert_parsed_expression_is("(1×2/3)", Parenthesis::Builder(Multiplication::Builder(BasedInteger::Builder(1),Division::Builder(BasedInteger::Builder(2),BasedInteger::Builder(3)))));
  assert_parsed_expression_is("(1/2/3)", Parenthesis::Builder(Division::Builder(Division::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2)),BasedInteger::Builder(3))));
  assert_parsed_expression_is("1^2", Power::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2)));
  assert_parsed_expression_is("1^2^3", Power::Builder(BasedInteger::Builder(1),Power::Builder(BasedInteger::Builder(2),BasedInteger::Builder(3))));
  assert_parsed_expression_is("1=2", Equal::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2)));
  assert_text_not_parsable("=5");
  assert_text_not_parsable("1=2=3");
  assert_parsed_expression_is("-1", Opposite::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("(-1)", Parenthesis::Builder(Opposite::Builder(BasedInteger::Builder(1))));
  assert_parsed_expression_is("1-2", Subtraction::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2)));
  assert_parsed_expression_is("-1-2", Subtraction::Builder(Opposite::Builder(BasedInteger::Builder(1)),BasedInteger::Builder(2)));
  assert_parsed_expression_is("1-2-3", Subtraction::Builder(Subtraction::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2)),BasedInteger::Builder(3)));
  assert_parsed_expression_is("(1-2)", Parenthesis::Builder(Subtraction::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2))));
  assert_parsed_expression_is("1+-2", Addition::Builder(BasedInteger::Builder(1),Opposite::Builder(BasedInteger::Builder(2))));
  assert_parsed_expression_is("--1", Opposite::Builder((Expression)Opposite::Builder(BasedInteger::Builder(1))));
  assert_parsed_expression_is("(1+2)-3", Subtraction::Builder(Parenthesis::Builder(Addition::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2))),BasedInteger::Builder(3)));
  assert_parsed_expression_is("(2×-3)", Parenthesis::Builder(Multiplication::Builder(BasedInteger::Builder(2),Opposite::Builder(BasedInteger::Builder(3)))));
  assert_parsed_expression_is("1^(2)-3", Subtraction::Builder(Power::Builder(BasedInteger::Builder(1),Parenthesis::Builder(BasedInteger::Builder(2))),BasedInteger::Builder(3)));
  assert_parsed_expression_is("1^2-3", Subtraction::Builder(Power::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2)),BasedInteger::Builder(3)));
  assert_parsed_expression_is("2^-3", Power::Builder(BasedInteger::Builder(2),Opposite::Builder(BasedInteger::Builder(3))));
  assert_parsed_expression_is("2--2+-1", Addition::Builder(Subtraction::Builder(BasedInteger::Builder(2),Opposite::Builder(BasedInteger::Builder(2))),Opposite::Builder(BasedInteger::Builder(1))));
  assert_parsed_expression_is("2--2×-1", Subtraction::Builder(BasedInteger::Builder(2),Opposite::Builder(Multiplication::Builder(BasedInteger::Builder(2),Opposite::Builder(BasedInteger::Builder(1))))));
  assert_parsed_expression_is("-1^2", Opposite::Builder(Power::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2))));
  assert_parsed_expression_is("2e^(3)", Multiplication::Builder(BasedInteger::Builder(2),Power::Builder(Constant::Builder("e"),Parenthesis::Builder(BasedInteger::Builder(3)))));
  assert_parsed_expression_is("2/-3/-4", Division::Builder(Division::Builder(BasedInteger::Builder(2),Opposite::Builder(BasedInteger::Builder(3))),Opposite::Builder(BasedInteger::Builder(4))));
  assert_parsed_expression_is("1×2-3×4", Subtraction::Builder(Multiplication::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2)),Multiplication::Builder(BasedInteger::Builder(3),BasedInteger::Builder(4))));
  assert_parsed_expression_is("-1×2", Opposite::Builder(Multiplication::Builder(BasedInteger::Builder(1), BasedInteger::Builder(2))));
  assert_parsed_expression_is("1!", Factorial::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("1+2!", Addition::Builder(BasedInteger::Builder(1),Factorial::Builder(BasedInteger::Builder(2))));
  assert_parsed_expression_is("1!+2", Addition::Builder(Factorial::Builder(BasedInteger::Builder(1)),BasedInteger::Builder(2)));
  assert_parsed_expression_is("1!+2!", Addition::Builder(Factorial::Builder(BasedInteger::Builder(1)),Factorial::Builder(BasedInteger::Builder(2))));
  assert_parsed_expression_is("1×2!", Multiplication::Builder(BasedInteger::Builder(1),Factorial::Builder(BasedInteger::Builder(2))));
  assert_parsed_expression_is("1!×2", Multiplication::Builder(Factorial::Builder(BasedInteger::Builder(1)),BasedInteger::Builder(2)));
  assert_parsed_expression_is("1!×2!", Multiplication::Builder(Factorial::Builder(BasedInteger::Builder(1)),Factorial::Builder(BasedInteger::Builder(2))));
  assert_parsed_expression_is("1-2!", Subtraction::Builder(BasedInteger::Builder(1),Factorial::Builder(BasedInteger::Builder(2))));
  assert_parsed_expression_is("1!-2", Subtraction::Builder(Factorial::Builder(BasedInteger::Builder(1)),BasedInteger::Builder(2)));
  assert_parsed_expression_is("1!-2!", Subtraction::Builder(Factorial::Builder(BasedInteger::Builder(1)),Factorial::Builder(BasedInteger::Builder(2))));
  assert_parsed_expression_is("1/2!", Division::Builder(BasedInteger::Builder(1),Factorial::Builder(BasedInteger::Builder(2))));
  assert_parsed_expression_is("1!/2", Division::Builder(Factorial::Builder(BasedInteger::Builder(1)),BasedInteger::Builder(2)));
  assert_parsed_expression_is("1!/2!", Division::Builder(Factorial::Builder(BasedInteger::Builder(1)),Factorial::Builder(BasedInteger::Builder(2))));
  assert_parsed_expression_is("1^2!", Power::Builder(BasedInteger::Builder(1),Factorial::Builder(BasedInteger::Builder(2))));
  assert_parsed_expression_is("1!^2", Power::Builder(Factorial::Builder(BasedInteger::Builder(1)),BasedInteger::Builder(2)));
  assert_parsed_expression_is("1!^2!", Power::Builder(Factorial::Builder(BasedInteger::Builder(1)),Factorial::Builder(BasedInteger::Builder(2))));
  assert_parsed_expression_is("(1)!", Factorial::Builder(Parenthesis::Builder(BasedInteger::Builder(1))));
  assert_text_not_parsable("1+");
  assert_text_not_parsable(")");
  assert_text_not_parsable(")(");
  assert_text_not_parsable("()");
  assert_text_not_parsable("(1");
  assert_text_not_parsable("1)");
  assert_text_not_parsable("1++2");
  assert_text_not_parsable("1//2");
  assert_text_not_parsable("×1");
  assert_text_not_parsable("1^^2");
  assert_text_not_parsable("^1");
  assert_text_not_parsable("t0000000");
  assert_text_not_parsable("[[t0000000[");
  assert_text_not_parsable("0→x=0");
  assert_text_not_parsable("0→3=0");
  assert_text_not_parsable("0=0→x");
  assert_text_not_parsable("1ᴇ2ᴇ3");
  assert_text_not_parsable("0b001112");
  assert_text_not_parsable("0x123G");
}

Matrix BuildMatrix(int rows, int columns, Expression entries[]) {
  Matrix m = Matrix::Builder();
  int position = 0;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      m.addChildAtIndexInPlace(entries[position], position, position);
      position++;
    }
  }
  m.setDimensions(rows, columns);
  return m;
}

QUIZ_CASE(poincare_parsing_matrices) {
  Expression m1[] = {BasedInteger::Builder(1)};
  assert_parsed_expression_is("[[1]]", BuildMatrix(1,1,m1));
  Expression m2[] = {BasedInteger::Builder(1),BasedInteger::Builder(2),BasedInteger::Builder(3)};
  assert_parsed_expression_is("[[1,2,3]]", BuildMatrix(1,3,m2));
  Expression m3[] = {BasedInteger::Builder(1),BasedInteger::Builder(2),BasedInteger::Builder(3),BasedInteger::Builder(4),BasedInteger::Builder(5),BasedInteger::Builder(6)};
  assert_parsed_expression_is("[[1,2,3][4,5,6]]", BuildMatrix(2,3,m3));
  Expression m4[] = {BasedInteger::Builder(1), BuildMatrix(1,1,m1)};
  assert_parsed_expression_is("[[1,[[1]]]]", BuildMatrix(1,2,m4));
  assert_text_not_parsable("[");
  assert_text_not_parsable("]");
  assert_text_not_parsable("[[");
  assert_text_not_parsable("][");
  assert_text_not_parsable("[]");
  assert_text_not_parsable("[1]");
  assert_text_not_parsable("[[1,2],[3]]");
  assert_text_not_parsable("[[]");
  assert_text_not_parsable("[[1]");
  assert_text_not_parsable("[1]]");
  assert_text_not_parsable("[[,]]");
  assert_text_not_parsable("[[1,]]");
  assert_text_not_parsable(",");
  assert_text_not_parsable("[,]");
}

template<size_t N>
List BuildList(Expression (&elements)[N]) {
  List l = List::Builder();
  for (int i = 0; i < N; i++) {
    l.addChildAtIndexInPlace(elements[i], i, i);
  }
  return l;
}

QUIZ_CASE(poincare_parse_lists) {
  assert_parsed_expression_is("{}", List::Builder());
  {
    Expression elements[] = { BasedInteger::Builder(1) };
    assert_parsed_expression_is("{1}", BuildList(elements));
  }
  {
    Expression elements[] = { BasedInteger::Builder(1), BasedInteger::Builder(2), BasedInteger::Builder(3) };
    assert_parsed_expression_is("{1,2,3}", BuildList(elements));
  }
  {
    Expression inner[] = { BasedInteger::Builder(2), BasedInteger::Builder(3) };
    Expression outer[] = { BasedInteger::Builder(1), BuildList(inner), BasedInteger::Builder(4) };
    assert_parsed_expression_is("{1,{2,3},4}", BuildList(outer));
  }
  assert_text_not_parsable("{");
  assert_text_not_parsable("{{");
  assert_text_not_parsable("}");
  assert_text_not_parsable("}}");
  assert_text_not_parsable("}{");
  assert_text_not_parsable("{,}");
  assert_text_not_parsable("{1,2,}");
  assert_text_not_parsable("{1,,3}");
  assert_text_not_parsable("{,2,3}");
}

QUIZ_CASE(poincare_parsing_constants) {
  for (ConstantNode::ConstantInfo info : Constant::k_constants) {
    assert_tokenizes_as_constant(info.name());
  }
}

QUIZ_CASE(poincare_parsing_units) {
  // Units
  for (int i = 0; i < Unit::Representative::k_numberOfDimensions; i++) {
    const Unit::Representative * dim = Unit::Representative::DefaultRepresentatives()[i];
    for (int j = 0; j < dim->numberOfRepresentatives(); j++) {
      const Unit::Representative * rep = dim->representativesOfSameDimension() + j;
      static constexpr size_t bufferSize = 10;
      char buffer[bufferSize];
      Unit::Builder(rep, Unit::Prefix::EmptyPrefix()).serialize(buffer, bufferSize, Preferences::PrintFloatMode::Decimal, Preferences::VeryShortNumberOfSignificantDigits);
      Expression unit = parse_expression(buffer, nullptr, false);
      quiz_assert_print_if_failure(unit.type() == ExpressionNode::Type::Unit, "Should be parsed as a Unit");
      if (rep->isInputPrefixable()) {
        for (size_t i = 0; i < Unit::Prefix::k_numberOfPrefixes; i++) {
          const Unit::Prefix * pre = Unit::Prefix::Prefixes();
          Unit::Builder(rep, pre).serialize(buffer, bufferSize, Preferences::PrintFloatMode::Decimal, Preferences::VeryShortNumberOfSignificantDigits);
          Expression unit = parse_expression(buffer, nullptr, false);
          quiz_assert_print_if_failure(unit.type() == ExpressionNode::Type::Unit, "Should be parsed as a Unit");
        }
      }
    }
  }

  // Non-existing units are not parsable
  assert_text_not_parsable("_n");
  assert_text_not_parsable("_a");

  // Any identifier starting with '_' is tokenized as a unit
  assert_tokenizes_as_unit("_m");
  assert_tokenizes_as_unit("_A");

  // Can parse implicit multiplication with units
  Expression kilometer = Expression::Parse("_km", nullptr);
  Expression second = Expression::Parse("_s", nullptr);
  assert_parsed_expression_is("_kmπ", Multiplication::Builder(kilometer, Constant::Builder("π")));
  assert_parsed_expression_is("π_km", Multiplication::Builder(Constant::Builder("π"), kilometer));
  assert_parsed_expression_is("_s_km", Multiplication::Builder(second, kilometer));
  assert_parsed_expression_is("3_s", Multiplication::Builder(BasedInteger::Builder(3), second));
}

QUIZ_CASE(poincare_parsing_identifiers) {
  // Custom variables without storage
  assert_parsed_expression_is("a", Symbol::Builder("a", 1));
  assert_parsed_expression_is("x", Symbol::Builder("x", 1));
  assert_parsed_expression_is("\"toot\"", Symbol::Builder("\"toot\"", 6));
  assert_parsed_expression_is("\"tot12\"", Symbol::Builder("\"tot12\"", 7));
  assert_parsed_expression_is("f(f)", Multiplication::Builder(Symbol::Builder("f", 1), Parenthesis::Builder(Symbol::Builder("f", 1))));
  assert_parsed_expression_is("f((1))", Multiplication::Builder(Symbol::Builder("f", 1), Parenthesis::Builder( Parenthesis::Builder(BasedInteger::Builder(1)))));
  assert_text_not_parsable("_a");
  assert_text_not_parsable("abcdefgh");
  assert_text_not_parsable("f(1,2,3)");

  // User-defined functions
  assert_parsed_expression_is("1→f(x)", Store::Builder(BasedInteger::Builder(1), Function::Builder("f", 1, Symbol::Builder("x",1))));
  assert_parsed_expression_is("1→ab12AB_(x)", Store::Builder(BasedInteger::Builder(1), Function::Builder("ab12AB_", 7, Symbol::Builder("x",1))));

  // Reserved symbols
  assert_parsed_expression_is("Ans", Symbol::Builder("Ans", 3));
  assert_parsed_expression_is("ans", Symbol::Builder("Ans", 3));
  assert_parsed_expression_is("i", Constant::Builder("i"));
  assert_parsed_expression_is("π", Constant::Builder("π"));
  assert_parsed_expression_is("e", Constant::Builder("e"));
  assert_parsed_expression_is(Infinity::Name(), Infinity::Builder(false));
  assert_parsed_expression_is(Undefined::Name(), Undefined::Builder());

  assert_text_not_parsable("u");
  assert_text_not_parsable("v");

  // Reserved functions
  assert_parsed_expression_is("acos(1)", ArcCosine::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("acosh(1)", HyperbolicArcCosine::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("acot(1)", ArcCotangent::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("acsc(1)", ArcCosecant::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("abs(1)", AbsoluteValue::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("arg(1)", ComplexArgument::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("asec(1)", ArcSecant::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("asin(1)", ArcSine::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("asinh(1)", HyperbolicArcSine::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("atan(1)", ArcTangent::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("atanh(1)", HyperbolicArcTangent::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("binomial(2,1)", BinomialCoefficient::Builder(BasedInteger::Builder(2),BasedInteger::Builder(1)));
  assert_parsed_expression_is("ceil(1)", Ceiling::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("cross(1,1)", VectorCross::Builder(BasedInteger::Builder(1),BasedInteger::Builder(1)));
  assert_text_not_parsable("diff(1,2,3)");
  assert_text_not_parsable("diff(0,_s,0)");
  assert_parsed_expression_is("diff(1,x,3)", Derivative::Builder(BasedInteger::Builder(1),Symbol::Builder("x",1),BasedInteger::Builder(3)));
  assert_parsed_expression_is("dim(1)", Dimension::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("conj(1)", Conjugate::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("cot(1)", Cotangent::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("det(1)", Determinant::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("dot(1,1)", VectorDot::Builder(BasedInteger::Builder(1),BasedInteger::Builder(1)));
  assert_parsed_expression_is("cos(1)", Cosine::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("cosh(1)", HyperbolicCosine::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("csc(1)", Cosecant::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("factor(1)", Factor::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("floor(1)", Floor::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("frac(1)", FracPart::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("gcd(1,2,3)", GreatCommonDivisor::Builder({BasedInteger::Builder(1),BasedInteger::Builder(2),BasedInteger::Builder(3)}));
  assert_text_not_parsable("gcd(1)");
  assert_text_not_parsable("gcd()");
  assert_parsed_expression_is("im(1)", ImaginaryPart::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("int(1,x,2,3)", Integral::Builder(BasedInteger::Builder(1),Symbol::Builder("x",1),BasedInteger::Builder(2),BasedInteger::Builder(3)));
  assert_text_not_parsable("int(1,2,3,4)");
  assert_text_not_parsable("int(1,_s,3,4)");
  assert_parsed_expression_is("inverse(1)", MatrixInverse::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("lcm(1,2,3)", LeastCommonMultiple::Builder({BasedInteger::Builder(1),BasedInteger::Builder(2),BasedInteger::Builder(3)}));
  assert_text_not_parsable("lcm(1)");
  assert_text_not_parsable("lcm()");
  assert_parsed_expression_is("ln(1)", NaperianLogarithm::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("log(1)", CommonLogarithm::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("log(1,2)", Logarithm::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2)));
  assert_parsed_expression_is("log{2}(1)", Logarithm::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2)));
  assert_parsed_expression_is("permute(2,1)", PermuteCoefficient::Builder(BasedInteger::Builder(2),BasedInteger::Builder(1)));
  assert_parsed_expression_is("norm(1)", VectorNorm::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("product(1,n,2,3)", Product::Builder(BasedInteger::Builder(1),Symbol::Builder("n",1),BasedInteger::Builder(2),BasedInteger::Builder(3)));
  assert_text_not_parsable("product(1,2,3,4)");
  assert_text_not_parsable("product(1,_s,3,4)");
  assert_parsed_expression_is("quo(1,2)", DivisionQuotient::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2)));
  assert_parsed_expression_is("random()", Random::Builder());
  assert_parsed_expression_is("randint(1,2)", Randint::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2)));
  assert_parsed_expression_is("re(1)", RealPart::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("ref(1)", MatrixRowEchelonForm::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("rem(1,2)", DivisionRemainder::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2)));
  assert_parsed_expression_is("root(1,2)", NthRoot::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2)));
  assert_parsed_expression_is("round(1,2)", Round::Builder(BasedInteger::Builder(1),BasedInteger::Builder(2)));
  assert_parsed_expression_is("rref(1)", MatrixReducedRowEchelonForm::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("sec(1)", Secant::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("sin(1)", Sine::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("sign(1)", SignFunction::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("sinh(1)", HyperbolicSine::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("sum(1,n,2,3)", Sum::Builder(BasedInteger::Builder(1),Symbol::Builder("n",1),BasedInteger::Builder(2),BasedInteger::Builder(3)));
  assert_text_not_parsable("sum(1,2,3,4)");
  assert_text_not_parsable("sum(1,_s,3,4)");
  assert_parsed_expression_is("tan(1)", Tangent::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("tanh(1)", HyperbolicTangent::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("trace(1)", MatrixTrace::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("transpose(1)", MatrixTranspose::Builder(BasedInteger::Builder(1)));
  assert_parsed_expression_is("√(1)", SquareRoot::Builder(BasedInteger::Builder(1)));
  assert_text_not_parsable("cos(1,2)");
  assert_text_not_parsable("log(1,2,3)");

  // Custom identifiers with storage
  Ion::Storage::sharedStorage()->createRecordWithExtension("ab", "exp", "", 0);
  Ion::Storage::sharedStorage()->createRecordWithExtension("bacos", "func", "", 0);
  Ion::Storage::sharedStorage()->createRecordWithExtension("azfoo", "exp", "", 0);
  Ion::Storage::sharedStorage()->createRecordWithExtension("foobar", "func", "", 0);
  assert_parsed_expression_is("xyz", Multiplication::Builder(Symbol::Builder("x", 1), Multiplication::Builder(Symbol::Builder("y", 1), Symbol::Builder("z", 1))));
  assert_parsed_expression_is("3→xyz", Store::Builder(BasedInteger::Builder(3), Symbol::Builder("xyz", 3)));
  assert_parsed_expression_is("ab", Symbol::Builder("ab", 2));
  assert_parsed_expression_is("aacos(x)", Multiplication::Builder(Symbol::Builder("a", 1), ArcCosine::Builder(Symbol::Builder("x", 1))));
  assert_parsed_expression_is("bacos(x)", Function::Builder("bacos", 5, Symbol::Builder("x", 1)));
  assert_parsed_expression_is("azfoobar(x)", Multiplication::Builder(Symbol::Builder("a", 1), Multiplication::Builder(Symbol::Builder("z", 1), Function::Builder("foobar", 6, Symbol::Builder("x", 1)))));
  Ion::Storage::sharedStorage()->destroyAllRecords();
}

QUIZ_CASE(poincare_parsing_parse_store) {
  assert_parsed_expression_is("1→a", Store::Builder(BasedInteger::Builder(1),Symbol::Builder("a",1)));
  assert_parsed_expression_is("1→f(x)", Store::Builder(BasedInteger::Builder(1),Function::Builder("f",1,Symbol::Builder("x",1))));
  assert_parsed_expression_is("x→f(x)", Store::Builder(Symbol::Builder("x",1),Function::Builder("f",1,Symbol::Builder("x",1))));
  assert_parsed_expression_is("n→f(x)", Store::Builder(Symbol::Builder("n",1),Function::Builder("f",1,Symbol::Builder("x",1))));
  Expression m0[] = {Symbol::Builder('x')};
  assert_parsed_expression_is("[[x]]→f(x)", Store::Builder(BuildMatrix(1,1,m0), Function::Builder("f", 1, Symbol::Builder('x'))));
  assert_text_not_parsable("a→b→c");
  assert_text_not_parsable("0→0→c");
  assert_text_not_parsable("1→");
  assert_text_not_parsable("→2");
  assert_text_not_parsable("(1→a)");
  assert_text_not_parsable("1→\1"); // UnknownX
  assert_text_not_parsable("1→\2"); // UnknownN
  assert_text_not_parsable("1→acos");
  assert_text_not_parsable("1→f(f)");
}

QUIZ_CASE(poincare_parsing_parse_unit_convert) {
  Expression meter = Expression::Parse("_m", nullptr);
  assert_parsed_expression_is("1→_m", UnitConvert::Builder(BasedInteger::Builder(1), meter));
  Expression kilometer = Expression::Parse("_km", nullptr);
  assert_parsed_expression_is("1→_m/_km", UnitConvert::Builder(BasedInteger::Builder(1), Division::Builder(meter, kilometer)));
}

QUIZ_CASE(poincare_parsing_implicit_multiplication) {
  assert_text_not_parsable(".1.2");
  assert_text_not_parsable("1 2");
  assert_parsed_expression_is("1x", Multiplication::Builder(BasedInteger::Builder(1),Symbol::Builder("x", 1)));
  assert_parsed_expression_is("1Ans", Multiplication::Builder(BasedInteger::Builder(1),Symbol::Builder("Ans", 3)));
  // Fallback from binary number
  assert_parsed_expression_is("0b2", Multiplication::Builder(BasedInteger::Builder(0),Multiplication::Builder(Symbol::Builder("b", 1),BasedInteger::Builder(2))));
  assert_parsed_expression_is("0xG", Multiplication::Builder(BasedInteger::Builder(0),Multiplication::Builder(Symbol::Builder("x", 1),Symbol::Builder("G", 1))));
  assert_parsed_expression_is("1x+2", Addition::Builder(Multiplication::Builder(BasedInteger::Builder(1),Symbol::Builder("x", 1)),BasedInteger::Builder(2)));
  assert_parsed_expression_is("1π", Multiplication::Builder(BasedInteger::Builder(1),Constant::Builder("π")));
  assert_parsed_expression_is("1x-2", Subtraction::Builder(Multiplication::Builder(BasedInteger::Builder(1),Symbol::Builder("x", 1)),BasedInteger::Builder(2)));
  assert_parsed_expression_is("-1x", Opposite::Builder(Multiplication::Builder(BasedInteger::Builder(1),Symbol::Builder("x", 1))));
  assert_parsed_expression_is("2×1x", Multiplication::Builder(BasedInteger::Builder(2),Multiplication::Builder(BasedInteger::Builder(1),Symbol::Builder("x", 1))));
  assert_parsed_expression_is("2^1x", Multiplication::Builder(Power::Builder(BasedInteger::Builder(2),BasedInteger::Builder(1)),Symbol::Builder("x", 1)));
  assert_parsed_expression_is("1x^2", Multiplication::Builder(BasedInteger::Builder(1),Power::Builder(Symbol::Builder("x", 1),BasedInteger::Builder(2))));
  assert_parsed_expression_is("2/1x", Division::Builder(BasedInteger::Builder(2),Multiplication::Builder(BasedInteger::Builder(1),Symbol::Builder("x", 1))));
  assert_parsed_expression_is("1x/2", Division::Builder(Multiplication::Builder(BasedInteger::Builder(1),Symbol::Builder("x", 1)),BasedInteger::Builder(2)));
  assert_parsed_expression_is("(1)2", Multiplication::Builder(Parenthesis::Builder(BasedInteger::Builder(1)),BasedInteger::Builder(2)));
  assert_parsed_expression_is("1(2)", Multiplication::Builder(BasedInteger::Builder(1),Parenthesis::Builder(BasedInteger::Builder(2))));
  assert_parsed_expression_is("sin(1)2", Multiplication::Builder(Sine::Builder(BasedInteger::Builder(1)),BasedInteger::Builder(2)));
  assert_parsed_expression_is("1cos(2)", Multiplication::Builder(BasedInteger::Builder(1),Cosine::Builder(BasedInteger::Builder(2))));
  assert_parsed_expression_is("1!2", Multiplication::Builder(Factorial::Builder(BasedInteger::Builder(1)),BasedInteger::Builder(2)));
  assert_parsed_expression_is("2e^(3)", Multiplication::Builder(BasedInteger::Builder(2),Power::Builder(Constant::Builder("e"),Parenthesis::Builder(BasedInteger::Builder(3)))));
  assert_parsed_expression_is("\u00122^3\u00133", Multiplication::Builder(Power::Builder(BasedInteger::Builder(2),BasedInteger::Builder(3)), BasedInteger::Builder(3)));
  Expression m1[] = {BasedInteger::Builder(1)}; Matrix M1 = BuildMatrix(1,1,m1);
  Expression m2[] = {BasedInteger::Builder(2)}; Matrix M2 = BuildMatrix(1,1,m2);
  assert_parsed_expression_is("[[1]][[2]]", Multiplication::Builder(M1,M2));
  Expression l1[] = {BasedInteger::Builder(1), BasedInteger::Builder(2)};
  assert_parsed_expression_is("2{1,2}", Multiplication::Builder(BasedInteger::Builder(2), BuildList(l1)));
  Expression l2[] = {BasedInteger::Builder(1), BasedInteger::Builder(2)};
  assert_parsed_expression_is("{1,2}2", Multiplication::Builder(BuildList(l2), BasedInteger::Builder(2)));
}

QUIZ_CASE(poincare_parsing_adding_missing_parentheses) {
  assert_parsed_expression_with_user_parentheses_is("1+-2", Addition::Builder(BasedInteger::Builder(1),Parenthesis::Builder(Opposite::Builder(BasedInteger::Builder(2)))));
  assert_parsed_expression_with_user_parentheses_is("1--2", Subtraction::Builder(BasedInteger::Builder(1),Parenthesis::Builder(Opposite::Builder(BasedInteger::Builder(2)))));
  assert_parsed_expression_with_user_parentheses_is("1+conj(-2)", Addition::Builder(BasedInteger::Builder(1),Parenthesis::Builder(Conjugate::Builder(Opposite::Builder(BasedInteger::Builder(2))))));
  assert_parsed_expression_with_user_parentheses_is("1-conj(-2)", Subtraction::Builder(BasedInteger::Builder(1),Parenthesis::Builder(Conjugate::Builder(Opposite::Builder(BasedInteger::Builder(2))))));
  assert_parsed_expression_with_user_parentheses_is("3conj(1+i)", Multiplication::Builder(BasedInteger::Builder(3), Parenthesis::Builder(Conjugate::Builder(Addition::Builder(BasedInteger::Builder(1), Constant::Builder("i"))))));
  assert_parsed_expression_with_user_parentheses_is("2×-3", Multiplication::Builder(BasedInteger::Builder(2), Parenthesis::Builder(Opposite::Builder(BasedInteger::Builder(3)))));
  assert_parsed_expression_with_user_parentheses_is("2×-3", Multiplication::Builder(BasedInteger::Builder(2), Parenthesis::Builder(Opposite::Builder(BasedInteger::Builder(3)))));
  assert_parsed_expression_with_user_parentheses_is("--2", Opposite::Builder(Parenthesis::Builder(Opposite::Builder(BasedInteger::Builder(2)))));
  assert_parsed_expression_with_user_parentheses_is("\u00122/3\u0013^2", Power::Builder(Parenthesis::Builder(Division::Builder(BasedInteger::Builder(2), BasedInteger::Builder(3))), BasedInteger::Builder(2)));
  assert_parsed_expression_with_user_parentheses_is("log(1+-2)", CommonLogarithm::Builder(Addition::Builder(BasedInteger::Builder(1),Parenthesis::Builder(Opposite::Builder(BasedInteger::Builder(2))))));

  // Conjugate expressions
  assert_parsed_expression_with_user_parentheses_is("conj(-3)+2", Addition::Builder(Conjugate::Builder(Opposite::Builder(BasedInteger::Builder(3))), BasedInteger::Builder(2)));
  assert_parsed_expression_with_user_parentheses_is("2+conj(-3)", Addition::Builder(BasedInteger::Builder(2), Parenthesis::Builder(Conjugate::Builder(Opposite::Builder(BasedInteger::Builder(3))))));
  assert_parsed_expression_with_user_parentheses_is("conj(-3)×2", Multiplication::Builder(Conjugate::Builder(Opposite::Builder(BasedInteger::Builder(3))), BasedInteger::Builder(2)));
  assert_parsed_expression_with_user_parentheses_is("2×conj(-3)", Multiplication::Builder(BasedInteger::Builder(2), Parenthesis::Builder(Conjugate::Builder(Opposite::Builder(BasedInteger::Builder(3))))));
  assert_parsed_expression_with_user_parentheses_is("conj(-3)-2", Subtraction::Builder(Conjugate::Builder(Opposite::Builder(BasedInteger::Builder(3))), BasedInteger::Builder(2)));
  assert_parsed_expression_with_user_parentheses_is("2-conj(-3)", Subtraction::Builder(BasedInteger::Builder(2), Parenthesis::Builder(Conjugate::Builder(Opposite::Builder(BasedInteger::Builder(3))))));
  assert_parsed_expression_with_user_parentheses_is("conj(2+3)^2", Power::Builder(Parenthesis::Builder(Conjugate::Builder(Addition::Builder(BasedInteger::Builder(2), BasedInteger::Builder(3)))), BasedInteger::Builder(2)));
  assert_parsed_expression_with_user_parentheses_is("-conj(2+3)", Opposite::Builder(Parenthesis::Builder(Conjugate::Builder(Addition::Builder(BasedInteger::Builder(2), BasedInteger::Builder(3))))));
  assert_parsed_expression_with_user_parentheses_is("conj(2+3)!", Factorial::Builder(Parenthesis::Builder(Conjugate::Builder(Addition::Builder(BasedInteger::Builder(2), BasedInteger::Builder(3))))));
}
