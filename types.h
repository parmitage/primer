#ifndef __TYPES_H__
#define __TYPES_H__

#define MAX_TUPLES 10

typedef enum {
   t_any,
   t_num,
   t_int,
   t_float,
   t_bool,
   t_symbol,
   t_char,
   t_pair,
   t_list,
   t_tuple,
   t_closure
} t_type;

enum kind { K_PRIMITIVE, K_LIST, K_TUPLE };

typedef struct k_list {
   t_type t;
} k_list;

typedef struct k_tuple {
   int count;
   t_type t[MAX_TUPLES];
} k_tuple;

typedef struct type {
   enum kind k;
   union {
      t_type typ;
      struct k_list lst;
      struct k_tuple tup;
   };
} type;

#endif
