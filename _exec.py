def f():
    a=0
    exec('a=1')
    print(a)

f() # gives 0

def f2():
    a=0
    print(locals())
    exec('print(locals());a=1;print(locals())')
    print(locals())

f2()# gives:
    # {'a': 0}
    # {'a': 0}
    # {'a': 1}
    # {'a': 0}

def f3():
    a=0
    print(id(locals()))
    exec('print(id(locals()));a=1;print(id(locals()))')
    print(id(locals()))

f3() # gives 4 same ids

# can not change local variables by using exec()