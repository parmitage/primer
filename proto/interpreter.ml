open Utils
open Runtime

let error msg = Format.printf "@[error: %s@]@." msg

let rec pprint exp =
  match exp with
    | Int i     -> Format.print_int i
    | Float f   -> Format.print_float f
    | Char c    -> Format.print_char c
    | Bool b    -> Format.print_bool b
    | String s  -> Format.print_string s
    | List l    -> pprint_list l
    | Lambda _
    | Closure _ -> Format.print_string "#<function>"
    | _         -> Format.print_string "#<builtin>"

and pprint_list l =
  Format.print_char '[';
  ignore (Utils.map pprint (intersperse (String ", ") l)) ;
  Format.print_char ']'

let pprint_uniop op =
  match op with
    | Not  -> Format.print_char '!'
    | Neg  -> Format.print_char '-'
    | Bnot -> Format.print_char '^'

let pprint_bitop op =
  match op with
    | Band    -> Format.print_char '&'
    | Bor     -> Format.print_char '|'
    | Xor     -> Format.print_char '~'
    | LShift  -> Format.print_string "<<"
    | RShift  -> Format.print_string ">>"

let pprint_binop op =
  match op with
    | Add  -> Format.print_char '+'
    | Sub  -> Format.print_char '-' 
    | Mul  -> Format.print_char '*'
    | Div  -> Format.print_char '/'
    | Mod  -> Format.print_string "mod"
    | Eq   -> Format.print_string "=="
    | Ne   -> Format.print_string "!="
    | Lt   -> Format.print_char '<'
    | Gt   -> Format.print_char '>'
    | Gte  -> Format.print_string ">="
    | Lte  -> Format.print_string "<="
    | And  -> Format.print_string "and"
    | Or   -> Format.print_string "or"
    | App  -> Format.print_string "++"
    | Rge  -> Format.print_string ".."
    | Cons -> Format.print_string "::"

let head exp =
  match exp with
    | List(x::xs) -> x
    | List([])    -> List []
    | String ""   -> List []
    | String s    -> Char(String.get s 0)
    | _           -> raise Type_mismatch
    
let tail exp =
  match exp with
    | List(x::xs) -> List(xs)
    | List([])    -> List []
    | String ""   -> List []
    | String s    -> String(String.sub s 1 ((String.length s) - 1))
    | _           -> raise Type_mismatch

let unary_op oper arg = 
  match oper, arg with
    | Not,  Bool x  -> Bool(not x)
    | Neg,  Int x   -> Int(-x)
    | Neg,  Float x -> Float(-1.0 *. x)
    | Bnot, Int x   -> Int(lnot x)
    | _             -> raise Type_mismatch

let rec list_of_string str =
  let rec inner str =
    match str with
      | "" -> []
      | s  -> Char(String.get s 0) ::
        inner (String.sub s 1 ((String.length s) - 1))
  in List (inner str)

let rec eq lhs rhs =
  match lhs, rhs with 
    | Int x,     Int y     -> x == y
    | Float x,   Float y   -> (compare x y) == 0
    | Int x,     Float y   -> float_of_int x == y
    | Float x,   Int y     -> x == float_of_int y
    | Char x,    Char y    -> x = y
    | Bool x,    Bool y    -> x == y
    | List xs1,  List xs2  -> list_eq xs1 xs2
    | String s1, String s2 -> s1 = s2
    | List s1,   String s2 -> eq lhs (list_of_string s2)
    | String s1, List s2   -> eq (list_of_string s1) rhs
    | Any,       _         -> true
    | _,         Any       -> true
    | _,         _         -> false

and list_eq xs ys =
  match xs, ys with
    | x::xs, y::ys -> eq x y && list_eq xs ys
    | [],    []    -> true
    | _,     _     -> false

let rec binary_op oper lhs rhs =
  match oper, lhs, rhs with
    | Add,  Int x,     Int y     -> Int(x + y)
    | Add,  Int x,     Float y   -> Float(float_of_int x +. y)
    | Add,  Float x,   Int y     -> Float(x +. float_of_int y)
    | Add,  Float x,   Float y   -> Float(x +. y)
    | Sub,  Int x,     Int y     -> Int(x - y)
    | Sub,  Int x,     Float y   -> Float(float_of_int x -. y)
    | Sub,  Float x,   Int y     -> Float(x -. float_of_int y)
    | Sub,  Float x,   Float y   -> Float(x -. y)
    | Mul,  Int x,     Int y     -> Int(x * y)
    | Mul,  Int x,     Float y   -> Float(float_of_int x *. y)
    | Mul,  Float x,   Int y     -> Float(x *. float_of_int y)
    | Mul,  Float x,   Float y   -> Float(x *. y)
    | Div,  Int x,     Int y     -> Float(float_of_int x /. float_of_int y)
    | Div,  Int x,     Float y   -> Float(float_of_int x /. y)
    | Div,  Float x,   Int y     -> Float(x /. float_of_int y)
    | Div,  Float x,   Float y   -> Float(x /. y)
    | Mod,  Int x,     Int y     -> Int(x mod y)
    | Eq,   _,         _         -> Bool(eq lhs rhs)
    | Ne,   _,         _         -> Bool(not (eq lhs rhs))
    | Lt,   Int x,     Int y     -> Bool(x < y)
    | Lt,   Float x,   Float y   -> Bool(x < y)
    | Lt,   Int x,     Float y   -> Bool(float_of_int x < y)
    | Lt,   Float x,   Int y     -> Bool(x < float_of_int y)
    | Lt,   Char x,    Char y    -> Bool(compare x y < 0)
    | Gt,   Int x,     Int y     -> Bool(x > y)
    | Gt,   Float x,   Float y   -> Bool(x > y)
    | Gt,   Int x,     Float y   -> Bool(float_of_int x > y)
    | Gt,   Float x,   Int y     -> Bool(x > float_of_int y)
    | Gt,   Char x,    Char y    -> Bool(compare x y > 0)
    | Lte,  Int x,     Int y     -> Bool(x <= y)
    | Lte,  Float x,   Float y   -> Bool(x <= y)
    | Lte,  Int x,     Float y   -> Bool(float_of_int x <= y)
    | Lte,  Float x,   Int y     -> Bool(x <= float_of_int y)
    | Lte,  Char x,    Char y    -> Bool(compare x y <= 0)
    | Gte,  Int x,     Int y     -> Bool(x >= y)
    | Gte,  Float x,   Float y   -> Bool(x >= y)
    | Gte,  Int x,     Float y   -> Bool(float_of_int x >= y)
    | Gte,  Float x,   Int y     -> Bool(x >= float_of_int y)
    | Gte,  Char x,    Char y    -> Bool(compare x y >= 0)
    | And,  Bool x,    Bool y    -> Bool(x && y)
    | Or,   Bool x,    Bool y    -> Bool(x || y)
    | App,  List xs1,  List xs2  -> List(List.append xs1 xs2)
    | App,  String s1, String s2 -> String(s1 ^ s2)
    | Rge,  Int x,     Int y     -> List(Utils.map (fun i -> Int(i)) (x -- y))
    | Cons, _,         List xs   -> List(lhs :: xs)
    | Cons, Char c,    String s  -> String(String.make 1 c ^ s)
    | _                          -> raise Type_mismatch

let bitwise_op oper lhs rhs = 
  match oper, lhs, rhs with
    | Band, Int x, Int y   -> Int(x land y)
    | Bor, Int x, Int y    -> Int(x lor y)
    | Xor, Int x, Int y    -> Int(x lxor y)
    | LShift, Int x, Int y -> Int(x lsl y)
    | RShift, Int x, Int y -> Int(x lsr y)
    | _                    -> raise Type_mismatch

let length exp =
  match exp with
    | List xs  -> Int(List.length xs)
    | String s -> Int(String.length s)
    | _        -> raise Type_mismatch

let at lst idx =
  match lst, idx with
    | List l, Int i -> List.nth l i
    | _             -> raise Type_mismatch

let random exp =
  match exp with
    | Int i -> Int(Random.int i)
    | _     -> raise Type_mismatch

let cast f t =
  match f, t with
    | Int i,    Type TFloat  -> Float(float_of_int i)
    | Int i,    Type TString -> String(string_of_int i)
    | Int i,    Type TBool   -> Bool(if i <= 0 then false else true)
    | Float f,  Type TInt    -> Int(int_of_float f)
    | Float f,  Type TString -> String(string_of_float f)
    | Bool b,   Type TInt    -> Int(if b then 1 else 0)
    | Bool b,   Type TFloat  -> Float(if b then 1.0 else 0.0)
    | Bool b,   Type TString -> String(if b then "true" else "false")
    | Char c,   Type TInt    -> Int(int_of_char c)
    | Char c,   Type TFloat  -> Float(float_of_int (int_of_char c))
    | Char c,   Type TString -> String(String.make 1 c)
    | String s, Type TInt    -> Int(int_of_string s)
    | String s, Type TFloat  -> Float(float_of_string s)
    | String s, Type TChar   -> Char(String.get s 0)
    | _                      -> raise Invalid_cast

let is lhs typ =
  match lhs, typ with
    | Int _,     Type TInt
    | Float _,   Type TFloat
    | Char _,    Type TChar
    | Bool _,    Type TBool
    | String _,  Type TString
    | List _,    Type TList 
    | Closure _, Type TLambda -> Bool(true)
    | _,         _            -> Bool(false)

let show exp =
  pprint exp;
  Format.print_newline();
  exp

let is_primitive_list l =
  List.for_all
    (fun e -> match e with
      | Int _
      | Float _
      | Char _
      | Bool _
      | String _ -> true
      | _        -> false) l

let rec eval exp env =
  match exp with
    | Int _
    | Float _
    | Char _
    | Bool _
    | String _
    | Closure _
    | Any
    | Type _             -> exp
    | Symbol s           -> eval (Environment.lookup exp env) env
    | List l             -> if is_primitive_list l then exp else List(evlis l env)
    | If _               -> condition exp env
    | Let(s, e1, e2)     -> plet s e1 e2 env
    | Lambda(p, b)       -> Closure(p, b, env)
    | Apply(s, a)        -> apply (eval s env) (evlis a env) env
    | UniOp(o, arg)      -> unary_op o (eval arg env)
    | BinOp(o, lhs, rhs) -> binary_op o (eval lhs env) (eval rhs env)
    | BitOp(o, lhs, rhs) -> bitwise_op o (eval lhs env) (eval rhs env)
    | Head xs            -> head (eval xs env)
    | Tail xs            -> tail (eval xs env)
    | Length exp         -> length (eval exp env)
    | At(xs, i)          -> at (eval xs env) (eval i env)
    | Is(x, t)           -> is (eval x env) (eval t env)
    | Show exp           -> show (eval exp env)
    | Rnd i              -> random (eval i env)
    | Cast(f, t)         -> cast (eval f env) (eval t env)
    | Seq exps           -> seq exps env
    | Match(t, c)        -> matches t c env
    | Def(_, e)          -> raise Internal_error (* handled by reader *)
    | Using s            -> raise Internal_error (* handled by reader *)
      
and apply f args env =
  match f with
    | Closure(p, b, ce) -> eval b (Environment.bind p args ce)
    | _                 -> raise Type_mismatch

and evlis lst env =
  Utils.map (fun exp -> eval exp env) lst

and plet sym exp1 exp2 env =
  eval exp2 (Environment.bind [sym] [exp1] env)

and condition exp env =
  match exp with
    | If(p, c, a) ->
      begin match (eval p env) with
        | Bool b -> if b then (eval c env) else (eval a env)
        | _      -> raise Type_mismatch
      end
    | _ -> raise Type_mismatch

(* TODO handle Inexhaustive_pattern somewhere *)
and matches t c env =
  let test_results = evlis t env in
  match c with
    | Pattern(c, a) :: ps ->
      let match_results = evlis c env in
      if List.for_all2 (fun a b -> eq a b) test_results match_results
      then eval a env
      else matches t ps env
    | _ -> raise Inexhaustive_pattern

and seq exps env =
  match exps with
    | [x]   -> eval x env  
    | x::xs -> ignore (eval x env); seq xs env
    | _     -> List []
      
let initial_toplevel env = 
  Def(Symbol(SymbolTable.intern("int")), Type(TInt)) ::
    Def(Symbol(SymbolTable.intern("float")), Type(TFloat)) ::
    Def(Symbol(SymbolTable.intern("char")), Type(TChar)) ::
    Def(Symbol(SymbolTable.intern("bool")), Type(TBool)) ::
    Def(Symbol(SymbolTable.intern("string")), Type(TString)) ::
    Def(Symbol(SymbolTable.intern("list")), Type(TList)) ::
    Def(Symbol(SymbolTable.intern("lambda")), Type(TLambda)) ::
    Def(Symbol(SymbolTable.intern("newline")), Char('\n')) ::
    Def(Symbol(SymbolTable.intern("tab")), Char('\t')) :: env    

let interactive = Array.length Sys.argv == 1

let lexbuf =
  if interactive
  then Lexing.from_channel stdin
  else
    try
      Lexing.from_channel (open_in Sys.argv .(1))
    with
      | Sys_error _ -> error "file not found"; exit 0

let rec repl env =
  if interactive then Format.print_string "> "; Format.print_flush();
  try
    let result = Parser.main Lexer.token lexbuf in
    match result with
      | Def(s, e) -> repl (Def(s, e) :: env)
      | Using s   -> repl (using s env)
      | _         -> ignore (show (eval result env)); repl env
  with
    | Symbol_unbound      -> error "unbound symbol"; repl env
    | Type_mismatch       -> error "type mismatch"; repl env
    | Invalid_cast        -> error "invalid cast"; repl env
    | Parsing.Parse_error -> error "parse error"; repl env
    | Lexer.Eof           -> exit 0

and load buf env =
  try let result = Parser.main Lexer.token buf in
      match result with
        | Def(s, e) -> load buf (Def(s, e) :: env)
        | Using s   -> load buf (using s env)
        | _         -> load buf env
  with Lexer.Eof    -> env
    
and using str env =
  match str with
    | String s ->
      begin match LibraryCache.cached s with
        | true  -> env
        | false -> load (Lexing.from_channel (open_in (Utils.library_path s))) env
      end
    | _        -> raise Type_mismatch

let _ =
  Random.self_init();
  repl (initial_toplevel []) ;;
