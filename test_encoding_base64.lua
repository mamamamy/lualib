local base64 = require "encoding.base64"

print(base64.URLEncoding:Encode("t"))
print(base64.URLEncoding:Encode("test"))
print(base64.URLEncoding:Encode("test23"))
print(base64.URLEncoding:Encode([[Man is distinguished, not only by his reason, but by this singular passion from
other animals, which is a lust of the mind, that by a perseverance of delight
in the continued and indefatigable generation of knowledge, exceeds the short
vehemence of any carnal pleasure.]]))

print(base64.StdEncoding:Encode("t"))

local myEncoding = base64.New("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/")
print(myEncoding:Encode([[Man is distinguished, not only by his reason, but by this singular passion from
other animals, which is a lust of the mind, that by a perseverance of delight
in the continued and indefatigable generation of knowledge, exceeds the short
vehemence of any carnal pleasure.]]))

print(base64.URLEncoding:Decode(base64.URLEncoding:Encode([[Man is distinguished, not only by his reason, but by this singular passion from
other animals, which is a lust of the mind, that by a perseverance of delight
in the continued and indefatigable generation of knowledge, exceeds the short
vehemence of any carnal pleasure.]])))

print(base64.URLEncoding:Decode(base64.URLEncoding:Encode("H")))
print(base64.URLEncoding:Decode(base64.URLEncoding:Encode("He")))
print(base64.URLEncoding:Decode(base64.URLEncoding:Encode("Hel")))
print(base64.URLEncoding:Decode(base64.URLEncoding:Encode("Hell")))
print(base64.URLEncoding:Decode(base64.URLEncoding:Encode("Hello")))
print(base64.URLEncoding:Decode(base64.URLEncoding:Encode("Hello ")))
print(base64.URLEncoding:Decode(base64.URLEncoding:Encode("Hello w")))
print(base64.URLEncoding:Decode(base64.URLEncoding:Encode("Hello wo")))
print(base64.URLEncoding:Decode(base64.URLEncoding:Encode("Hello wor")))
print(base64.URLEncoding:Decode(base64.URLEncoding:Encode("Hello worl")))
print(base64.URLEncoding:Decode(base64.URLEncoding:Encode("Hello world")))
print(base64.URLEncoding:Decode(base64.URLEncoding:Encode("Hello world!")))
