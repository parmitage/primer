val assert = fun id act exp ->
   if act == exp
   then show("  PASS")
   else show("* FAIL");