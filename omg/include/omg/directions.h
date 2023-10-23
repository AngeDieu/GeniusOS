#ifndef OMG_DIRECTIONS_H
#define OMG_DIRECTIONS_H

#include <assert.h>
#include <ion/events.h>

namespace OMG {

class Direction {
 public:
  Direction(Ion::Events::Event event);

  bool isLeft() const { return m_tag == Tag::Left; }
  bool isRight() const { return m_tag == Tag::Right; }
  bool isUp() const { return m_tag == Tag::Up; }
  bool isDown() const { return m_tag == Tag::Down; }

  bool isHorizontal() const {
    return m_tag == Tag::Left || m_tag == Tag::Right;
  }
  bool isVertical() const { return m_tag == Tag::Up || m_tag == Tag::Down; }

  operator class HorizontalDirection() const;
  operator class VerticalDirection() const;

  constexpr static Direction Left() { return Direction(Tag::Left); }
  constexpr static Direction Right() { return Direction(Tag::Right); }
  constexpr static Direction Up() { return Direction(Tag::Up); }
  constexpr static Direction Down() { return Direction(Tag::Down); }

  bool operator==(const Direction &other) const { return m_tag == other.m_tag; }
  bool operator!=(const Direction &other) const { return m_tag != other.m_tag; }

 private:
  enum class Tag { Left, Right, Up, Down };

  constexpr Direction(Tag tag) : m_tag(tag) {}

  Tag m_tag;
};

class HorizontalDirection : public Direction {
 private:
  using Direction::Down;
  using Direction::Up;
};

class VerticalDirection : public Direction {
 private:
  using Direction::Left;
  using Direction::Right;
};

}  // namespace OMG

#endif
