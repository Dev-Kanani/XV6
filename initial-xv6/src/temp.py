import re

# Define the pattern for valid lines
pattern = re.compile(r'^timestamp=\d+ process_id=\d+ queue_level=\d+$')

# Open the file and filter lines
with open('bo.txt', 'r') as file:
    lines = file.readlines()

# Keep only the lines that match the pattern
valid_lines = [line for line in lines if pattern.match(line.strip())]

# Write the valid lines back to the file
with open('bo.txt', 'w') as file:
    file.writelines(valid_lines)

print("File has been updated with valid lines only.")
