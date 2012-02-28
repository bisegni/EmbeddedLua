local foo = Foo()
local autoFoo = AutoFoo()

local sum = 1

for i=1,10 do
    sum = foo:foo(sum,i);
    print("Result of calling foo:" .. sum)
end

autoFoo:foo()