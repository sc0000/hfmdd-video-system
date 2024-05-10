#pragma once

enum class ELanguage
{
  English,
  German,
  Default
};

class Translatable
{
public:
  virtual void translate(ELanguage language) = 0;
};