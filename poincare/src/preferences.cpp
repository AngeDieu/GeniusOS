#include <poincare/preferences.h>
#include <poincare/print_float.h>

namespace Poincare {

constexpr int Preferences::LargeNumberOfSignificantDigits;
constexpr int Preferences::MediumNumberOfSignificantDigits;
constexpr int Preferences::ShortNumberOfSignificantDigits;

Preferences::Preferences() :
  m_angleUnit(AngleUnit::Radian),
  m_displayMode(Preferences::PrintFloatMode::Decimal),
  m_editionMode(EditionMode::Edition2D),
  m_complexFormat(Preferences::ComplexFormat::Cartesian),
  m_numberOfSignificantDigits(PrintFloat::k_numberOfPrintedSignificantDigits),
  m_symbolMultiplication(Preferences::SymbolMultiplication::Auto),
  m_symbolFunction(Preferences::SymbolFunction::ArgDefault),
  m_pythonFont(Preferences::PythonFont::Small)
{}

Preferences * Preferences::sharedPreferences() {
  static Preferences preferences;
  return &preferences;
}

}
