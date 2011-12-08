{
  open Runtime
  open Parser
  exception Eof
}

let digit = ['0'-'9']
let ident = ['a'-'z' 'A'-'Z']['a'-'z' 'A'-'Z' '0'-'9']*
let string = [^ '\n' '"']*
let any = [^ '\t' '\n']
let whitespace = [' ' '\t' '\n']

rule token = parse
  | "++"                          { APPEND }
  | '+'                           { PLUS }
  | '-'                           { MINUS }
  | '*'                           { TIMES }
  | '/'                           { DIV }
  | '('                           { LPAREN }
  | ')'                           { RPAREN }
  | '['                           { LSQUARE }
  | ']'                           { RSQUARE }
  | '='                           { DEF }
  | "not"                         { NOT }
  | '<'                           { LT }
  | '>'                           { GT }
  | ">="                          { GE }
  | "<="                          { LE }
  | "=="                          { EQ }
  | "!="                          { NE }
  | "and"                         { AND }
  | "or"                          { OR }
  | ".."                          { RANGE }
  | "mod"                         { MOD }
  | ','                           { COMMA }
  | ';'                           { SEMICOLON }
  | '&'                           { B_AND }
  | '|'                           { B_OR }
  | '^'                           { B_XOR }
  | '~'                           { B_NOT }
  | "<<"                          { B_LSHIFT }
  | ">>"                          { B_RSHIFT }
  | "if"                          { IF }
  | "then"                        { THEN }
  | "else"                        { ELSE }
  | "match"                       { MATCH }
  | "with"                        { WITH }
  | "let"                         { LET }
  | "val"                         { VAL }
  | "in"                          { IN }
  | "fun"                         { LAMBDA }
  | "->"                          { DEFINED }
  | "begin"                       { BEGIN }
  | "end"                         { END }
  | "true"                        { TRUE }
  | "false"                       { FALSE }
  | "_"                           { ANY }
  | "is"                          { IS }
  | "as"                          { AS }
  | "at"                          { AT }
  | "::"                          { CONS }
  | "head"                        { HEAD }
  | "tail"                        { TAIL }
  | "show"                        { SHOW }
  | "rnd"                         { RND }
  | "length"                      { LENGTH }
  | "using"                       { USING }
  | '#' any* '\n'?                { token lexbuf }
  | ''' any ''' as lxm            { CHAR(String.get lxm 1) }
  | '"' string '"' as lxm         { STRING(String.sub lxm 1 (String.length lxm - 2)) }
  | whitespace                    { token lexbuf }
  | digit+ as lxm                 { INT(int_of_string lxm) }
  | digit+ '.' digit+ as lxm      { FLOAT(float_of_string lxm) }
  | ident as lxm                  { SYMBOL(lxm) }
  | eof                           { raise Eof }
