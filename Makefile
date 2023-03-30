all:
	gcc -O2 -Wall -fPIC -shared ./crypto/lualib_sha256.c -o ./crypto/sha256.so -lssl -lcrypto
	gcc -O2 -Wall -fPIC -shared ./encoding/lualib_base64.c -o ./encoding/base64.so
	gcc -O2 -Wall -fPIC -shared ./encoding/lualib_json.c -o ./encoding/json.so

test:
	lua ./test_crypto_sha256.lua
	lua ./test_encoding_base64.lua
	lua ./test_encoding_json.lua