def hex_digit2dec(s):
    hex_digits = ['0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F']
    return hex_digits.index(s)

def hex2dec_rec(s):
    if len(s) == 0: return None
    if len(s) == 1:
        return hex_digit2dec(s)
    else:
        return hex2dec_rec(s[:-1]) * 16 + hex_digit2dec(s[-1])


print(hex2dec_rec('1ABC'))

print(hex2dec_rec('CAFE'))