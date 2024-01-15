import random

def generate_random_rectangle():
    left_top_x = random.randint(0, 1000)
    left_top_y = random.randint(0, 1000)
    right_bottom_x = random.randint(left_top_x, 1000)
    right_bottom_y = random.randint(0, left_top_y)
    decimal_value = round(random.uniform(0, 1), 2)
    
    return left_top_x, left_top_y, right_bottom_x, right_bottom_y, decimal_value

def write_to_file(filename, num_lines):
    with open(filename, 'w') as file:
        for _ in range(num_lines):
            left_top_x, left_top_y, right_bottom_x, right_bottom_y, decimal_value = generate_random_rectangle()
            line = f"{left_top_x} {left_top_y} {right_bottom_x} {right_bottom_y} {decimal_value}\n"
            file.write(line)

if __name__ == "__main__":
    filename = "Data/in.txt"
    num_lines = 100000
    write_to_file(filename, num_lines)
    print(f"{num_lines} lines of data have been written to {filename}.")
 