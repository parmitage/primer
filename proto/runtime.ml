open Utils

exception Type_mismatch
exception Symbol_unbound
exception Symbol_redefined
exception Invalid_cast
exception Inexhaustive_pattern
exception Internal_error

type typename = TInt | TFloat | TChar | TBool | TList | TString | TLambda
type uniop = Not | Neg | Bnot
type bitop = Band | Bor | Xor | LShift | RShift
type binop = Add | Sub | Mul | Div | Mod | Eq | Ne | Lt | Gt | Gte | Lte | And
             | Or | App | Rge | Cons

type expression =
  | Symbol of int
  | Int of int
  | Float of float
  | Char of char
  | Bool of bool
  | String of string
  | List of expression list
  | If of expression * expression * expression
  | Let of expression * expression * expression
  | Lambda of expression list * expression
  | Closure of expression list * expression * expression list
  | Apply of expression * expression list
  | BinOp of binop * expression * expression
  | UniOp of uniop * expression
  | BitOp of bitop * expression * expression
  | Head of expression
  | Tail of expression
  | Length of expression
  | At of expression * expression
  | Is of expression * expression
  | Show of expression
  | Rnd of expression
  | Cast of expression * expression
  | Def of expression * expression
  | Type of typename
  | Seq of expression list
  | Match of expression list * pattern list
  | Using of expression
  | Any
and pattern =
  | Pattern of expression list * expression

module Environment =
struct
  let symbol_eq s1 s2 = match s1, s2 with
    | Symbol sym1, Symbol sym2 -> sym1 = sym2
    | _ -> raise Type_mismatch

  let definition_eq sym def = match def with
    | Def(sym2, exp) -> symbol_eq sym sym2
    | _ -> raise Type_mismatch

  let symbol_bound sym env = List.exists (fun b -> definition_eq sym b) env

  let lookup sym env =
    try match List.find (fun b -> definition_eq sym b) env with
      | Def(s, v) -> v
      | _ -> raise Type_mismatch
    with _ -> raise Symbol_unbound

  let bind params args env =
    let bind_one sym exp env = Def(sym, exp) :: env in
    let rebind_one sym exp env =
      replace_one_by definition_eq sym (Def(sym, exp)) env in
    List.fold_left
      (fun e (sym, exp) ->
        if symbol_bound sym e
        then rebind_one sym exp e
        else bind_one sym exp e)
      env (zip params args)
end

module SymbolTable =
struct
  let index = ref 0

  (* forward lookups of string -> symbol *)
  let symtab : (string, int) Hashtbl.t = Hashtbl.create 100

  (* reverse lookups of symbol -> string *)
  let stringtab : (int, string) Hashtbl.t = Hashtbl.create 100

  let intern str =
    let member = Hashtbl.mem symtab str in
    let idx = !index in
    if not member
    then
      begin
        Hashtbl.add symtab str idx;
        Hashtbl.add stringtab idx str;
      end;
    if not member then index := !index + 1;
    Hashtbl.find symtab str

  let lookup_sym sym = Hashtbl.find stringtab sym
end

module LibraryCache =
struct
  let cache : (string, int) Hashtbl.t = Hashtbl.create 10

  let cached name =
    if not (Hashtbl.mem cache name)
    then
      begin
        Hashtbl.add cache name 0;
        false;
      end
    else true
    
end
