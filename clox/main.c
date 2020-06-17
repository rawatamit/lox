#include "Chunk.h"
#include "Debug.h"
#include "Opcode.h"
#include "VM.h"

#include <stdio.h>
#include <stdlib.h>

static void repl(VM *vm)
{
  char line[1024];
  for (;;)
  {
    printf("> ");

    if (!fgets(line, sizeof(line), stdin))
    {
      putchar('\n');
      break;
    }

    interpret(vm, line);
  }
}

static char *read_file(const char *path)
{
  FILE *file = fopen(path, "r");

  if (file == NULL)
  {
    fprintf(stderr, "Could not open file \"%s\".\n", path);
    exit(74);
  }

  fseek(file, 0L, SEEK_END);
  size_t file_size = ftell(file);
  rewind(file);

  char *buffer = malloc(file_size + 1);
  if (buffer == NULL)
  {
    fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
    exit(74);
  }

  size_t bytes_read = fread(buffer, sizeof(char), file_size, file);

  if (bytes_read < file_size)
  {
    fprintf(stderr, "Could not read file \"%s\".\n", path);
    exit(74);
  }

  buffer[bytes_read] = '\0';
  fclose(file);
  return buffer;
}

static void run_file(VM *vm, const char *path)
{
  char *src = read_file(path);
  InterpretResult result = interpret(vm, src);
  free(src);

  if (result == INTERPRET_COMPILE_ERROR)
    exit(65);
  if (result == INTERPRET_RUNTIME_ERROR)
    exit(70);
}

int main(int argc, const char *argv[])
{
  VM vm;
  init_VM(&vm);

  if (argc == 1)
  {
    repl(&vm);
  }
  else if (argc == 2)
  {
    run_file(&vm, argv[1]);
  }
  else
  {
    fprintf(stderr, "Usage: clox [path]\n");
  }

  free_VM(&vm);
  return 0;
}
