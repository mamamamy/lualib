local sha256 = require "crypto.sha256"
local base64 = require "encoding.base64"

local h = sha256.New()

h:Write("")
local hash = h:Sum()
print(base64.URLEncoding:Encode(hash))
print(base64.StdEncoding:Encode(hash))

print("sha256.Size", sha256.Size)
