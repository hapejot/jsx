

x = "/Date(1588279975000)/"
print(x.replace(/\//g , ""))
print(x.replace(/[0-9]+/, "&"))
y = eval("new "+  x.replace(/\//g , ""))
print(y.toLocalString())
// x = new Date(1588279975000)
