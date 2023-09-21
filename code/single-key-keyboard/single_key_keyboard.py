import hid

try:
    h = hid.device()
    h.open(0x16C0, 0x27DB)
    h.set_nonblocking(1)

    while True:
        keyboard_string = input("New keyboard string: ")
        if len(keyboard_string) < 150:
            break
        print("Keyboard string too long, must be less than 150 characters")

    newline = input("Newline (Y/n): ").lower() != "n"

    h.send_feature_report(
        [0] + [ord(x) for x in keyboard_string] + ([ord("\n")] if newline else []) + [0]
    )
    print("Successfully set the new keyboard string as {}".format(keyboard_string))
except OSError:
    print("SingleKeyKeyboard not found")
except Exception as e:
    print("Some other error occured", e)
