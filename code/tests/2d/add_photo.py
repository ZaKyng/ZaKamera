import ZaKnode
import ast


input_arr = input("Enter a list of numbers separated by commas: ")

window = ZaKnode.Game((900, 900), __file__, "ZaKamera gallery", fps = 30, screen_ratio = 1)

lib = ZaKnode.ReadData(window.directory("archive"), "data")

input_list = ast.literal_eval(input_arr)

print(lib)

lib.append(input_list)


ZaKnode.SaveData(window.directory("archive"), "data", lib)