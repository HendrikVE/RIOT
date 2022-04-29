### About

This example consists of three components, each of which can be compiled separately. 

- Full encoder implementation
- Full parser implementation
- Partial parser implementation

These implementations are implemented for the struct `thesis_t`:
```C
typedef struct {
    char *student_name; /* mandatory */
    int  student_id;    /* mandatory */
    int  *grade;        /* optional */
} thesis_t;
```

### Compilation

To compile all implementations, use the following command

```bash
$ COMPILE_BENCHMARK=0 COMPILE_ENCODER=1 COMPILE_FULL_PARSER=1 COMPILE_PARTIAL_PARSER=1 \
  make flash term
``` 

To compare the implementations set `COMPILE_BENCHMARK=1` to exclude all debug output.
