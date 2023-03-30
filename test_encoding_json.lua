local json = require "encoding.json"

local obj
obj = json.Parse([====[{"test": true}]====])
print("obj.test", obj.test)
obj = json.Parse([====[["test", {}, true, null, false, 3.1415]]====])
print("obj[1]", obj[1])
print("obj[2]", obj[2])
print("obj[3]", obj[3])
print("obj[4]", obj[4])
print("obj[5]", obj[5])
print("obj[6]", obj[6])
print(json.Parse([====["test string"]====]))
print(json.Parse([====["test \rnew line"]====]))
print(json.Parse([====["\u6d4b\u8bd5\u4e2d\u6587unicode"]====]))
print(json.Parse([====[0]====]))
print(json.Parse([====[0.00]====]))
print(json.Parse([====[0.01]====]))
print(json.Parse([====[123]====]))
print(json.Parse([====[0.1415]====]))
print(json.Parse([====[3.1415]====]))
print(json.Parse([====[123e+10]====]))
print(json.Parse([====[123e-10]====]))
print(json.Parse([====[12.3e-10]====]))
print(json.Parse([====[12.3e10]====]))
print(json.Parse([====[true]====]))
print(json.Parse([====[false]====]))
print(json.Parse([====[null]====]))
obj = json.Parse([====[
[{
  "id": "1",
  "first_name": "Jeanette",
  "last_name": "Penddreth",
  "email": "jpenddreth0@census.gov",
  "gender": "Female",
  "ip_address": "26.58.193.2"
}, {
  "id": 2,
  "first_name": "Giavani",
  "last_name": "Frediani",
  "email": "gfrediani1@senate.gov",
  "gender": "Male",
  "ip_address": "229.179.4.212"
}, {
  "id": 3,
  "first_name": "Noell",
  "last_name": "Bea",
  "email": "nbea2@imageshack.us",
  "gender": "Female",
  "ip_address": "180.66.162.255"
}, {
  "id": 4,
  "first_name": "Willard",
  "last_name": "Valek",
  "email": "wvalek3@vk.com",
  "gender": "Male",
  "ip_address": "67.76.188.26"
}]
]====])
print("#obj:", #obj)
for i, v in ipairs(obj) do
  for k, v in pairs(v) do
    print(k, v)
  end
end


print(json.Stringify(json.Parse([=====[
{
  "id": 1,
  "first_name": "Jeanette",
  "last_name": "Penddreth",
  "email": "jpenddreth0@census.gov",
  "gender": "Female",
  "ip_address": "26.58.193.2"
}
]=====])))
print(json.Stringify(json.Parse([=====[
[{
  "id": 1,
  "first_name": "Jeanette",
  "last_name": "Penddreth",
  "email": "jpenddreth0@census.gov",
  "gender": "Female",
  "ip_address": "26.58.193.2"
}]
]=====])))
print(json.Stringify(obj))
print(json.Parse(json.Stringify(obj)))
print(json.Stringify("test string"))
print(json.Stringify("你好"))
print(json.Stringify(301415))
print(json.Stringify(3.1415))
print(json.Stringify(true))
print(json.Stringify(false))
print(json.Stringify(nil))
print(json.Stringify({}))
print(json.Stringify({test="123", test2=false}))
print(json.Stringify({[3.1415]="string"}))
print(json.Stringify({[true]="123", test2=false}))
print(json.Stringify({["test"]="123.321value", [123.321]="123.321value"}))
print(json.Stringify({[123.321]="123.321", ["test\\2"]=false}))
print(json.Stringify({[123.321]="123.321", ["123.321"]="2"}))
print(json.Stringify({[1]="el1", [2]="el2", [3]="el3"}))