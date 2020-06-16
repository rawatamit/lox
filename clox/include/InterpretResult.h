#ifndef _INTERPRET_RESULT_H_
#define _INTERPRET_RESULT_H_

enum InterpretResult {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR,
};

typedef enum InterpretResult InterpretResult;

#endif
