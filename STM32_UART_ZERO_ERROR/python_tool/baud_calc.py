def calc_error(fck, baud):
    usartdiv = fck / (16 * baud)
    actual = fck / (16 * round(usartdiv))
    error = (actual - baud) / baud * 100
    return error

fck = 36864000
bauds = [9600,19200,38400,57600,115200]

for b in bauds:
    print(f"{b} : {calc_error(fck,b):.6f}%")
