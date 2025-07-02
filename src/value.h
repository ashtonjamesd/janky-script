#ifndef value_h
#define value_h

typedef struct Object Object;

typedef enum {
    OBJ_STRING,
} ObjectType;

typedef struct {
  int    length;
  char*  chars;
} ObjString;

struct Object {
    ObjectType type;

    union {
        ObjString string;
    } as;
};

typedef enum {
    TYPE_BOOL,
    TYPE_NUMBER,
    TYPE_STRING,
    TYPE_IDENTIFIER,
} ValueType;

typedef struct {
    ValueType type;
    
    union {
        bool    boolean;
        double  number;
        Object *object;
        char   *identifier;
    } as;
} Value;

#endif