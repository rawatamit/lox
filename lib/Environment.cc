#include "Environment.h"
#include "Utils.h"
#include "RuntimeException.h"

lox::Environment* lox::Environment::ancestor(int depth)
{
  Environment* env = this;

  for (int i = 0; i < depth; ++i)
  {
    env = env->enclosing.get();
  }
  
  return env;
}

int lox::Environment::depth() const
{
  int depth = 0;
  const Environment* current = this;

  while (current != nullptr)
  {
    current = current->enclosing.get();
    ++depth;
  }

  return depth;
}

void lox::Environment::define(std::string k, std::any v)
{
  values[k] = v;
}

void lox::Environment::assign(lox::Token name, std::any v)
{
  Environment* current = this;

  while (current != nullptr)
  {
    auto it = current->values.find(name.lexeme);
    if (it != current->values.end())
    {
      it->second = v;
      return;
    }
    else
    {
      current = current->enclosing.get();
    }
  }

  throw RuntimeException(name,
      "Undefined variable '" + name.lexeme + "'.");
}

std::any lox::Environment::get(lox::Token name)
{
  Environment* current = this;

  while (current != nullptr)
  {
    auto it = current->values.find(name.lexeme);
    if (it != current->values.end())
    {
      return it->second;
    }
    else
    {
      current = current->enclosing.get();
    }
  }

  throw RuntimeException(name,
      "Undefined variable '" + name.lexeme + "'.");
}

std::any lox::Environment::get(int depth, const lox::Token& name)
{
  auto it = ancestor(depth)->values.find(name.lexeme);
  return it->second;
}

void lox::Environment::assign(int depth, const lox::Token& name, std::any v)
{
  ancestor(depth)->values[name.lexeme] = v;
}

void lox::Environment::print(std::ostream& out)
{
  int level = 0;
  auto current = this;

  while (current != nullptr)
  {
    for (auto& e : current->values)
    {
      out << std::string(2*level, ' ')
          << "level=" << level << ':' << e.first
          << ':' << stringify(e.second) << '\n';
    }

    ++level;
    current = current->enclosing.get();
  }
}
