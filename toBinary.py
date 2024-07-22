# Define the instructions as bytes
instructions = bytearray([
    0x11, 0x00,  # POW to memory starting at 0x00 with difficulty 4
    0xD0,        # OUT (print the nonce)
    0xE0         # HALT
])

# Write the binary data to a file
with open("your_image.obj", "wb") as file:
    file.write(instructions)

print("Image file 'your_image.obj' created successfully.")