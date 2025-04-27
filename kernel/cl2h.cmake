file(READ ${CLC_IN} CLC_FILEDATA HEX)
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," CLC_FILEDATA ${CLC_FILEDATA})
file(WRITE ${CLC_OUT} "#pragma once\nconst unsigned char source[] = {${CLC_FILEDATA}};")
