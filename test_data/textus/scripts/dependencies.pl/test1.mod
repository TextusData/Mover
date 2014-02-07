# this is a test mod file.
# the syntax is foo=bar or foo=[bar, baz, ...] where bar, baz can be { foo=bar or foo=[bar, baz] } 
# :'s can be subsituted for =.
# directives begin with $
# currently the only directive is $include.
FOO=bar,
$include test_data/textus/scripts/dependencies.pl/test2.mod
