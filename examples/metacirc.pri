### Type definitions (TODO symbols) ###

val priTypeSym = "sym";
val priTypeInt = "int";
val priTypeFloat = "float";
val priTypeBool = "bool";
val priTypeChar = "char";
val priTypeList = "list";
val priTypeBinop = "binop";
val priTypeCond = "if";
val priTypeDef = "def";
val priTypeLambda = "lambda";
val priTypeClosure = "closure";
val priTypeApply = "apply";

### Accessor functions ###

val priNodeType = fun x -> x at 0;

val priIntResultType = fun x y ->
    let i = x at 1 in
    let j = y at 1 in
       if (i is int) and (j is int) then true
       else if (i is float) or (j is float) then false
       else "non-numeric type";

### Constructor functions ###

val priMakeSym = fun s -> [priTypeSym, s];
val priMakeInt = fun i -> [priTypeInt, i];
val priMakeFloat = fun f -> [priTypeFloat, f];
val priMakeBool = fun b -> [priTypeBool, b];
val priMakeChar = fun c -> [priTypeChar, c];
val priMakeList = fun l -> [priTypeList, l];
val priMakeBinOp = fun op arg1 arg2 -> [priTypeBinop, op, arg1, arg2];
val priMakeIf = fun p c a -> [priTypeCond, p, c, a];
val priMakeDef = fun s e -> [priTypeDef, s, e];
val priMakeLambda = fun p b -> [priTypeLambda, p, b];
val priMakeClosure = fun p b e -> [priTypeClosure, p, b, e];
val priMakeApply = fun s a -> [priTypeApply, s, a];

### Environment handling ###

val environmentExtend = fun env def -> def :: env;

val environmentLookup = fun env sym ->
    if env == []
    then []
    else let def = head(env) in
         let sym2 = def at 1 in
         let exp = def at 2 in
             if sym == sym2
             then exp
             else environmentLookup(tail(env), sym);

val environmentUpdate = fun ds k v ->
    if head(ds) != []
    then if head(head(ds)) == k
         then environmentExtend(tail(ds), k, v)
         else head(ds) :: environmentUpdate(tail(ds), k, v)
    else head(ds);

val symbolEq = fun sym1 sym2 ->
    let s1 = sym1 at 1 in
    let s2 = sym2 at 1 in
        s1 == s2;

val definitionEq = fun sym def ->
    let sym2 = def at 1 in symbolEq(sym, sym2);

val environmentBind = fun params args env ->
    let replaceOneBy = fun f a b l ->
        if l == []
        then []
        else let x = head(l) in
             let xs = tail(l) in
                 if f(a, x)
                 then b :: xs
                 else x :: replaceOneBy(f, a, b, xs) in
    let bindOne = fun sym exp env -> priMakeDef(sym, exp) :: env in
    let rebindOne = fun sym exp env ->
                        replaceOneBy(definitionEq, sym, priMakeDef(sym, exp), env)
    in foldl(fun e b -> # b is a zipper of [sym, exp]
                 let sym = b at 0 in
                 let exp = (b at 1) at 1 in
                     if environmentLookup(e, sym) != []
                     then rebindOne(sym, exp, e)
                     else bindOne(sym, exp, e),
             env, zip(cadr(params), (cadr(args))));

### Primitive operators ###

val priIsTrue = fun exp ->
    priNodeType(exp) == priTypeBool and exp at 1 == true;

val priAdd = fun i j ->
    if priIntResultType(i, j)
    then priMakeInt((i at 1) + (j at 1))
    else priMakeFloat((i at 1) + (j at 1));

val priMul = fun i j ->
    if priIntResultType(i, j)
    then priMakeInt((i at 1) * (j at 1))
    else priMakeFloat((i at 1) * (j at 1));

val priEvalBinOp = fun exp lhs rhs ->
    if exp == "+" then priAdd(lhs, rhs)
    else if exp == "*" then priMul(lhs, rhs)
    else "unknown operator";

### Evaluator ####

val priEval = fun pair ->
    let exp = head(pair) in
    let env = cadr(pair) in
    let kind = head(exp) in
        if kind == "int" then [exp, env]
        else if kind == "float" then [exp, env]
        else if kind == "bool" then [exp, env]
        else if kind == "char" then [exp, env]
        else if kind == "list" then [exp, env]
        else if kind == "closure" then [exp, env]
        else if kind == "sym" then let exp2 = environmentLookup(env, exp) in
                                       if exp2 == []
                                       then show("symbol unbound")
                                       else [priEval(exp2), env]
        else if kind == "def" then [exp, environmentExtend(env, exp)]                             
        else if kind == "binop" then priEvalBinOp(exp at 1,
                                                  priEval([exp at 2, env]),
                                                  priEval([exp at 3, env]),
                                                  env)
        else if kind == "if" then if priIsTrue(priEval([exp at 1, env]))
                                  then priEval([exp at 2, env])
                                  else priEval([exp at 3, env])
        else if kind == "lambda" then let params = exp at 1 in
                                      let body = exp at 2 in
                                          [priMakeClosure(params, body, env), env]
        else if kind == "apply" then let sym = exp at 1 in
                                     let args = exp at 2 in
                                     let func = priEval(sym, env) in
                                     #let args = priEvlis(args, env) in
                                         [priApply(func, args), env]
      else "unknown expression type";

val priApply = fun f args ->
    let params = f at 1 in
    let body = f at 2 in
    let closureEnv = f at 3 in
    let ext = environmentBind(params, args, closureEnv) in
        begin
            #show(params); show(args); show(body); show(closureEnv); show(ext);
            priEval(body, ext)
        end;

val priEvlis = fun lst env -> map(fun exp -> priEval(exp, env), lst);

### Test data ###

val testSym = priMakeSym("x");
val testDef = priMakeDef(testSym, priMakeInt(100));
val testInt = priMakeInt(44);
val testFloat = priMakeFloat(3.14);
val testBool = priMakeBool(true);
val testList = priMakeList([1,2,3,4]);
val testAdd = priMakeBinOp("+", priMakeInt(5), priMakeInt(12));
val testMul = priMakeBinOp("*", priMakeFloat(5.4), testSym);
val testAddMul = priMakeBinOp("+", priMakeInt(5),
                 priMakeBinOp("*", priMakeFloat(5.4), priMakeInt(12)));
val testIf = priMakeIf(priMakeBool(true), priMakeInt(1), priMakeInt(0));
val testLambda = priMakeDef(priMakeSym("f"),
                            priMakeLambda(priMakeList([priMakeSym("x")]),
                                          priMakeBinOp("*",
                                                       priMakeInt(2),
                                                       priMakeSym("x"))));
val testApply = priMakeApply(priMakeSym("f"), priMakeList([priMakeInt(12)]));

### Reader ###

val initialToplevel = [];

# TODO the reader will return a list/stream of expressions as AST nodes
val read = [testDef, testInt, testFloat, testBool, testSym, testAdd, testMul,
            testAddMul, testIf, testLambda];

val repl = fun exps env ->
    if exps == []
    then show("exit")
    else let exp = head(exps) in
         let rest = tail(exps) in
         let ret = priEval([exp, env]) in
         let result = show(head(ret)) in
         let ext = cadr(ret) in
             repl(rest, ext);

repl(read, initialToplevel);
#priEval([testInt, initialToplevel]);