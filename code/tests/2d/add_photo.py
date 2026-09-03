import ZaKnode
import ast


snake_like = input("Is the photo snake-like? (y/n): ").strip().lower() == 'y'

input_arr = input("Enter a list of numbers separated by commas: ")

window = ZaKnode.Game((900, 900), __file__, "ZaKamera gallery", fps = 30, screen_ratio = 1)


lib = ZaKnode.ReadData(window.directory("archive"), "data")


input_list = ast.literal_eval(input_arr)

if snake_like:
    input_list = [
        row[::-1] if y % 2 == 1 else row 
        for y, row in enumerate(input_list)
    ]

lib.append(input_list)


ZaKnode.SaveData(window.directory("archive"), "data", lib)