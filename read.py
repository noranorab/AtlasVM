# Read the binary data from the image file
with open("your_image.obj", "rb") as file:
    data = file.read()

# Display the binary data in a readable format
print("Image file contents:")
for i in range(0, len(data), 16):
    print(" ".join(f"{byte:02X}" for byte in data[i:i+16]))