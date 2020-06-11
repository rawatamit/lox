#include "Environment.h"
#include "RuntimeException.h"

lox::Environment *lox::Environment::ancestor(int depth) {
  Environment *env = this;

  for (int i = 0; i < depth; ++i) {
    env = env->enclosing.get();
  }

  return env;
}

std::shared_ptr<lox::Environment> lox::Environment::getEnclosing() const {
  return enclosing;
}

int lox::Environment::depth() const {
  int depth = 0;
  const Environment *current = this;

  while (current != nullptr) {
    current = current->enclosing.get();
    ++depth;
  }

  return depth;
}

void lox::Environment::define(std::string k, std::shared_ptr<LoxObject> v) {
  values[k] = v;
}

void lox::Environment::assign(lox::Token name, std::shared_ptr<LoxObject> v) {
  Environment *current = this;

  while (current != nullptr) {
    auto it = current->values.find(name.lexeme);
    if (it != current->values.end()) {
      it->second = v;
      return;
    } else {
      current = current->enclosing.get();
    }
  }

  throw RuntimeException(name, "Undefined variable '" + name.lexeme + "'.");
}

std::shared_ptr<lox::LoxObject> lox::Environment::get(lox::Token name) {
  Environment *current = this;

  while (current != nullptr) {
    auto it = current->values.find(name.lexeme);
    if (it != current->values.end()) {
      return it->second;
    } else {
      current = current->enclosing.get();
    }
  }

  throw RuntimeException(name, "Undefined variable '" + name.lexeme + "'.");
}

std::shared_ptr<lox::LoxObject> lox::Environment::get(int depth,
                                                      const lox::Token &name) {
  return get(depth, name.lexeme);
}

std::shared_ptr<lox::LoxObject> lox::Environment::get(int depth,
                                                      const std::string &name) {
  auto it = ancestor(depth)->values.find(name);
  return it->second;
}

void lox::Environment::assign(int depth, const lox::Token &name,
                              std::shared_ptr<LoxObject> v) {
  ancestor(depth)->values[name.lexeme] = v;
}

void lox::Environment::print(std::ostream &out) {
  int level = 0;
  auto current = this;

  while (current != nullptr) {
    for (auto &e : current->values) {
      out << std::string(2 * level, ' ') << "level=" << level << ':' << e.first
          << ':' << e.second->str() << '\n';
    }

    ++level;
    current = current->enclosing.get();
  }
}
